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
		return false;
	}
	NodoPila* nuevo = new NodoPila;
	nuevo->dato = p;
	nuevo->siguiente = pila->tope;
	pila->tope = nuevo;
	pila->cantidad++;
	return true;
}

// Igual que en la cola, el juego llama a estas dos solo con la pila con
// elementos; si llegara vacia se devuelve una pieza neutra en vez de
// desreferenciar nullptr.
Pieza popHold(PilaHold* pila) {
	NodoPila* antiguo = pila->tope;
	if (antiguo == nullptr) {
		return crearPieza(PIEZA_I);
	}
	Pieza resultado = antiguo->dato;
	pila->tope = antiguo->siguiente;
	delete antiguo;
	pila->cantidad--;
	return resultado;
}

Pieza topeHold(const PilaHold* pila) {
	if (pila->tope == nullptr) {
		return crearPieza(PIEZA_I);
	}
	return pila->tope->dato;
}
