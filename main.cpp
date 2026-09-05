#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Interfaz.h"
#include "Juego.h" 
#include <iostream>

int main() {
	sf::RenderWindow ventana(sf::VideoMode(720, 540), "Tetris");
	ventana.setFramerateLimit(60);
	
	// La musica se precarga entera en memoria (sf::Sound en vez de sf::Music):
	// evita el streaming en un hilo propio, que en algunos equipos hacia
	// crashear el programa a los pocos segundos de empezar a sonar.
	sf::SoundBuffer bufferMusica;
	sf::Sound musica;
	bool musicaCargada = bufferMusica.loadFromFile("audio/OTS_Tetris.ogg");
	if (!musicaCargada) {
		std::cout << "no se pudo cargar audio/OTS_Tetris.ogg\n";
	} else {
		musica.setBuffer(bufferMusica);
		musica.setLoop(true); 
		musica.play();
	}
	
	ContextoInterfaz ctx;
	inicializarInterfaz(&ctx, &ventana);
	ctx.musicaFondo = musicaCargada ? &musica : nullptr; // para pausarla en ciertas pantallas
	
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
