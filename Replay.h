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

// Fotografia completa del estado del juego en un momento dado.
struct EstadoJuego {
	int celdas[ALTO_TABLERO][ANCHO_TABLERO]; // copia plana del tablero
	Pieza piezaActiva;
	int puntaje;
};

struct NodoMovimiento {
	TipoMovimiento tipo;
	EstadoJuego estado; // estado del juego DESPUES de aplicar este movimiento
	NodoMovimiento* anterior;
	NodoMovimiento* siguiente;
};

// Lista doblemente enlazada propia. NO usar std::list.
struct ListaReplay {
	NodoMovimiento* primero;
	NodoMovimiento* ultimo;
	NodoMovimiento* actual;     // nullptr = "estamos en estadoInicial" (antes de cualquier movimiento)
	EstadoJuego estadoInicial;  // foto del juego antes del primer movimiento
	int cantidad;
};

void inicializarReplay(ListaReplay* lista, const EstadoJuego* estadoInicial);
void destruirReplay(ListaReplay* lista);

// Registra un nuevo movimiento a partir de "actual". IMPORTANTE: si el
// jugador habia deshecho movimientos y "actual" no es el ultimo nodo,
// se descarta todo lo que estaba "adelante" antes de insertar el nuevo
// (se pierde el redo de una rama vieja, como en cualquier editor de texto).
void registrarMovimiento(ListaReplay* lista, TipoMovimiento tipo, const EstadoJuego* nuevoEstado);

// Retrocede un paso. Si tiene exito, copia en "destino" el estado al que
// hay que volver. Devuelve false si ya se esta en el estadoInicial.
bool deshacerMovimiento(ListaReplay* lista, EstadoJuego* destino);

// Avanza un paso. Si tiene exito, copia en "destino" el estado al que
// hay que avanzar. Devuelve false si ya se esta en el ultimo nodo.
bool rehacerMovimiento(ListaReplay* lista, EstadoJuego* destino);

// Reinicia el cursor de reproduccion al estado inicial (usar antes de
// reproducir la partida completa desde el principio con rehacerMovimiento
// en un bucle).
void iniciarReproduccion(ListaReplay* lista);

#endif
