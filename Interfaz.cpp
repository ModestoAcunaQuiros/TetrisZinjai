#include "Interfaz.h"
#include <SFML/Audio.hpp>
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
	ctx->musicaFondo = nullptr; // main.cpp lo conecta si hay musica
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
	
	// Sonido al ingresar a la pantalla de nombre (se carga una sola vez).
	static sf::SoundBuffer bufferIngresar;
	static bool bufferIngresarCargado = false;
	if (!bufferIngresarCargado) {
		bufferIngresarCargado = bufferIngresar.loadFromFile("audio/ingresarNombre.ogg");
		if (!bufferIngresarCargado) {
			cout << "[interfaz] Aviso: no se pudo cargar audio/ingresarNombre.ogg.\n";
		}
	}
	sf::Sound sonidoIngresar;
	if (bufferIngresarCargado) {
		sonidoIngresar.setBuffer(bufferIngresar);
		sonidoIngresar.play();
	}

	// Pausa la musica de fondo para que se oiga bien el audio de esta
	// pantalla; se reanuda al salir de ella.
	bool pausarMusicaDeFondo = false;
	if (ctx->musicaFondo != nullptr &&
		ctx->musicaFondo->getStatus() == sf::SoundSource::Playing) {
		ctx->musicaFondo->pause();
		pausarMusicaDeFondo = true;
	}
	auto reanudarMusicaDeFondo = [&]() {
		if (pausarMusicaDeFondo && ctx->musicaFondo != nullptr) {
			ctx->musicaFondo->play();
			pausarMusicaDeFondo = false;
		}
	};

	while (ctx->ventana->isOpen()) {
		sf::Event evento;
		while (ctx->ventana->pollEvent(evento)) {
			if (evento.type == sf::Event::Closed) {
				ctx->ventana->close();
				reanudarMusicaDeFondo();
				return ESTADO_SALIR;
			}
			if (evento.type == sf::Event::KeyPressed) {
				if (evento.key.code == sf::Keyboard::Escape) {
					reanudarMusicaDeFondo();
					return ESTADO_MENU;
				}
				if (evento.key.code == sf::Keyboard::Return && !nombreActual.empty()) {
					strncpy(ctx->nombreJugador, nombreActual.c_str(), INTERFAZ_MAX_LONGITUD_NOMBRE - 1);
					ctx->nombreJugador[INTERFAZ_MAX_LONGITUD_NOMBRE - 1] = '\0';
					reanudarMusicaDeFondo();
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
	
	reanudarMusicaDeFondo();
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

// Pequeno generador pseudoaleatorio (determinista) para efectos visuales.
static float azar01() {
	static unsigned long long semilla = 88172645463325252ULL;
	semilla = semilla * 6364136223846793005ULL + 1442695040888963407ULL;
	return static_cast<float>((semilla >> 40) & 0xFFFF) / 65535.f;
}

// Dibuja un rectangulo con las esquinas cortadas en diagonal (VertexArray).
static void dibujarRectanguloChamfer(sf::RenderWindow* ventana, float x, float y,
									 float ancho, float alto, float corte, sf::Color color) {
	const int lados = 8;
	sf::VertexArray forma(sf::PrimitiveType::TriangleFan, lados + 2);
	float centroX = x + ancho * 0.5f;
	float centroY = y + alto * 0.5f;

	struct Punto { float px, py; };
	Punto esquinas[lados] = {
		{x + corte, y},               {x + ancho - corte, y},
		{x + ancho, y + corte},       {x + ancho, y + alto - corte},
		{x + ancho - corte, y + alto},{x + corte, y + alto},
		{x, y + alto - corte},        {x, y + corte}
	};

	forma[0].position = sf::Vector2f(centroX, centroY);
	forma[0].color = color;
	for (int i = 0; i < lados; i++) {
		forma[i + 1].position = sf::Vector2f(esquinas[i].px, esquinas[i].py);
		forma[i + 1].color = color;
	}
	forma[lados + 1].position = sf::Vector2f(esquinas[0].px, esquinas[0].py);
	forma[lados + 1].color = color;
	ventana->draw(forma);
}

// Panel principal con esquinas biseladas (exterior = borde, interior = relleno).
void dibujarPanelChamfer(ContextoInterfaz* ctx, float x, float y, float ancho, float alto,
						 sf::Color relleno, sf::Color borde) {
	float corte = std::min(12.f, std::min(ancho, alto) * 0.22f);
	dibujarRectanguloChamfer(ctx->ventana, x - 3.f, y - 3.f, ancho + 6.f, alto + 6.f,
							 corte + 4.f, borde);
	dibujarRectanguloChamfer(ctx->ventana, x, y, ancho, alto, corte, relleno);
}

static void dibujarFondoPanel(ContextoInterfaz* ctx, float x, float y, float ancho, float alto) {
	dibujarPanelChamfer(ctx, x, y, ancho, alto, sf::Color(16, 22, 38), sf::Color(50, 72, 112));
}

// Fondo de la pantalla de juego: cielo nocturno degradado, estrellas lentas,
// nubes con parallax, silueta de ciudad con ventanas y columnas ecualizador.
void dibujarFondoEscenario(ContextoInterfaz* ctx, float impulso) {
	sf::Vector2u dim = ctx->ventana->getSize();
	float ancho = static_cast<float>(dim.x);
	float alto = static_cast<float>(dim.y);

	// --- Cielo degradado (azul oscuro -> violeta -> tono calido) ---
	{
		sf::VertexArray cielo(sf::PrimitiveType::TriangleStrip, 8);
		const float fracciones[4] = {0.f, 0.30f, 0.70f, 1.f};
		sf::Color tonos[4];
		tonos[0] = sf::Color(6, 9, 24);
		tonos[1] = sf::Color(16, 14, 44);
		tonos[2] = sf::Color(44, 26, 62);
		tonos[3] = sf::Color(104, 58, 88);
		for (int i = 0; i < 4; i++) {
			float y = fracciones[i] * alto;
			cielo[2 * i].position = sf::Vector2f(0.f, y);
			cielo[2 * i + 1].position = sf::Vector2f(ancho, y);
			cielo[2 * i].color = tonos[i];
			cielo[2 * i + 1].color = tonos[i];
		}
		ctx->ventana->draw(cielo);
	}

	// Reloj y tiempo globales para todas las capas animadas.
	static sf::Clock relojFondo;
	static float tiempoFondo = 0.f;
	float dt = relojFondo.restart().asSeconds();
	if (dt > 0.05f) dt = 0.05f;
	tiempoFondo += dt;

	// --- Estrellas lejanas (capa profunda, se mueven muy lento) ---
	const int CANTIDAD_ESTRELLAS = 90;
	struct Estrella { float x, y, vel, tam, fase; };
	static Estrella estrellas[CANTIDAD_ESTRELLAS];
	static bool estrellasListas = false;
	if (!estrellasListas) {
		for (int i = 0; i < CANTIDAD_ESTRELLAS; i++) {
			estrellas[i].x = azar01();
			estrellas[i].y = azar01();
			estrellas[i].vel = 0.004f + azar01() * 0.012f; // muy lento (parallax lejano)
			estrellas[i].tam = 0.6f + azar01() * 1.2f;
			estrellas[i].fase = azar01() * 6.2831f;
		}
		estrellasListas = true;
	}
	float tamEscalaEstrella = std::max(1.f, ancho * 0.003f);
	for (int i = 0; i < CANTIDAD_ESTRELLAS; i++) {
		estrellas[i].y -= estrellas[i].vel * dt;
		if (estrellas[i].y < 0.f) {
			estrellas[i].y = 1.02f;
			estrellas[i].x = azar01();
			estrellas[i].vel = 0.004f + azar01() * 0.012f;
		}
		float brillo = 0.55f + 0.45f * sinf(tiempoFondo * 2.f + estrellas[i].fase);
		float tamPx = estrellas[i].tam * tamEscalaEstrella;
		sf::RectangleShape puntito(sf::Vector2f(tamPx, tamPx));
		puntito.setPosition(estrellas[i].x * ancho, estrellas[i].y * alto);
		puntito.setFillColor(sf::Color(170, 205, 255,
									  static_cast<sf::Uint8>(30 + 85 * brillo)));
		ctx->ventana->draw(puntito);
	}

	// --- Nubes (capa mas cercana; al borrar lineas se aceleran) ---
	const int CANTIDAD_NUBES = 7;
	struct Nube { float x, y, vel, escala; };
	static Nube nubes[CANTIDAD_NUBES];
	static bool nubesListas = false;
	if (!nubesListas) {
		for (int i = 0; i < CANTIDAD_NUBES; i++) {
			nubes[i].x = azar01() * 1.3f - 0.15f;
			nubes[i].y = 0.08f + azar01() * 0.65f;
			nubes[i].vel = 0.02f + azar01() * 0.03f; // mas rapido que las estrellas
			nubes[i].escala = 0.5f + azar01() * 0.9f;
		}
		nubesListas = true;
	}
	float aceleracionNubes = 1.f + impulso * 4.f;
	float radioNube = std::max(8.f, ancho * 0.035f);
	for (int i = 0; i < CANTIDAD_NUBES; i++) {
		nubes[i].x -= nubes[i].vel * aceleracionNubes * dt;
		if (nubes[i].x < -0.35f) {
			nubes[i].x = 1.35f;
			nubes[i].y = 0.08f + azar01() * 0.65f;
			nubes[i].vel = 0.02f + azar01() * 0.03f;
		}
		sf::Color colorNube = sf::Color(150, 165, 210, 22);
		sf::CircleShape gota;
		const float radio = radioNube * nubes[i].escala;
		float cx = nubes[i].x * ancho;
		float cy = nubes[i].y * alto;
		gota.setRadius(radio);
		gota.setOrigin(radio, radio);
		gota.setFillColor(colorNube);
		// Tres circulos solapados para dar forma de nube suave.
		gota.setPosition(cx - radio * 0.6f, cy + radio * 0.1f);
		ctx->ventana->draw(gota);
		gota.setPosition(cx, cy - radio * 0.25f);
		ctx->ventana->draw(gota);
		gota.setPosition(cx + radio * 0.6f, cy + radio * 0.15f);
		ctx->ventana->draw(gota);
	}

	// --- Silueta de ciudad nocturna con ventanas iluminadas ---
	const int MAX_EDIFICIOS = 42;
	struct Edificio { float x0, x1, hFrac; };
	static Edificio edificios[MAX_EDIFICIOS];
	static int cantidadEdificios = 0;
	static bool ciudadLista = false;
	if (!ciudadLista) {
		float cursor = 0.f;
		cantidadEdificios = 0;
		while (cursor < 1.f && cantidadEdificios < MAX_EDIFICIOS) {
			Edificio& e = edificios[cantidadEdificios];
			float anchoEdif = 0.05f + azar01() * 0.10f;
			if (cursor + anchoEdif > 1.f) anchoEdif = 1.f - cursor;
			e.x0 = cursor;
			e.x1 = cursor + anchoEdif;
			// Unos pocos edificios altos tipo torre.
			bool esTorre = (azar01() < 0.22f);
			e.hFrac = esTorre ? (0.26f + azar01() * 0.16f)
							  : (0.10f + azar01() * 0.14f);
			cursor += anchoEdif + 0.006f;
			cantidadEdificios++;
		}
		ciudadLista = true;
	}
	// Se regenera la ciudad si cambia mucho el tamano de la ventana (los datos
	// estan en fracciones, asi que solo se redibujan a escala).
	{
		sf::Color colorEdificio(4, 5, 12);
		sf::Color colorSombra(8, 10, 20);
		float winVentana = std::max(2.f, ancho * 0.0042f);
		float winAlto = winVentana * 1.4f;
		float gapX = winVentana * 2.1f;
		float gapY = winAlto * 1.9f;
		for (int b = 0; b < cantidadEdificios; b++) {
			float px0 = edificios[b].x0 * ancho;
			float px1 = edificios[b].x1 * ancho;
			float pyBase = alto;
			float pyArriba = alto * (1.f - edificios[b].hFrac);
			float bw = px1 - px0;

			// Cuerpo del edificio (silueta casi negra).
			sf::RectangleShape cuerpo(sf::Vector2f(bw, pyBase - pyArriba));
			cuerpo.setPosition(px0, pyArriba);
			cuerpo.setFillColor(colorEdificio);
			ctx->ventana->draw(cuerpo);

			// Pequeno retoque de lado iluminado (luna desde la derecha).
			sf::RectangleShape lado(sf::Vector2f(std::max(1.f, bw * 0.04f), pyBase - pyArriba));
			lado.setPosition(px1 - std::max(1.f, bw * 0.04f), pyArriba);
			lado.setFillColor(colorSombra);
			ctx->ventana->draw(lado);

			// Antena en algunas torres.
			if (pyArriba < alto * 0.40f) {
				sf::RectangleShape antena(sf::Vector2f(1.5f, alto * 0.03f));
				antena.setPosition(px0 + bw * 0.5f, pyArriba - alto * 0.03f);
				antena.setFillColor(colorEdificio);
				ctx->ventana->draw(antena);
			}

			// Ventanas iluminadas (patron deterministico).
			int columnas = static_cast<int>((bw - winVentana) / (winVentana + gapX)) + 1;
			if (columnas > 6) columnas = 6;
			float winXIni = px0 + (bw - (columnas * (winVentana + gapX) - gapX)) * 0.5f;
			float winYIni = pyArriba + gapY * 0.6f;
			int filaVentana = 0;
			for (float wy = winYIni; wy < pyBase - gapY; wy += gapY, filaVentana++) {
				for (int col = 0; col < columnas; col++) {
					float wx = winXIni + col * (winVentana + gapX);
					// Celda determinista: "enciende" ~35% de las ventanas.
					unsigned int semilla = static_cast<unsigned int>(b * 73856093u) ^
										   static_cast<unsigned int>(filaVentana * 19349663u) ^
										   static_cast<unsigned int>((col + 1) * 83492791u);
					semilla = semilla * 2654435761u + 97u;
					bool encendida = ((semilla >> 13) & 0xFFu) < 92;
					if (!encendida) continue;
					bool tonoCeleste = ((semilla >> 21) & 0xFFu) < 22;
					sf::Color colorVentana = tonoCeleste
						? sf::Color(150, 200, 255, 170)
						: sf::Color(255, 205, 130, 190);
					sf::RectangleShape ventana(sf::Vector2f(winVentana, winAlto));
					ventana.setPosition(wx, wy);
					ventana.setFillColor(colorVentana);
					ctx->ventana->draw(ventana);
				}
			}
		}
	}

	// --- Scanlines sutiles sobre todo el fondo ---
	for (float y = 0.f; y < alto; y += 5.f) {
		sf::RectangleShape scanline(sf::Vector2f(ancho, 1.f));
		scanline.setPosition(0.f, y);
		scanline.setFillColor(sf::Color(0, 0, 0, 20));
		ctx->ventana->draw(scanline);
	}

	// --- Columnas laterales tipo ecualizador ---
	const float cajita = 8.f;
	const float paso = 16.f;
	float yInicio = alto * 0.10f;
	float yFin = alto * 0.90f;
	float te = tiempoFondo;
	int indiceColor = 0;
	for (float y = yInicio; y < yFin; y += paso) {
		float rel = (y - yInicio) / (yFin - yInicio); // 0 arriba ... 1 abajo
		TipoPieza tipo = static_cast<TipoPieza>(indiceColor % CANTIDAD_TIPOS_PIEZA);
		sf::Color colorBase = colorPieza(tipo);
		indiceColor++;

		float boost = std::min(1.f, impulso) * 0.7f;
		float ondaIzq = 0.5f + 0.5f * sinf(te * 6.f - rel * 10.f);
		float ondaDer = 0.5f + 0.5f * sinf(te * 6.f + rel * 10.f + 2.0f);
		float brilloIzq = std::min(1.f, ondaIzq + boost);
		float brilloDer = std::min(1.f, ondaDer + boost);

		sf::Color cIzq = colorBase;
		cIzq.a = static_cast<sf::Uint8>(40 + 190 * brilloIzq);
		sf::RectangleShape lateralIzq(sf::Vector2f(cajita, cajita));
		lateralIzq.setPosition(10.f, y);
		lateralIzq.setFillColor(cIzq);
		ctx->ventana->draw(lateralIzq);

		sf::Color cDer = colorBase;
		cDer.a = static_cast<sf::Uint8>(40 + 190 * brilloDer);
		sf::RectangleShape lateralDer(sf::Vector2f(cajita, cajita));
		lateralDer.setPosition(ancho - 10.f - cajita, y);
		lateralDer.setFillColor(cDer);
		ctx->ventana->draw(lateralDer);
	}
}

void dibujarTablero(ContextoInterfaz* ctx, const Tablero* tablero, const Pieza* piezaActual, float origenEnX, float origenEnY, float celda, bool espejo){
	float anchoTab = ANCHO_TABLERO * celda;
	float altoTab = ALTO_TABLERO * celda;
	const float bisel = 12.f;
	const float ribete = 5.f;

	// Convierte una columna logica a la posicion visual segun el modo espejo.
	auto columnaVisual = [&](int col) -> float {
		int colFinal = espejo ? (ANCHO_TABLERO - 1 - col) : col;
		return origenEnX + colFinal * celda;
	};

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
				dibujarBloque(ctx->ventana, columnaVisual(col), origenEnY + indice * celda, celda, colorPieza(tipo));
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
							celdaFantasma.setPosition(columnaVisual(colAbs), origenEnY + filaAbs * celda);
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
						dibujarBloque(ctx->ventana, columnaVisual(colAbs), origenEnY + filaAbs * celda, celda, colorActual);
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

