#ifndef PUNTAJES_H
#define PUNTAJES_H

// Modulo de persistencia de la tabla de mejores puntajes.
// El archivo se guarda siempre ordenado de mayor a menor (el ordenamiento
// lo hace el modulo Ordenamiento con los algoritmos propios).

const int MAX_PUNTAJES = 10;
const int MAX_LONGITUD_NOMBRE_PUNTAJE = 20;
const char* const ARCHIVO_PUNTAJES = "data/puntajes.txt";

struct RegistroPuntaje {
	char nombre[MAX_LONGITUD_NOMBRE_PUNTAJE];
	int puntaje;
};

// Carga hasta "maximo" registros desde ARCHIVO_PUNTAJES.
// Devuelve cuantos registros se cargaron (0 si el archivo no existe).
int cargarPuntajes(RegistroPuntaje* destino, int maximo);

// Escribe "cantidad" registros en ARCHIVO_PUNTAJES (sobrescribe).
// Devuelve false si no se pudo abrir el archivo.
bool guardarPuntajes(const RegistroPuntaje* registros, int cantidad);

// Devuelve true si "puntaje" entra en el top (hay espacio, o supera al
// menor de los 10 guardados).
bool calificaEnTop(const RegistroPuntaje* registros, int cantidad, int puntaje);

// Inserta el registro en el arreglo respetando el maximo de MAX_PUNTAJES.
// Si ya hay 10, reemplaza al de menor puntaje solo si el nuevo es mayor.
// NO ordena; el llamador debe llamar a ordenarPuntajes() despues.
// Devuelve la cantidad de registros resultante.
int agregarPuntaje(RegistroPuntaje* registros, int cantidad,
				   const char* nombre, int puntaje);

#endif
