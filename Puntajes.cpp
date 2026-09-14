#include "Puntajes.h"
#include <fstream>
#include <iomanip>
#include <cstring>

int cargarPuntajes(RegistroPuntaje* destino, int maximo) {
	std::ifstream archivo(ARCHIVO_PUNTAJES);
	int cantidad = 0;
	if (!archivo.is_open()) {
		return 0; // todavia no hay tabla guardada
	}
	// Formato por linea: nombre puntaje (separados por espacio).
	// Se limita el ancho leido para no desbordar el arreglo del nombre.
	while (cantidad < maximo &&
		   (archivo >> std::setw(MAX_LONGITUD_NOMBRE_PUNTAJE) >> destino[cantidad].nombre
					>> destino[cantidad].puntaje)) {
		destino[cantidad].nombre[MAX_LONGITUD_NOMBRE_PUNTAJE - 1] = '\0';
		cantidad++;
	}
	return cantidad;
}

bool guardarPuntajes(const RegistroPuntaje* registros, int cantidad) {
	std::ofstream archivo(ARCHIVO_PUNTAJES, std::ios::trunc);
	if (!archivo.is_open()) {
		return false;
	}
	for (int i = 0; i < cantidad; i++) {
		archivo << registros[i].nombre << ' ' << registros[i].puntaje << '\n';
	}
	return true;
}

bool calificaEnTop(const RegistroPuntaje* registros, int cantidad, int puntaje) {
	if (cantidad < MAX_PUNTAJES) {
		return true; // aun hay espacio en la tabla
	}
	int menor = registros[0].puntaje;
	for (int i = 1; i < cantidad; i++) {
		if (registros[i].puntaje < menor) {
			menor = registros[i].puntaje;
		}
	}
	return puntaje > menor;
}

static void copiarNombre(char* destino, const char* origen) {
	strncpy(destino, origen, MAX_LONGITUD_NOMBRE_PUNTAJE - 1);
	destino[MAX_LONGITUD_NOMBRE_PUNTAJE - 1] = '\0';
}

int agregarPuntaje(RegistroPuntaje* registros, int cantidad,
				   const char* nombre, int puntaje) {
	if (cantidad < MAX_PUNTAJES) {
		copiarNombre(registros[cantidad].nombre, nombre);
		registros[cantidad].puntaje = puntaje;
		return cantidad + 1;
	}

	// Tabla llena: se busca el menor y se reemplaza si el nuevo es mayor.
	int menorIndice = 0;
	for (int i = 1; i < cantidad; i++) {
		if (registros[i].puntaje < registros[menorIndice].puntaje) {
			menorIndice = i;
		}
	}
	if (puntaje <= registros[menorIndice].puntaje) {
		return cantidad; // no califica, no se toca la tabla
	}
	copiarNombre(registros[menorIndice].nombre, nombre);
	registros[menorIndice].puntaje = puntaje;
	return cantidad;
}
