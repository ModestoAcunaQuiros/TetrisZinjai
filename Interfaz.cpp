#include "Interfaz.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
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
		ctx->ventana->setSize(tamanoReal);
	}
	
	ctx->nombreJugador[0] = '\0';
}

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

sf::Color colorPieza(TipoPieza pieza){
	switch (pieza){
		case PIEZA_I: return sf::Color(0, 226, 255);
		case PIEZA_O: return sf::Color(255, 213, 0);
		case PIEZA_T: return sf::Color(176, 64, 255);
		case PIEZA_S: return sf::Color(0, 230, 118);
		case PIEZA_Z: return sf::Color(255, 61, 82);
		case PIEZA_J: return sf::Color(30, 120, 255);
		case PIEZA_L: return sf::Color(255, 146, 26);
		default: return sf::Color::White;
	}
}
	
static sf::Color aclarar(sf:: Color color, int cantidad){
	return sf::Color(
		static_cast<sf::Uint8>(std::min(255, color.r + cantidad)),
		static_cast<sf::Uint8>(std::min(255, color.g + cantidad)),
		static_cast<sf::Uint8>(std::min(255, color.b + cantidad))
		);
}
	
static sf::Color oscurecer(sf::Color color, int cantidad){
	return sf::Color(
		static_cast<sf::Uint8>(std::max(0, color.r - cantidad)),
		static_cast<sf::Uint8>(std::max(0, color.g - cantidad)),
		static_cast<sf::Uint8>(std::max(0, color.b - cantidad))
		);
}

// Calcula la fila a la que caeria la pieza si se dejara caer en ese momento
// (proyeccion "fantasma" hacia abajo). Devuelve la ultima fila valida.
static int calcularFilaAterrizaje(const Tablero* tablero, const Pieza* pieza){
	const int* forma = obtenerFormaPieza(pieza->tipo, pieza->orientacion);
	int fila = pieza->filaOrigen;
	while (fila < ALTO_TABLERO) {
		bool bloqueada = false;
		for (int r = 0; r < 4 && !bloqueada; r++) {
			for (int c = 0; c < 4; c++) {
				if (forma[r * 4 + c]) {
					int filaAbs = fila + r;
					int colAbs = pieza->colOrigen + c;
					if (filaAbs >= ALTO_TABLERO ||
						colAbs < 0 || colAbs >= ANCHO_TABLERO ||
						(filaAbs >= 0 && !celdaLibre(tablero, filaAbs, colAbs))) {
						bloqueada = true;
						break;
					}
				}
			}
		}
		if (bloqueada) break;
		fila++;
	}
	return fila - 1;
}

void dibujarBloque(sf::RenderWindow* ventana, float x, float y, float lado, sf::Color color) {
	// Estilo pixel art: celda separada por linea oscura, sombreado a 2 tonos
	// y brillo "specular" en la esquina, sin degradados suaves.
	float g = std::max(1.0f, std::round(lado * 0.06f));
	float s = lado - 2.f * g;
	float px = x + g;
	float py = y + g;

	// Cuerpo del bloque (colores planos, borde duro).
	sf::RectangleShape cuerpo(sf::Vector2f(s, s));
	cuerpo.setPosition(px, py);
	cuerpo.setFillColor(color);
	ventana->draw(cuerpo);

	// Sombreado pixel: luz arriba/izquierda, sombra abajo/derecha.
	float t = std::max(1.0f, std::round(s * 0.14f));
	sf::Color tonoLuz = aclarar(color, 65);
	sf::Color tonoSombra = oscurecer(color, 90);

	sf::RectangleShape luzArriba(sf::Vector2f(s, t));
	luzArriba.setPosition(px, py);
	luzArriba.setFillColor(tonoLuz);
	ventana->draw(luzArriba);

	sf::RectangleShape luzIzquierda(sf::Vector2f(t, s));
	luzIzquierda.setPosition(px, py);
	luzIzquierda.setFillColor(tonoLuz);
	ventana->draw(luzIzquierda);

	sf::RectangleShape sombraDerecha(sf::Vector2f(t, s));
	sombraDerecha.setPosition(px + s - t, py);
	sombraDerecha.setFillColor(tonoSombra);
	ventana->draw(sombraDerecha);

	sf::RectangleShape sombraAbajo(sf::Vector2f(s, t));
	sombraAbajo.setPosition(px, py + s - t);
	sombraAbajo.setFillColor(tonoSombra);
	ventana->draw(sombraAbajo);

	// Pixel de brillo en la esquina (efecto "pixel glossy").
	if (lado >= 8.f) {
		float especular = s * 0.22f;
		sf::RectangleShape brillo(sf::Vector2f(especular, especular));
		brillo.setPosition(px + t, py + t);
		brillo.setFillColor(sf::Color(255, 255, 255, 70));
		ventana->draw(brillo);
	}
}

