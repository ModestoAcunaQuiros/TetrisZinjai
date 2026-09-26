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

// Mide el tiempo de cada algoritmo para N registros aleatorios. Como el reloj
// del sistema es poco preciso, cada algoritmo se repite varias veces y se
// promedia; "repeticiones" dice cuantas se usaron en cada caso.
struct ResultadoTiempo {
	int cantidad;
	double msInsercion;
	double msQuicksort;
	int repeticionesInsercion;
	int repeticionesQuicksort;
};

ResultadoTiempo medirTiempos(int cantidad);

#endif
