#include "Replay.h"

void inicializarReplay(ListaReplay* lista, const EstadoReplay* estadoInicial) {
	lista->primero = nullptr;
	lista->ultimo = nullptr;
	lista->actual = nullptr; // arrancamos parados en el estado inicial
	lista->estadoInicial = *estadoInicial;
	lista->cantidad = 0;
}

void destruirReplay(ListaReplay* lista) {
	NodoMovimiento* actual = lista->primero;
	while (actual != nullptr) {
		NodoMovimiento* siguiente = actual->siguiente;
		delete actual;
		actual = siguiente;
	}
	lista->primero = nullptr;
	lista->ultimo = nullptr;
	lista->actual = nullptr;
	lista->cantidad = 0;
}

// Elimina todos los nodos DESPUES de "desde" (sin incluirlo). Se usa
// cuando el jugador deshizo movimientos y luego hizo uno nuevo: se
// descarta la "rama" vieja de redo, igual que un editor de texto.
static void descartarNodosDespuesDe(ListaReplay* lista, NodoMovimiento* desde) {
	NodoMovimiento* primerADescartar = (desde == nullptr) ? lista->primero : desde->siguiente;
	
	NodoMovimiento* actual = primerADescartar;
	while (actual != nullptr) {
		NodoMovimiento* siguiente = actual->siguiente;
		delete actual;
		lista->cantidad--;
		actual = siguiente;
	}
	
	if (desde == nullptr) {
		lista->primero = nullptr;
		lista->ultimo = nullptr;
	} else {
		desde->siguiente = nullptr;
		lista->ultimo = desde;
	}
}

void registrarMovimiento(ListaReplay* lista, TipoMovimiento tipo, const EstadoReplay* nuevoEstado) {
	// Si "actual" no es el ultimo nodo, hay una rama de redo vieja: se descarta.
	if (lista->actual != lista->ultimo) {
		descartarNodosDespuesDe(lista, lista->actual);
	}
	
	NodoMovimiento* nuevo = new NodoMovimiento;
	nuevo->tipo = tipo;
	nuevo->estado = *nuevoEstado;
	nuevo->anterior = lista->ultimo;
	nuevo->siguiente = nullptr;
	
	if (lista->ultimo == nullptr) {
		lista->primero = nuevo;
	} else {
		lista->ultimo->siguiente = nuevo;
	}
	lista->ultimo = nuevo;
	lista->actual = nuevo;
	lista->cantidad++;
}

bool deshacerMovimiento(ListaReplay* lista, EstadoReplay* destino) {
	if (lista->actual == nullptr) {
		return false; // ya estamos en el estado inicial, no hay nada mas atras
	}
	
	NodoMovimiento* nodoAnterior = lista->actual->anterior;
	*destino = (nodoAnterior != nullptr) ? nodoAnterior->estado : lista->estadoInicial;
	lista->actual = nodoAnterior;
	return true;
}

bool rehacerMovimiento(ListaReplay* lista, EstadoReplay* destino) {
	NodoMovimiento* siguienteNodo = (lista->actual == nullptr) ? lista->primero : lista->actual->siguiente;
	if (siguienteNodo == nullptr) {
		return false; // ya estamos en el ultimo movimiento, no hay mas adelante
	}
	
	*destino = siguienteNodo->estado;
	lista->actual = siguienteNodo;
	return true;
}

void iniciarReproduccion(ListaReplay* lista) {
	lista->actual = nullptr; // el bucle de reproduccion usa rehacerMovimiento desde aca
}

void capturarEstado(const Tablero* t, const Pieza* p, int puntaje, EstadoReplay* destino) {
	NodoFila* fila = t->primeraFila;
	int i = 0;
	while (fila != nullptr && i < ALTO_TABLERO) {
		for (int c = 0; c < ANCHO_TABLERO; c++) {
			destino->celdas[i][c] = fila->celdas[c];
		}
		fila = fila->siguiente;
		i++;
	}
	for (; i < ALTO_TABLERO; i++) {
		for (int c = 0; c < ANCHO_TABLERO; c++) {
			destino->celdas[i][c] = 0;
		}
	}
	destino->piezaActiva = *p;
	destino->puntaje = puntaje;
}

void aplicarEstado(Tablero* t, Pieza* p, int* puntaje, const EstadoReplay* origen) {
	NodoFila* fila = t->primeraFila;
	int i = 0;
	while (fila != nullptr && i < ALTO_TABLERO) {
		for (int c = 0; c < ANCHO_TABLERO; c++) {
			fila->celdas[c] = origen->celdas[i][c];
		}
		fila = fila->siguiente;
		i++;
	}
	*p = origen->piezaActiva;
	*puntaje = origen->puntaje;
}
