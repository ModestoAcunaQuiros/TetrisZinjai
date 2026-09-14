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
// (Se llama EstadoReplay para no chocar con el enum EstadoJuego de la interfaz.)
struct EstadoReplay {
	int celdas[ALTO_TABLERO][ANCHO_TABLERO]; // copia plana del tablero
	Pieza piezaActiva;
	int puntaje;
};

struct NodoMovimiento {
	TipoMovimiento tipo;
	EstadoReplay estado; // estado del juego DESPUES de aplicar este movimiento
	NodoMovimiento* anterior;
	NodoMovimiento* siguiente;
};

// Lista doblemente enlazada propia. NO usar std::list.
struct ListaReplay {
	NodoMovimiento* primero;
	NodoMovimiento* ultimo;
	NodoMovimiento* actual;     // nullptr = "estamos en estadoInicial" (antes de cualquier movimiento)
	EstadoReplay estadoInicial; // foto del juego antes del primer movimiento
	int cantidad;
};

void inicializarReplay(ListaReplay* lista, const EstadoReplay* estadoInicial);
void destruirReplay(ListaReplay* lista);

// Registra un nuevo movimiento a partir de "actual". IMPORTANTE: si el
// jugador habia deshecho movimientos y "actual" no es el ultimo nodo,
// se descarta todo lo que estaba "adelante" antes de insertar el nuevo
// (se pierde el redo de una rama vieja, como en cualquier editor de texto).
void registrarMovimiento(ListaReplay* lista, TipoMovimiento tipo, const EstadoReplay* nuevoEstado);

// Retrocede un paso. Si tiene exito, copia en "destino" el estado al que
// hay que volver. Devuelve false si ya se esta en el estadoInicial.
bool deshacerMovimiento(ListaReplay* lista, EstadoReplay* destino);

// Avanza un paso. Si tiene exito, copia en "destino" el estado al que
// hay que avanzar. Devuelve false si ya se esta en el ultimo nodo.
bool rehacerMovimiento(ListaReplay* lista, EstadoReplay* destino);

// Reinicia el cursor de reproduccion al estado inicial (usar antes de
// reproducir la partida completa desde el principio con rehacerMovimiento
// en un bucle).
void iniciarReproduccion(ListaReplay* lista);

// Copia el estado actual del tablero/pieza/puntaje a una instantanea.
void capturarEstado(const Tablero* t, const Pieza* p, int puntaje, EstadoReplay* destino);

// Restaura el tablero/pieza/puntaje a partir de una instantanea.
void aplicarEstado(Tablero* t, Pieza* p, int* puntaje, const EstadoReplay* origen);

#endif
