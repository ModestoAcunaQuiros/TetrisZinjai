#ifndef ORDENAMIENTO_H
#define ORDENAMIENTO_H

#include "Puntajes.h"

// Algoritmos de ordenamiento propios (NO se usa std::sort).
enum AlgoritmoOrdenamiento {
	ORDEN_INSERCION = 0,  // O(n^2)
	ORDEN_QUICKSORT,      // O(n log n) promedio
	CANTIDAD_ALGORITMOS
};

// Ordena "registros" de MAYOR a MENOR puntaje usando el algoritmo elegido.
void ordenarPuntajes(RegistroPuntaje* registros, int cantidad,
					 AlgoritmoOrdenamiento algoritmo);

// Nombre legible del algoritmo (para mostrar en pantalla/informe).
const char* nombreAlgoritmo(AlgoritmoOrdenamiento algoritmo);

// Resultado de medir cuanto tarda cada algoritmo en ordenar "cantidad"
// registros de prueba. Sirve para la comparacion empirica del informe.
struct ResultadoTiempo {
	int cantidad;
	double msInsercion;
	double msQuicksort;
};

ResultadoTiempo medirTiempos(int cantidad);

#endif
