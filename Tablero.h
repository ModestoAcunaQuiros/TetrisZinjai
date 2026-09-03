#ifndef TABLERO_H
#define TABLERO_H

const int ANCHO_TABLERO = 10;
const int ALTO_TABLERO = 20;

// Cada nodo es una fila de 10 celdas. Internamente usa un arreglo fijo,
// pero la estructura PRINCIPAL del tablero es la lista de estos nodos
// (no un arreglo 2D ni std::vector<std::vector<...>>).
struct NodoFila {
	int celdas[ANCHO_TABLERO]; // 0 = vacia, distinto de 0 = ocupada (color/tipo)
	NodoFila* siguiente;
};

struct Tablero {
	NodoFila* primeraFila; // la fila 0 visualmente (arriba de todo)
	int cantidadFilas;     // deberia mantenerse siempre en ALTO_TABLERO
};

void inicializarTablero(Tablero* t);
void destruirTablero(Tablero* t);

// Devuelve un puntero al nodo de la fila "indice" (0 = arriba).
// Asume 0 <= indice < ALTO_TABLERO.
NodoFila* obtenerFila(const Tablero* t, int indice);

// Revisa si la celda (fila, columna) esta libre y dentro del tablero.
bool celdaLibre(const Tablero* t, int fila, int columna);

// Marca la celda (fila, columna) como ocupada con el valor "valor" (>0).
// Asume que la posicion es valida.
void ocuparCelda(Tablero* t, int fila, int columna, int valor);

// Revisa cuales filas estan completas (todas las celdas != 0).
// Llena "indicesCompletos" (arreglo de tamano >= ALTO_TABLERO) con sus
// posiciones y devuelve cuantas se encontraron.
int detectarFilasCompletas(const Tablero* t, int* indicesCompletos);

// Elimina las filas completas indicadas e inserta la misma cantidad de
// filas vacias AL INICIO de la lista, para que el resto "caiga" gratis
// (solo se mueven punteros, no se copian celdas fila por fila).
void limpiarFilasCompletas(Tablero* t, const int* indicesCompletos, int cantidad);

#endif
