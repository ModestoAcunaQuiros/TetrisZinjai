#include "ColaPieza.h"
#include <cstdlib>
#include <ctime>


void inicializarColaPiezas(ColaPiezas* cola){
	cola->frente = nullptr;
	cola->ultimo = nullptr;
	cola->cantidad = 0;
}
	
void destruirColaPiezas(ColaPiezas* cola){
	NodoPieza* actual = cola->frente;
	while(actual != nullptr){
		NodoPieza* siguiente = actual->siguiente;
		delete actual;
		actual = siguiente;
	}
	cola->frente = nullptr;
	cola->ultimo = nullptr;
	cola->cantidad = 0;
}

void encolarPieza(ColaPiezas* cola, Pieza pieza){
	NodoPieza* nuevo = new NodoPieza;
	nuevo->dato = pieza;
	nuevo->siguiente = nullptr;
	
	if(cola->ultimo == nullptr){
	cola->frente = nuevo;
	cola->ultimo = nuevo;
	}else{
		cola->ultimo->siguiente = nuevo;
		cola->ultimo = nuevo;
	}
	cola->cantidad++;
}
	
Pieza desencolarPieza(ColaPiezas* cola){
	NodoPieza* aux = cola->frente;
	Pieza resultado = aux->dato;
	
	cola->frente = aux->siguiente;
	if(cola->frente == nullptr){
		cola->ultimo = nullptr;
	}
	
	delete aux;
	cola->cantidad--;
	return resultado;
}
	
bool colaPiezasVacia(const ColaPiezas* cola){
	return cola->cantidad == 0;
}
	
void generarNuevaBolsa(ColaPiezas* cola){
	TipoPieza bolsa[CANTIDAD_TIPOS_PIEZA];
	for(int i =0; i < CANTIDAD_TIPOS_PIEZA; i++){
		bolsa[i] = static_cast<TipoPieza>(i);
	}
	
	for(int i = CANTIDAD_TIPOS_PIEZA - 1; i > 0; i--){
		int j = rand() % (i + 1);
		TipoPieza temp = bolsa[i];
		bolsa[i] = bolsa[j];
		bolsa[j] = temp;
	}
	
	for(int i = 0; i < CANTIDAD_TIPOS_PIEZA; i++){
		encolarPieza(cola, crearPieza(bolsa[i]));
	}
}

	
void piezasSuficientes(ColaPiezas* cola, int minimoSuficiente){
	while ( cola->cantidad < minimoSuficiente){
		generarNuevaBolsa(cola);
	}
}
	
	
void proximasPiezas(const ColaPiezas* cola, Pieza* destino, int cantidad){
	NodoPieza* actual = cola->frente;
	int i = 0;
	while (actual != nullptr && i < cantidad){
		destino[i] = actual->dato;
		actual = actual->siguiente;
		i++;
	}
}
