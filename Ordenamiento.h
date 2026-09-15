#ifndef ORDENAMIENTO_H
#define ORDENAMIENTO_H

#include "Puntajes.h"

enum AlgoritmoOrdenamiento {
	ORDEN_INSERCION = 0,
	ORDEN_QUICKSORT,
	CANTIDAD_ALGORITMOS
};

// Ordena los registros de mayor a menor puntaje.
void ordenarPuntajes(RegistroPuntaje* registros, int cantidad,
					 AlgoritmoOrdenamiento algoritmo);

const char* nombreAlgoritmo(AlgoritmoOrdenamiento algoritmo);

// Mide el tiempo de cada algoritmo para N registros aleatorios.
struct ResultadoTiempo {
	int cantidad;
	double msInsercion;
	double msQuicksort;
};

ResultadoTiempo medirTiempos(int cantidad);

#endif
