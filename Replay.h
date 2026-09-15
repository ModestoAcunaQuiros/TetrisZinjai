#ifndef REPLAY_H
#define REPLAY_H

#include "Pieza.h"
#include "Tablero.h"

enum TipoMovimiento {
	MOV_IZQUIERDA,
	MOV_DERECHA,
	MOV_ROTAR,
	MOV_BAJAR,
	MOV_COLOCAR
};

// Foto del estado del juego en un momento dado.
struct EstadoReplay {
	int celdas[ALTO_TABLERO][ANCHO_TABLERO];
	Pieza piezaActiva;
	int puntaje;
};

struct NodoMovimiento {
	TipoMovimiento tipo;
	EstadoReplay estado;
	NodoMovimiento* anterior;
	NodoMovimiento* siguiente;
};

// Lista doblemente enlazada de movimientos.
struct ListaReplay {
	NodoMovimiento* primero;
	NodoMovimiento* ultimo;
	NodoMovimiento* actual;     // nullptr = estado inicial
	EstadoReplay estadoInicial;
	int cantidad;
};

void inicializarReplay(ListaReplay* lista, const EstadoReplay* estadoInicial);
void destruirReplay(ListaReplay* lista);

// Agrega un movimiento despues de "actual"; descarta la rama de redo.
void registrarMovimiento(ListaReplay* lista, TipoMovimiento tipo, const EstadoReplay* nuevoEstado);

bool deshacerMovimiento(ListaReplay* lista, EstadoReplay* destino);
bool rehacerMovimiento(ListaReplay* lista, EstadoReplay* destino);
void iniciarReproduccion(ListaReplay* lista);

void capturarEstado(const Tablero* t, const Pieza* p, int puntaje, EstadoReplay* destino);
void aplicarEstado(Tablero* t, Pieza* p, int* puntaje, const EstadoReplay* origen);

#endif
