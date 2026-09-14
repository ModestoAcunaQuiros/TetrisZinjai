#include "Ordenamiento.h"
#include <cstdlib>
#include <ctime>

// ---------------------------------------------------------------------------
// Insercion: O(n^2). Recorre el arreglo insertando cada elemento en su lugar
// dentro de la parte ya ordenada (descendente por puntaje).
// ---------------------------------------------------------------------------
static void ordenarInsercion(RegistroPuntaje* v, int n) {
	for (int i = 1; i < n; i++) {
		RegistroPuntaje clave = v[i];
		int j = i - 1;
		while (j >= 0 && v[j].puntaje < clave.puntaje) {
			v[j + 1] = v[j];
			j--;
		}
		v[j + 1] = clave;
	}
}

static void intercambiar(RegistroPuntaje* a, RegistroPuntaje* b) {
	RegistroPuntaje temp = *a;
	*a = *b;
	*b = temp;
}

// ---------------------------------------------------------------------------
// Quicksort: O(n log n) promedio. Particion de Hoare con pivote central.
// Ordena de mayor a menor puntaje.
// ---------------------------------------------------------------------------
static void quickSortRecursivo(RegistroPuntaje* v, int izquierda, int derecha) {
	int i = izquierda;
	int j = derecha;
	int pivote = v[(izquierda + derecha) / 2].puntaje;

	while (i <= j) {
		while (v[i].puntaje > pivote) i++;
		while (v[j].puntaje < pivote) j--;
		if (i <= j) {
			intercambiar(&v[i], &v[j]);
			i++;
			j--;
		}
	}
	if (izquierda < j) quickSortRecursivo(v, izquierda, j);
	if (i < derecha) quickSortRecursivo(v, i, derecha);
}

void ordenarPuntajes(RegistroPuntaje* registros, int cantidad,
					 AlgoritmoOrdenamiento algoritmo) {
	if (cantidad < 2) {
		return;
	}
	if (algoritmo == ORDEN_QUICKSORT) {
		quickSortRecursivo(registros, 0, cantidad - 1);
	} else {
		ordenarInsercion(registros, cantidad);
	}
}

const char* nombreAlgoritmo(AlgoritmoOrdenamiento algoritmo) {
	if (algoritmo == ORDEN_QUICKSORT) {
		return "Quicksort O(n log n)";
	}
	return "Insercion O(n^2)";
}

// ---------------------------------------------------------------------------
// Medicion empirica: genera "cantidad" registros aleatorios y mide el tiempo
// de cada algoritmo por separado (se ordena una copia distinta en cada uno).
// ---------------------------------------------------------------------------
ResultadoTiempo medirTiempos(int cantidad) {
	ResultadoTiempo resultado;
	resultado.cantidad = cantidad;
	resultado.msInsercion = 0.0;
	resultado.msQuicksort = 0.0;
	if (cantidad <= 0) {
		return resultado;
	}

	RegistroPuntaje* original = new RegistroPuntaje[cantidad];
	RegistroPuntaje* copia = new RegistroPuntaje[cantidad];
	for (int i = 0; i < cantidad; i++) {
		// Puntajes aleatorios dentro de un rango amplio.
		original[i].puntaje = rand() % 1000000;
		original[i].nombre[0] = 'T';
		original[i].nombre[1] = '\0';
	}

	// Insercion
	for (int i = 0; i < cantidad; i++) copia[i] = original[i];
	std::clock_t inicio = std::clock();
	ordenarInsercion(copia, cantidad);
	std::clock_t fin = std::clock();
	resultado.msInsercion = 1000.0 * (fin - inicio) / CLOCKS_PER_SEC;

	// Quicksort
	for (int i = 0; i < cantidad; i++) copia[i] = original[i];
	inicio = std::clock();
	quickSortRecursivo(copia, 0, cantidad - 1);
	fin = std::clock();
	resultado.msQuicksort = 1000.0 * (fin - inicio) / CLOCKS_PER_SEC;

	delete[] original;
	delete[] copia;
	return resultado;
}
