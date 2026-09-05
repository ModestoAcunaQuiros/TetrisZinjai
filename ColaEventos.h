#ifndef COLAEVENTOS_H
#define COLAEVENTOS_H

enum TipoDeEventos{
	aumentarVelocidad,
	piezaBomba,
	congelarControl,
	pantallaInvertida
};

struct Evento{
	float momentoEvento;
	TipoDeEventos evento;
};

struct NodoEvento{
	Evento dato;
	NodoEvento* siguiente;
};

struct ColaEventos{
	int cantidad;
	NodoEvento* frente;
};

void inicializarColaEventos(ColaEventos* cola);
void destruirColaDeEventos(ColaEventos* cola);

void programarEvento(ColaEventos* cola, Evento evento);
bool eventosVacio(ColaEventos* cola);
bool eventolisto(ColaEventos* cola, float tiempoActual);

Evento extraeEvento(ColaEventos* cola);


#endif