static void dibujarFondoPanel(ContextoInterfaz* ctx, float x, float y, float ancho, float alto) {
	sf::RectangleShape panel(sf::Vector2f(ancho, alto));
	panel.setPosition(x, y);
	panel.setFillColor(sf::Color(16, 22, 38));
	panel.setOutlineThickness(2.f);
	panel.setOutlineColor(sf::Color(50, 72, 112));
	ctx->ventana->draw(panel);
}

// Fondo de la pantalla de juego con estetica retro: fondo plano oscuro,
// scanlines de monitor y pixeles decorativos de los colores de las piezas.
void dibujarFondoEscenario(ContextoInterfaz* ctx) {
	sf::Vector2u dim = ctx->ventana->getSize();
	float ancho = static_cast<float>(dim.x);
	float alto = static_cast<float>(dim.y);

	sf::RectangleShape fondo(sf::Vector2f(ancho, alto));
	fondo.setPosition(0.f, 0.f);
	fondo.setFillColor(sf::Color(11, 14, 26));
	ctx->ventana->draw(fondo);

	// Scanlines sutiles estilo CRT.
	for (float y = 0.f; y < alto; y += 5.f) {
		sf::RectangleShape scanline(sf::Vector2f(ancho, 1.f));
		scanline.setPosition(0.f, y);
		scanline.setFillColor(sf::Color(0, 0, 0, 28));
		ctx->ventana->draw(scanline);
	}

	// Columnas de pixeles con los 7 colores del juego en los costados.
	const float cajita = 8.f;
	const float paso = 16.f;
	sf::Color colorActual;
	int indice = 0;
	for (float y = alto * 0.10f; y < alto * 0.90f; y += paso) {
		TipoPieza tipo = static_cast<TipoPieza>(indice % CANTIDAD_TIPOS_PIEZA);
		colorActual = colorPieza(tipo);
		colorActual.a = 180;
		indice++;

		sf::RectangleShape lateralIzq(sf::Vector2f(cajita, cajita));
		lateralIzq.setPosition(10.f, y);
		lateralIzq.setFillColor(colorActual);
		ctx->ventana->draw(lateralIzq);

		sf::RectangleShape lateralDer(sf::Vector2f(cajita, cajita));
		lateralDer.setPosition(ancho - 10.f - cajita, y);
		lateralDer.setFillColor(colorActual);
		ctx->ventana->draw(lateralDer);
	}
}

