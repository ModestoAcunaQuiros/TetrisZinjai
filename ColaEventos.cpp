#include "ColaEventos.h"

void inicializarColaEventos(ColaEventos* cola){
	cola->frente = nullptr;
	cola->cantidad = 0;
}
void destruirColaDeEventos(ColaEventos* cola){
	NodoEvento* actual = cola->frente;
	while(actual != nullptr){
		NodoEvento* siguiente = actual->siguiente;
		delete actual;
		actual = siguiente;
	}
	cola->frente = nullptr;
	cola->cantidad = 0;
}
bool eventosVacio(ColaEventos* cola){
	return cola->cantidad == 0;
}
void programarEvento(ColaEventos* cola, Evento evento){
	NodoEvento* nuevo = new NodoEvento;
	nuevo->dato = evento;
	if(cola->frente == nullptr || evento.momentoEvento < cola->frente->dato.momentoEvento){
		nuevo->siguiente = cola->frente;
		cola->frente = nuevo;
		cola->cantidad++;
		return;
	}
	NodoEvento* actual = cola->frente;
	while(actual->siguiente != nullptr && actual->siguiente->dato.momentoEvento <= evento.momentoEvento){
		actual = actual->siguiente;
	}
	nuevo->siguiente = actual->siguiente;
	actual->siguiente = nuevo;
	cola->cantidad++;
}
	
bool eventolisto(ColaEventos* cola, float tiempoActual){
	if(cola->frente == nullptr){
		return false;
	}
	return cola->frente->dato.momentoEvento <= tiempoActual;
}
	
Evento extraeEvento(ColaEventos* cola){
	NodoEvento* antiguoFrente = cola->frente;
	Evento resultado = antiguoFrente ->dato;
	cola->frente = antiguoFrente->siguiente;
	delete antiguoFrente;
	cola->cantidad--;
	return resultado;
}
