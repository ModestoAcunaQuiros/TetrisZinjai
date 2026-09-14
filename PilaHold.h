#ifndef PILAHOLD_H
#define PILAHOLD_H

#include "Pieza.h"

// Pila propia (LIFO) implementada con nodos enlazados. NO usar std::stack.
// Para la casilla de espera (hold) se usa con capacidad 1, pero la
// estructura es una pila real (tope, push, pop, tope sin desapilar).
struct NodoPila {
	Pieza dato;
	NodoPila* siguiente;
};

struct PilaHold {
	NodoPila* tope;  // nullptr = pila vacia
	int cantidad;    // cuantos elementos hay apilados
	int capacidad;   // el hold usa 1
};

// Inicializa la pila con la capacidad indicada (el hold usa 1).
void inicializarPilaHold(PilaHold* pila, int capacidad);

// Libera todos los nodos de la pila.
void destruirPilaHold(PilaHold* pila);

bool pilaHoldVacia(const PilaHold* pila);
bool pilaHoldLlena(const PilaHold* pila);

// Apila una pieza (LIFO). Devuelve false si la pila esta llena.
bool pushHold(PilaHold* pila, Pieza p);

// Desapila y devuelve la pieza del tope. Asume que no esta vacia.
Pieza popHold(PilaHold* pila);

// Devuelve la pieza del tope sin desapilarla (para dibujarla).
// Asume que no esta vacia.
Pieza topeHold(const PilaHold* pila);

#endif