void dibujarTablero(ContextoInterfaz* ctx, const Tablero* tablero, const Pieza* piezaActual, float origenEnX, float origenEnY, float celda){
	float anchoTab = ANCHO_TABLERO * celda;
	float altoTab = ALTO_TABLERO * celda;
	const float bisel = 12.f;
	const float ribete = 5.f;

	// Sombra pixel (bloque negro solido, sin degradados).
	sf::RectangleShape sombra(sf::Vector2f(anchoTab + bisel * 2.f + 8.f, altoTab + bisel * 2.f + 8.f));
	sombra.setPosition(origenEnX - bisel + 7.f, origenEnY - bisel + 9.f);
	sombra.setFillColor(sf::Color(3, 5, 10, 200));
	ctx->ventana->draw(sombra);

	// Carcasa del tablero (marco grueso).
	sf::RectangleShape carcasa(sf::Vector2f(anchoTab + bisel * 2.f, altoTab + bisel * 2.f));
	carcasa.setPosition(origenEnX - bisel, origenEnY - bisel);
	carcasa.setFillColor(sf::Color(18, 25, 45));
	carcasa.setOutlineThickness(2.f);
	carcasa.setOutlineColor(sf::Color(6, 9, 16));
	ctx->ventana->draw(carcasa);

	// Ribete interior de color.
	sf::RectangleShape bordeColor(sf::Vector2f(anchoTab + bisel * 2.f - ribete * 2.f, altoTab + bisel * 2.f - ribete * 2.f));
	bordeColor.setPosition(origenEnX - bisel + ribete, origenEnY - bisel + ribete);
	bordeColor.setFillColor(sf::Color::Transparent);
	bordeColor.setOutlineThickness(2.f);
	bordeColor.setOutlineColor(sf::Color(62, 88, 134));
	ctx->ventana->draw(bordeColor);

	// Pantalla del juego.
	sf::RectangleShape pantalla(sf::Vector2f(anchoTab, altoTab));
	pantalla.setPosition(origenEnX, origenEnY);
	pantalla.setFillColor(sf::Color(8, 11, 20));
	pantalla.setOutlineThickness(1.f);
	pantalla.setOutlineColor(sf::Color(2, 3, 7));
	ctx->ventana->draw(pantalla);

	// Rejilla muy tenue de celdas.
	for (int i = 1; i < ANCHO_TABLERO; i++) {
		sf::RectangleShape linea(sf::Vector2f(1.f, altoTab));
		linea.setPosition(origenEnX + i * celda, origenEnY);
		linea.setFillColor(sf::Color(255, 255, 255, 8));
		ctx->ventana->draw(linea);
	}
	for (int j = 1; j < ALTO_TABLERO; j++) {
		sf::RectangleShape linea(sf::Vector2f(anchoTab, 1.f));
		linea.setPosition(origenEnX, origenEnY + j * celda);
		linea.setFillColor(sf::Color(255, 255, 255, 6));
		ctx->ventana->draw(linea);
	}

	// Celdas ya ocupadas (piezas fijadas en el tablero).
	NodoFila* fila = obtenerFila(tablero, 0);
	int indice = 0;
	while (fila != nullptr) {
		for (int col = 0; col < ANCHO_TABLERO; col++) {
			if (fila->celdas[col] != 0) {
				TipoPieza tipo = static_cast<TipoPieza>(fila->celdas[col] - 1);
				dibujarBloque(ctx->ventana, origenEnX + col * celda, origenEnY + indice * celda, celda, colorPieza(tipo));
			}
		}
		fila = fila->siguiente;
		indice++;
	}

	// "Fantasma": contorno donde aterrizaria la pieza activa.
	if (piezaActual != nullptr) {
		int filaDestino = calcularFilaAterrizaje(tablero, piezaActual);
		if (filaDestino > piezaActual->filaOrigen) {
			const int* forma = obtenerFormaPieza(piezaActual->tipo, piezaActual->orientacion);
			sf::Color borde = colorPieza(piezaActual->tipo);
			borde.a = 130;
			for (int r = 0; r < 4; r++) {
				for (int c = 0; c < 4; c++) {
					if (forma[r * 4 + c]) {
						int filaAbs = filaDestino + r;
						int colAbs = piezaActual->colOrigen + c;
						if (filaAbs >= 0 && filaAbs < ALTO_TABLERO &&
							colAbs >= 0 && colAbs < ANCHO_TABLERO) {
							sf::RectangleShape celdaFantasma(sf::Vector2f(celda, celda));
							celdaFantasma.setPosition(origenEnX + colAbs * celda, origenEnY + filaAbs * celda);
							celdaFantasma.setFillColor(sf::Color(255, 255, 255, 14));
							celdaFantasma.setOutlineThickness(1.f);
							celdaFantasma.setOutlineColor(borde);
							ctx->ventana->draw(celdaFantasma);
						}
					}
				}
			}
		}
	}

	// Pieza activa: encima del fantasma y de las fijadas.
	if (piezaActual != nullptr) {
		const int* forma = obtenerFormaPieza(piezaActual->tipo, piezaActual->orientacion);
		sf::Color colorActual = colorPieza(piezaActual->tipo);
		for (int r = 0; r < 4; r++) {
			for (int c = 0; c < 4; c++) {
				if (forma[r * 4 + c]) {
					int filaAbs = piezaActual->filaOrigen + r;
					int colAbs = piezaActual->colOrigen + c;
					if (filaAbs >= 0 && filaAbs < ALTO_TABLERO &&
						colAbs >= 0 && colAbs < ANCHO_TABLERO) {
						dibujarBloque(ctx->ventana, origenEnX + colAbs * celda, origenEnY + filaAbs * celda, celda, colorActual);
					}
				}
			}
		}
	}
}

