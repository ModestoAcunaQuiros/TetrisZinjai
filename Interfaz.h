#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <SFML/Graphics.hpp>

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
	
	char nombreJugador[INTERFAZ_MAX_LONGITUD_NOMBRE];
};

void inicializarInterfaz(ContextoInterfaz* ctx, sf::RenderWindow* ventana);

EstadoJuego pantallaMenu(ContextoInterfaz* ctx);
EstadoJuego pantallaIngresarNombre(ContextoInterfaz* ctx);
EstadoJuego pantallaTablaPuntajes(ContextoInterfaz* ctx); // placeholder por ahora

#endif
