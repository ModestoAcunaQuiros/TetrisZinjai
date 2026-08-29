#include "PilaHold.h"

void inicializarPilaHold(PilaHold* pila) {
	pila->hayPieza = false;
	// pila->tope queda con valores basura hasta el primer push,
	// pero no se lee mientras hayPieza sea false, asi que no importa.
}

bool pilaHoldVacia(const PilaHold* pila) {
	return !pila->hayPieza;
}

bool pushHold(PilaHold* pila, Pieza p) {
	if (pila->hayPieza) {
		return false;
	}
	pila->tope = p;
	pila->hayPieza = true;
	return true;
}

Pieza popHold(PilaHold* pila) {
	// Se asume que el llamador ya verifico pilaHoldVacia() == false.
	pila->hayPieza = false;
	return pila->tope;
}
