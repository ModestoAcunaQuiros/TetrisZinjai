#ifndef PILAHOLD_H
#define PILAHOLD_H

#include "Pieza.h"

// Pila propia de capacidad 1. NO usar std::stack.
// Aunque la capacidad sea 1, se implementa con la logica real de
// pila (tope, push, pop) para respetar el objetivo de aprendizaje.
struct PilaHold {
	Pieza tope;
	bool hayPieza; // false = pila vacia
};

void inicializarPilaHold(PilaHold* pila);
bool pilaHoldVacia(const PilaHold* pila);

// Intenta guardar una pieza en el hold. Devuelve false si ya habia una
// (en ese caso, el llamador debe decidir la regla: por ejemplo, no dejar
// usar hold de nuevo hasta que la pieza actual se coloque).
bool pushHold(PilaHold* pila, Pieza p);

// Saca la pieza guardada y deja el hold vacio. Asume que pilaHoldVacia()
// es false (revisar antes de llamar).
Pieza popHold(PilaHold* pila);

#endif
