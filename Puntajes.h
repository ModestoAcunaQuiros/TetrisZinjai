#ifndef PUNTAJES_H
#define PUNTAJES_H

// Tabla de mejores puntajes guardada en archivo.

const int MAX_PUNTAJES = 10;
const int MAX_LONGITUD_NOMBRE_PUNTAJE = 20;
const char* const ARCHIVO_PUNTAJES = "data/puntajes.txt";

struct RegistroPuntaje {
	char nombre[MAX_LONGITUD_NOMBRE_PUNTAJE];
	int puntaje;
};

int cargarPuntajes(RegistroPuntaje* destino, int maximo);
bool guardarPuntajes(const RegistroPuntaje* registros, int cantidad);
bool calificaEnTop(const RegistroPuntaje* registros, int cantidad, int puntaje);
int agregarPuntaje(RegistroPuntaje* registros, int cantidad,
				   const char* nombre, int puntaje);

#endif
