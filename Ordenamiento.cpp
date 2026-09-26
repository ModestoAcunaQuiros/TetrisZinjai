#include "Ordenamiento.h"
#include <cstdlib>
#include <ctime>

// Insercion: O(n^2)
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

// Quicksort: O(n log n) promedio
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

// Cuantas repeticiones hacen falta para que el reloj del sistema (que tiene
// granularidad de ~1 ms) alcance a medir algo. Se busca unos 20 ms por corrida.
static int repeticionesSuficientes(RegistroPuntaje* original, RegistroPuntaje* copia,
								   int cantidad, bool usarInsercion) {
	const int MAX_REPETICIONES = 64;
	int repeticiones = 1;
	while (repeticiones < MAX_REPETICIONES) {
		std::clock_t inicio = std::clock();
		for (int r = 0; r < repeticiones; r++) {
			for (int i = 0; i < cantidad; i++) copia[i] = original[i];
			if (usarInsercion) {
				ordenarInsercion(copia, cantidad);
			} else {
				quickSortRecursivo(copia, 0, cantidad - 1);
			}
		}
		std::clock_t fin = std::clock();
		double ms = 1000.0 * (fin - inicio) / CLOCKS_PER_SEC;
		if (ms >= 20.0) {
			return repeticiones;
		}
		repeticiones *= 2;
	}
	return repeticiones;
}

static double cronometrar(RegistroPuntaje* original, RegistroPuntaje* copia,
						  int cantidad, bool usarInsercion, int repeticiones) {
	std::clock_t inicio = std::clock();
	for (int r = 0; r < repeticiones; r++) {
		for (int i = 0; i < cantidad; i++) copia[i] = original[i];
		if (usarInsercion) {
			ordenarInsercion(copia, cantidad);
		} else {
			quickSortRecursivo(copia, 0, cantidad - 1);
		}
	}
	std::clock_t fin = std::clock();
	return 1000.0 * (fin - inicio) / CLOCKS_PER_SEC / repeticiones;
}

ResultadoTiempo medirTiempos(int cantidad) {
	ResultadoTiempo resultado;
	resultado.cantidad = cantidad;
	resultado.msInsercion = 0.0;
	resultado.msQuicksort = 0.0;
	resultado.repeticionesInsercion = 0;
	resultado.repeticionesQuicksort = 0;
	if (cantidad <= 0) {
		return resultado;
	}

	RegistroPuntaje* original = new RegistroPuntaje[cantidad];
	RegistroPuntaje* copia = new RegistroPuntaje[cantidad];
	for (int i = 0; i < cantidad; i++) {
		original[i].puntaje = rand() % 1000000;
		original[i].nombre[0] = 'T';
		original[i].nombre[1] = '\0';
	}

	// Quicksort primero: es el rapido, necesita mas repeticiones para medirse.
	resultado.repeticionesQuicksort = repeticionesSuficientes(original, copia, cantidad, false);
	resultado.msQuicksort = cronometrar(original, copia, cantidad, false, resultado.repeticionesQuicksort);

	resultado.repeticionesInsercion = repeticionesSuficientes(original, copia, cantidad, true);
	resultado.msInsercion = cronometrar(original, copia, cantidad, true, resultado.repeticionesInsercion);

	delete[] original;
	delete[] copia;
	return resultado;
}
