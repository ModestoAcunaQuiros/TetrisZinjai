#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <SFML/Graphics.hpp>

#include "Tablero.h"
#include "Pieza.h"
#include "ColaPieza.h"
#include "PilaHold.h"
#include "Ordenamiento.h"
#include "Replay.h"

namespace sf {
	class Sound; // solo puntero; los metodos se usan donde hay Audio.hpp
}

const int INTERFAZ_MAX_LONGITUD_NOMBRE = 20;

// La pausa y el replay NO son estados de este enum: la pausa se maneja con una
// bandera dentro de jugarPartida y el replay se abre desde la pantalla de fin
// de partida. asi cada estado de aqui tiene un case en el switch de main.cpp.

enum EstadoJuego {
	ESTADO_INTRO,
	ESTADO_MENU,
	ESTADO_INGRESAR_NOMBRE,
	ESTADO_TABLA_PUNTAJES,
	ESTADO_JUGANDO,
	ESTADO_GAMEOVER,
	ESTADO_SALIR
};

struct ContextoInterfaz {
	sf::RenderWindow* ventana;
	
	sf::Font fuente;
	bool fuenteCargada;
	
	sf::Texture texturaMenu;
	bool texturaMenuCargada;

	sf::Sound* musicaFondo; // nullptr = no hay musica de fondo
	
	char nombreJugador[INTERFAZ_MAX_LONGITUD_NOMBRE];

	AlgoritmoOrdenamiento algoritmoOrdenamiento;
	int puntajeUltimaPartida;
	ListaReplay* replay;
};

void inicializarInterfaz(ContextoInterfaz* ctx, sf::RenderWindow* ventana);

EstadoJuego pantallaIntro(ContextoInterfaz* ctx);
EstadoJuego pantallaMenu(ContextoInterfaz* ctx);
EstadoJuego pantallaIngresarNombre(ContextoInterfaz* ctx);
EstadoJuego pantallaTablaPuntajes(ContextoInterfaz* ctx);
EstadoJuego pantallaGameOver(ContextoInterfaz* ctx);
EstadoJuego pantallaReplay(ContextoInterfaz* ctx);

// Velo negro que se desvanece al entrar a una pantalla.
void dibujarFundidoEntrada(ContextoInterfaz* ctx, const sf::Clock& reloj, float duracion = 0.4f);

sf::Color colorPieza(TipoPieza pieza);
void dibujarFondoEscenario(ContextoInterfaz* ctx, float impulso = 0.f);
void dibujarPanelChamfer(ContextoInterfaz* ctx, float x, float y, float ancho, float alto,
                         sf::Color relleno, sf::Color borde);
void dibujarBloque(sf::RenderWindow* ventana, float x, float y, float lado, sf::Color color);
void dibujarTablero(ContextoInterfaz* ctx, const Tablero* tablero, const Pieza* piezaActiva,float origenX, float origenY, float celda, bool espejo = false, float desplazamientoPiezaY = 0.f);
void dibujarPanelSiguientes(ContextoInterfaz* ctx, const Pieza* proximas, int cantidad, float x, float y, bool proximaEsBomba = false);
void dibujarPanelHold(ContextoInterfaz* ctx, const PilaHold* hold, float x, float y, bool esBomba = false);
void dibujarPanelPuntaje(ContextoInterfaz* ctx, int puntaje, float x, float y);

#endif
