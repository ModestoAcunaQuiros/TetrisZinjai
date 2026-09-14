#include "PilaHold.h"

void inicializarPilaHold(PilaHold* pila, int capacidad) {
	pila->tope = nullptr;
	pila->cantidad = 0;
	pila->capacidad = capacidad;
}

void destruirPilaHold(PilaHold* pila) {
	NodoPila* actual = pila->tope;
	while (actual != nullptr) {
		NodoPila* siguiente = actual->siguiente;
		delete actual;
		actual = siguiente;
	}
	pila->tope = nullptr;
	pila->cantidad = 0;
}

bool pilaHoldVacia(const PilaHold* pila) {
	return pila->tope == nullptr;
}

bool pilaHoldLlena(const PilaHold* pila) {
	return pila->cantidad >= pila->capacidad;
}

bool pushHold(PilaHold* pila, Pieza p) {
	if (pilaHoldLlena(pila)) {
		return false; // no hay espacio en la pila
	}
	NodoPila* nuevo = new NodoPila;
	nuevo->dato = p;
	nuevo->siguiente = pila->tope;
	pila->tope = nuevo;
	pila->cantidad++;
	return true;
}

Pieza popHold(PilaHold* pila) {
	// Se asume que el llamador ya verifico pilaHoldVacia() == false.
	NodoPila* antiguo = pila->tope;
	Pieza resultado = antiguo->dato;
	pila->tope = antiguo->siguiente;
	delete antiguo;
	pila->cantidad--;
	return resultado;
}

Pieza topeHold(const PilaHold* pila) {
	// Se asume que el llamador ya verifico pilaHoldVacia() == false.
	return pila->tope->dato;
}
