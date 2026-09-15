#ifndef PILAHOLD_H
#define PILAHOLD_H

#include "Pieza.h"

// Pila enlazada (LIFO). El hold la usa con capacidad 1.
struct NodoPila {
	Pieza dato;
	NodoPila* siguiente;
};

struct PilaHold {
	NodoPila* tope;
	int cantidad;
	int capacidad;
};

void inicializarPilaHold(PilaHold* pila, int capacidad);
void destruirPilaHold(PilaHold* pila);

bool pilaHoldVacia(const PilaHold* pila);
bool pilaHoldLlena(const PilaHold* pila);
bool pushHold(PilaHold* pila, Pieza p);
Pieza popHold(PilaHold* pila);
Pieza topeHold(const PilaHold* pila);

#endif
