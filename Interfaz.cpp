#include "Interfaz.h"
#include <iostream>
#include <cstring>
using namespace std;

void inicializarInterfaz(ContextoInterfaz* ctx, sf::RenderWindow* ventana) {
	ctx->ventana = ventana;
	
	ctx->fuenteCargada = ctx->fuente.loadFromFile("data/deer-diary.ttf");
	if (!ctx->fuenteCargada) {
		cout << "[interfaz] Aviso: no se pudo cargar data/fuente.ttf.\n";
	}
	ctx->texturaMenuCargada = ctx->texturaMenu.loadFromFile("data/Menu1.png");
	
	if (!ctx->texturaMenuCargada) {
		cout << "[interfaz] Aviso: no se pudo cargar data/menu_fondo.png.\n";
	} else {
		ctx->texturaMenu.setSmooth(false);
		// Ajustamos la ventana al tamano REAL de la imagen, para que no
		// se recorte ni queden bordes negros de sobra.
		sf::Vector2u tamanoReal = ctx->texturaMenu.getSize();
		//std::cout << "[interfaz] menu_fondo.png mide " << tamanoReal.x << "x" << tamanoReal.y << "\n";
		ctx->ventana->setSize(tamanoReal);
	}
	
	ctx->nombreJugador[0] = '\0';
}

// ---------------------------------------------------------------------
// MENU PRINCIPAL
// ---------------------------------------------------------------------
EstadoJuego pantallaMenu(ContextoInterfaz* ctx) {
	sf::Sprite fondo;
	if (ctx->texturaMenuCargada) {
		fondo.setTexture(ctx->texturaMenu);
	}
	
	sf::Vector2u tam = ctx->texturaMenu.getSize();
	
	// Zonas como PORCENTAJE del tamano real de la imagen (calibradas a ojo
	// sobre una imagen de referencia de 1067x676; deberia mantenerse
	// proporcional aunque el archivo real tenga otra resolucion).
	sf::FloatRect zonaJugar(tam.x * 0.2717f, tam.y * 0.4512f, tam.x * 0.3983f, tam.y * 0.1553f);
	sf::FloatRect zonaHistorico(tam.x * 0.2717f, tam.y * 0.6583f, tam.x * 0.3983f, tam.y * 0.1479f);
	
	// ... el resto del while queda exactamente igual que antes
	
	while (ctx->ventana->isOpen()) {
		sf::Event evento;
		while (ctx->ventana->pollEvent(evento)) {
			if (evento.type == sf::Event::Closed) {
				ctx->ventana->close();
				return ESTADO_SALIR;
			}
			
			if (evento.type == sf::Event::MouseButtonPressed &&
				evento.mouseButton.button == sf::Mouse::Left) {
				sf::Vector2f click(static_cast<float>(evento.mouseButton.x),
								   static_cast<float>(evento.mouseButton.y));
				if (zonaJugar.contains(click)) {
					return ESTADO_INGRESAR_NOMBRE;
				}
				if (zonaHistorico.contains(click)) {
					return ESTADO_TABLA_PUNTAJES;
				}
			}
			
			if (evento.type == sf::Event::KeyPressed) {
				if (evento.key.code == sf::Keyboard::Return) {
					return ESTADO_INGRESAR_NOMBRE;
				}
				if (evento.key.code == sf::Keyboard::H) {
					return ESTADO_TABLA_PUNTAJES;
				}
				if (evento.key.code == sf::Keyboard::Escape) {
					ctx->ventana->close();
					return ESTADO_SALIR;
				}
			}
		}
		
		ctx->ventana->clear(sf::Color::Black);
		if (ctx->texturaMenuCargada) {
			ctx->ventana->draw(fondo);
		}
		ctx->ventana->display();
	}
	
	return ESTADO_SALIR;
}

