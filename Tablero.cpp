#include "Tablero.h"

static NodoFila* crearFilaVacia() {
	NodoFila* fila = new NodoFila;
	for (int c = 0; c < ANCHO_TABLERO; c++) {
		fila->celdas[c] = 0;
	}
	fila->siguiente = nullptr;
	return fila;
}

void inicializarTablero(Tablero* t) {
	t->primeraFila = nullptr;
	t->cantidadFilas = 0;
	
	NodoFila* anterior = nullptr;
	for (int i = 0; i < ALTO_TABLERO; i++) {
		NodoFila* nueva = crearFilaVacia();
		if (t->primeraFila == nullptr) {
			t->primeraFila = nueva;
		} else {
			anterior->siguiente = nueva;
		}
		anterior = nueva;
		t->cantidadFilas++;
	}
}

void destruirTablero(Tablero* t) {
	NodoFila* actual = t->primeraFila;
	while (actual != nullptr) {
		NodoFila* siguiente = actual->siguiente;
		delete actual;
		actual = siguiente;
	}
	t->primeraFila = nullptr;
	t->cantidadFilas = 0;
}

NodoFila* obtenerFila(const Tablero* t, int indice) {
	NodoFila* actual = t->primeraFila;
	int i = 0;
	while (actual != nullptr && i < indice) {
		actual = actual->siguiente;
		i++;
	}
	return actual;
}

bool celdaLibre(const Tablero* t, int fila, int columna) {
	if (fila < 0 || fila >= ALTO_TABLERO || columna < 0 || columna >= ANCHO_TABLERO) {
		return false; // fuera del tablero: se trata como "ocupado" (colisiona)
	}
	NodoFila* nodo = obtenerFila(t, fila);
	return nodo->celdas[columna] == 0;
}

void ocuparCelda(Tablero* t, int fila, int columna, int valor) {
	NodoFila* nodo = obtenerFila(t, fila);
	if (nodo != nullptr) {
		nodo->celdas[columna] = valor;
	}
}

int detectarFilasCompletas(const Tablero* t, int* indicesCompletos) {
	int cantidad = 0;
	NodoFila* actual = t->primeraFila;
	int indice = 0;
	
	while (actual != nullptr) {
		bool completa = true;
		for (int c = 0; c < ANCHO_TABLERO; c++) {
			if (actual->celdas[c] == 0) {
				completa = false;
				break;
			}
		}
		if (completa) {
			indicesCompletos[cantidad] = indice;
			cantidad++;
		}
		actual = actual->siguiente;
		indice++;
	}
	
	return cantidad;
}

void limpiarFilasCompletas(Tablero* t, const int* indicesCompletos, int cantidad) {
	if (cantidad == 0) {
		return;
	}
	
	// Recorremos la lista una sola vez, reconstruyendola sin las filas
	// completas. Guardamos aparte los nodos completos para reciclarlos
	// (en vez de destruirlos y crear nuevos, los vaciamos y los reusamos
	// como las nuevas filas vacias que van al inicio).
	NodoFila* nuevaLista = nullptr;
	NodoFila* colaNuevaLista = nullptr;
	NodoFila* filasRecicladas = nullptr;
	
	NodoFila* actual = t->primeraFila;
	int indice = 0;
	int cursorCompletos = 0; // indicesCompletos esta ordenado de menor a mayor
	
	while (actual != nullptr) {
		NodoFila* siguiente = actual->siguiente;
		
		bool esCompleta = (cursorCompletos < cantidad && indicesCompletos[cursorCompletos] == indice);
		
		if (esCompleta) {
			cursorCompletos++;
			// Se recicla: se vacia y se guarda para ponerla al inicio despues.
			for (int c = 0; c < ANCHO_TABLERO; c++) {
				actual->celdas[c] = 0;
			}
			actual->siguiente = filasRecicladas;
			filasRecicladas = actual;
		} else {
			actual->siguiente = nullptr;
			if (nuevaLista == nullptr) {
				nuevaLista = actual;
				colaNuevaLista = actual;
			} else {
				colaNuevaLista->siguiente = actual;
				colaNuevaLista = actual;
			}
		}
		
		actual = siguiente;
		indice++;
	}
	
	// Las filas recicladas (ahora vacias) se insertan AL INICIO.
	if (filasRecicladas != nullptr) {
		NodoFila* finReciclado = filasRecicladas;
		while (finReciclado->siguiente != nullptr) {
			finReciclado = finReciclado->siguiente;
		}
		finReciclado->siguiente = nuevaLista;
		t->primeraFila = filasRecicladas;
	} else {
		t->primeraFila = nuevaLista;
	}
	
	// cantidadFilas no cambia: siguen siendo ALTO_TABLERO filas en total.
}