void dibujarPanelSiguientes(ContextoInterfaz* ctx, const Pieza* proxima, int cantidad, float x, float y){
	float anchoPanel = 110.f;
	float altoPanel = 36.f * cantidad + 30.f;
	
	dibujarFondoPanel(ctx, x, y, anchoPanel, altoPanel);
	
	if(ctx->fuenteCargada) {
		sf::Text etiqueta("Siguiente", ctx->fuente, 12);
		etiqueta.setColor(sf::Color(150, 170, 195));
		etiqueta.setPosition(x + 8.f, y + 6.f);
		ctx->ventana->draw(etiqueta);
	}
	float celdaMini = 10.f;
	for(int i = 0; i < cantidad; i++){
		const int* forma = obtenerFormaPieza(proxima[i].tipo, 0);
		sf::Color color = colorPieza(proxima[i].tipo);
		float baseY = y + 26.f + i * 36.f;
		for(int j = 0; j < 4; j++){
			for(int k = 0; k < 4; k++){
				if(forma[j *4 + k]){
					dibujarBloque(ctx->ventana, x + 20 + k * celdaMini, baseY + j * celdaMini, celdaMini, color); 
				}
			}
		}
	}
}
	
void dibujarPanelHold(ContextoInterfaz* ctx, const PilaHold* hold, float x, float y){
	dibujarFondoPanel(ctx, x, y, 90.f, 70.f);
	
	if(ctx->fuenteCargada){
		sf::Text etiqueta("Hold", ctx->fuente, 12);
		etiqueta.setColor(sf::Color(150, 170, 195));
		etiqueta.setPosition(x + 8.f, y + 6.f);
		ctx->ventana->draw(etiqueta);

	}
	if(!pilaHoldVacia(hold)){
		const int* forma = obtenerFormaPieza(hold->tope.tipo, 0);
		sf::Color color = colorPieza(hold->tope.tipo);
		float celdaMini = 12.f;
		float anchoMini = 4.f * celdaMini;
		float xInicio = x + (90.f - anchoMini) * 0.5f;
		float yInicio = y + 24.f;
		for(int i = 0; i < 4; i++){
			for(int j = 0; j < 4; j++){
				if(forma[i * 4 + j]){
					dibujarBloque(ctx->ventana, xInicio + j * celdaMini, yInicio + i * celdaMini, celdaMini, color);
				}
			}
		}
	}
}

void dibujarPanelPuntaje(ContextoInterfaz* ctx, int puntaje, float x, float y){
	dibujarFondoPanel(ctx, x, y, 110.f, 50.f);
	
	if(ctx->fuenteCargada){
		sf::Text etiqueta("Puntos", ctx->fuente, 12);
		etiqueta.setColor(sf::Color(150, 170, 195));
		etiqueta.setPosition(x + 8.f, y + 6.f);
		ctx->ventana->draw(etiqueta);
		
		char textoPuntaje[16];
		snprintf(textoPuntaje, sizeof(textoPuntaje), "%06d", puntaje);
		sf::Text valor(textoPuntaje, ctx->fuente, 20);
		valor.setColor(sf::Color::White);
		valor.setPosition(x + 8.f, y + 24.f);
		ctx->ventana->draw(valor);
	}
}