// ---------------------------------------------------------------------
// INGRESAR NOMBRE
// ---------------------------------------------------------------------
EstadoJuego pantallaIngresarNombre(ContextoInterfaz* ctx) {
	std::string nombreActual = "";
	sf::Vector2u tamanoVentana = ctx->ventana->getSize();
	
	while (ctx->ventana->isOpen()) {
		sf::Event evento;
		while (ctx->ventana->pollEvent(evento)) {
			if (evento.type == sf::Event::Closed) {
				ctx->ventana->close();
				return ESTADO_SALIR;
			}
			if (evento.type == sf::Event::KeyPressed) {
				if (evento.key.code == sf::Keyboard::Escape) {
					return ESTADO_MENU;
				}
				if (evento.key.code == sf::Keyboard::Return && !nombreActual.empty()) {
					strncpy(ctx->nombreJugador, nombreActual.c_str(), INTERFAZ_MAX_LONGITUD_NOMBRE - 1);
					ctx->nombreJugador[INTERFAZ_MAX_LONGITUD_NOMBRE - 1] = '\0';
					return ESTADO_JUGANDO;
				}
				if (evento.key.code == sf::Keyboard::BackSpace && !nombreActual.empty()) {
					nombreActual.erase(nombreActual.size() - 1);
				}
			}
			if (evento.type == sf::Event::TextEntered) {
				unsigned int codigo = evento.text.unicode;
				bool esValido = (codigo >= 'a' && codigo <= 'z') ||
					(codigo >= 'A' && codigo <= 'Z') ||
					(codigo >= '0' && codigo <= '9') ||
					codigo == '_';
				if (esValido && nombreActual.size() < static_cast<size_t>(INTERFAZ_MAX_LONGITUD_NOMBRE - 1)) {
					nombreActual += static_cast<char>(codigo);
				}
			}
		}
		
		ctx->ventana->clear(sf::Color(20, 20, 30));
		
		if (ctx->fuenteCargada) {
			sf::Text etiqueta("Ingresa tu nombre:", ctx->fuente, 28);
			etiqueta.setColor(sf::Color::White); // en 2.4.x es setColor, no setFillColor
			sf::FloatRect limitesEtq = etiqueta.getLocalBounds();
			etiqueta.setPosition(tamanoVentana.x / 2.f - limitesEtq.width / 2.f, 200.f);
			ctx->ventana->draw(etiqueta);
			
			sf::Text texto(nombreActual + "_", ctx->fuente, 32);
			texto.setColor(sf::Color::Yellow);
			sf::FloatRect limites = texto.getLocalBounds();
			texto.setPosition(tamanoVentana.x / 2.f - limites.width / 2.f, 260.f);
			ctx->ventana->draw(texto);
			
			sf::Text ayuda("Enter para confirmar, Esc para volver", ctx->fuente, 16);
			ayuda.setColor(sf::Color(150, 150, 150));
			sf::FloatRect limitesAyuda = ayuda.getLocalBounds();
			ayuda.setPosition(tamanoVentana.x / 2.f - limitesAyuda.width / 2.f, 340.f);
			ctx->ventana->draw(ayuda);
		}
		
		ctx->ventana->display();
	}
	
	return ESTADO_SALIR;
}

// ---------------------------------------------------------------------
// TABLA DE MEJORES PUNTAJES (placeholder hasta que exista Puntaje.h/.cpp)
// ---------------------------------------------------------------------
EstadoJuego pantallaTablaPuntajes(ContextoInterfaz* ctx) {
	sf::Vector2u tamanoVentana = ctx->ventana->getSize();
	
	while (ctx->ventana->isOpen()) {
		sf::Event evento;
		while (ctx->ventana->pollEvent(evento)) {
			if (evento.type == sf::Event::Closed) {
				ctx->ventana->close();
				return ESTADO_SALIR;
			}
			if (evento.type == sf::Event::KeyPressed) {
				if (evento.key.code == sf::Keyboard::Escape || evento.key.code == sf::Keyboard::Return) {
					return ESTADO_MENU;
				}
			}
		}
		
		ctx->ventana->clear(sf::Color(20, 20, 30));
		
		if (ctx->fuenteCargada) {
			sf::Text titulo("MEJORES PUNTAJES", ctx->fuente, 32);
			titulo.setColor(sf::Color::White);
			sf::FloatRect limites = titulo.getLocalBounds();
			titulo.setPosition(tamanoVentana.x / 2.f - limites.width / 2.f, 50.f);
			ctx->ventana->draw(titulo);
			
			sf::Text aviso("Modulo de puntajes aun no implementado.", ctx->fuente, 20);
			aviso.setColor(sf::Color(180, 180, 180));
			sf::FloatRect limitesAviso = aviso.getLocalBounds();
			aviso.setPosition(tamanoVentana.x / 2.f - limitesAviso.width / 2.f, 150.f);
			ctx->ventana->draw(aviso);
			
			sf::Text ayuda("Enter o Esc para volver al menu", ctx->fuente, 16);
			ayuda.setColor(sf::Color(150, 150, 150));
			sf::FloatRect limitesAyuda = ayuda.getLocalBounds();
			ayuda.setPosition(tamanoVentana.x / 2.f - limitesAyuda.width / 2.f, 520.f);
			ctx->ventana->draw(ayuda);
		}
		
		ctx->ventana->display();
	}
	
	return ESTADO_SALIR;
}
