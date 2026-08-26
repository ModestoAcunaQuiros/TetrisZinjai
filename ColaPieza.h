#ifndef COLAPIEZA_H
#define COLAPIEZA_H

#include "Pieza.h"

struct NodoPieza{
	Pieza dato;
	NodoPieza* siguiente;
};

struct ColaPiezas{
	NodoPieza* frente;
	NodoPieza* ultimo;
	int cantidad;
};

void inicializarColaPiezas(ColaPiezas* cola);
void destruirColaPiezas(ColaPiezas* cola);

void encolarPieza(ColaPiezas* cola, Pieza pieza);
Pieza desencolarPieza(ColaPiezas* cola);
bool colaPiezasVacia(const ColaPiezas* cola);

void generarNuevaBolsa(ColaPiezas* cola);

void piezasSuficientes(ColaPiezas* cola, int minimoSuficiente);

void proximasPiezas(const ColaPiezas* cola, Pieza* destino, int cantidad);

#endif
