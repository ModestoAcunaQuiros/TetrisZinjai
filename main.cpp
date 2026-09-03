#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Interfaz.h"
#include "Juego.h" 
#include <iostream>

int main() {
	sf::RenderWindow ventana(sf::VideoMode(720, 540), "Tetris");
	ventana.setFramerateLimit(60);
	
	sf::Music musica;
	bool musicaCargada = musica.openFromFile("audio/OTS_Tetris.ogg");
	if (!musicaCargada) {
		std::cout << "no se pudo cargar audio/OTS_Tetris.ogg\n";
	} else {
		musica.setLoop(true); 
		musica.play();
	}
	
	ContextoInterfaz ctx;
	inicializarInterfaz(&ctx, &ventana);
	
	EstadoJuego estado = ESTADO_MENU;
	
	while (estado != ESTADO_SALIR) {
		switch (estado) {
		case ESTADO_MENU:
			estado = pantallaMenu(&ctx);
			break;
			
		case ESTADO_INGRESAR_NOMBRE:
			estado = pantallaIngresarNombre(&ctx);
			break;
			
		case ESTADO_TABLA_PUNTAJES:
			estado = pantallaTablaPuntajes(&ctx);
			break;
			
		case ESTADO_JUGANDO:
			estado = jugarPartida(&ctx);
			break;
			
		default:
			estado = ESTADO_SALIR;
			break;
		}
	}
	
	return 0;
}
