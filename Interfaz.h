#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <SFML/Graphics.hpp>

#include "Tablero.h"
#include "Pieza.h"
#include "ColaPieza.h"
#include "PilaHold.h"

namespace sf {
	class Sound; // solo puntero; los metodos se usan donde hay Audio.hpp
}

const int INTERFAZ_MAX_LONGITUD_NOMBRE = 20;

enum EstadoJuego {
	ESTADO_MENU,
	ESTADO_INGRESAR_NOMBRE,
	ESTADO_TABLA_PUNTAJES,
	ESTADO_JUGANDO,
	ESTADO_PAUSA,
	ESTADO_GAMEOVER,
	ESTADO_REPLAY,
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
};

void inicializarInterfaz(ContextoInterfaz* ctx, sf::RenderWindow* ventana);

EstadoJuego pantallaMenu(ContextoInterfaz* ctx);
EstadoJuego pantallaIngresarNombre(ContextoInterfaz* ctx);
EstadoJuego pantallaTablaPuntajes(ContextoInterfaz* ctx); // placeholder por ahora

sf::Color colorPieza(TipoPieza pieza);
void dibujarFondoEscenario(ContextoInterfaz* ctx, float impulso = 0.f);
void dibujarPanelChamfer(ContextoInterfaz* ctx, float x, float y, float ancho, float alto,
                         sf::Color relleno, sf::Color borde);
void dibujarBloque(sf::RenderWindow* ventana, float x, float y, float lado, sf::Color color);
void dibujarTablero(ContextoInterfaz* ctx, const Tablero* tablero, const Pieza* piezaActiva,float origenX, float origenY, float celda, bool espejo = false);
void dibujarPanelSiguientes(ContextoInterfaz* ctx, const Pieza* proximas, int cantidad, float x, float y);
void dibujarPanelHold(ContextoInterfaz* ctx, const PilaHold* hold, float x, float y);
void dibujarPanelPuntaje(ContextoInterfaz* ctx, int puntaje, float x, float y);

#endif
