#include "Juego.h"
#include <iostream>

// Revisa si la pieza "p" colisiona con el tablero o se sale de los bordes.
static bool piezaColisiona(const Tablero* t, const Pieza* p) {
    const int* forma = obtenerFormaPieza(p->tipo, p->orientacion);
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (forma[r * 4 + c]) {
                int filaAbs = p->filaOrigen + r;
                int colAbs = p->colOrigen + c;
                if (!celdaLibre(t, filaAbs, colAbs)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Copia la forma de la pieza al tablero como celdas ocupadas.
// Se guarda (tipo + 1) para que 0 siga significando "celda vacia".
static void fijarPieza(Tablero* t, const Pieza* p) {
    const int* forma = obtenerFormaPieza(p->tipo, p->orientacion);
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (forma[r * 4 + c]) {
                ocuparCelda(t, p->filaOrigen + r, p->colOrigen + c, static_cast<int>(p->tipo) + 1);
            }
        }
    }
}

EstadoJuego jugarPartida(ContextoInterfaz* ctx) {
    Tablero tablero;
    inicializarTablero(&tablero);

    ColaPiezas colaPiezas;
    inicializarColaPiezas(&colaPiezas);
    piezasSuficientes(&colaPiezas, 6);

    PilaHold hold;
    inicializarPilaHold(&hold);

    Pieza piezaActiva = desencolarPieza(&colaPiezas);
    piezasSuficientes(&colaPiezas, 5);

    bool huboHoldEstaVez = false;
    int puntaje = 0;
    bool pausado = false;

    sf::Clock relojCaida;
    float intervaloCaida = 0.8f; // segundos entre caidas automaticas

    // Estado de la animacion de filas completas (parpadeo antes de borrarlas).
    const float DURACION_FLASHEO = 0.32f;
    bool flasheandoFilas = false;
    int filasFlasheo[ALTO_TABLERO];
    int cantidadFilasFlasheo = 0;
    sf::Clock relojFlasheo;

    // Dimensiones del diseno logico (igual que en configurarVistaJuego).
    const float DISENO_ANCHO = 720.f;
    const float DISENO_ALTO = 540.f;
    const float ladoCelda = 20.f;
    const float anchoTablero = ANCHO_TABLERO * ladoCelda;
    const float yTablero = 70.f;

    while (ctx->ventana->isOpen()) {
        // Vista por defecto (pixeles reales) para el fondo de cada frame.
        ctx->ventana->setView(ctx->ventana->getDefaultView());

        // Escala y coordenadas logicas segun el tamano actual de la ventana.
        sf::Vector2u tamVentana = ctx->ventana->getSize();
        float escala = (static_cast<float>(tamVentana.x) / DISENO_ANCHO <
                        static_cast<float>(tamVentana.y) / DISENO_ALTO)
                       ? static_cast<float>(tamVentana.x) / DISENO_ANCHO
                       : static_cast<float>(tamVentana.y) / DISENO_ALTO;
        if (escala < 1.f) escala = 1.f;
        float anchoLogico = static_cast<float>(tamVentana.x) / escala;
        float altoLogico = static_cast<float>(tamVentana.y) / escala;
        float xTablero = (anchoLogico - anchoTablero) * 0.5f;

        sf::Event evento;
        while (ctx->ventana->pollEvent(evento)) {
            if (evento.type == sf::Event::Closed) {
                ctx->ventana->close();
                return ESTADO_SALIR;
            }

            if (evento.type == sf::Event::MouseButtonPressed &&
                evento.mouseButton.button == sf::Mouse::Left) {
                float mx = static_cast<float>(evento.mouseButton.x) / escala;
                float my = static_cast<float>(evento.mouseButton.y) / escala;

                if (pausado) {
                    // Menu de pausa: botones Continuar / Salir al menu.
                    float cx = xTablero + anchoTablero * 0.5f;
                    float cy = yTablero + 200.f;
                    if (mx >= cx - 95.f && mx <= cx + 95.f) {
                        if (my >= cy - 20.f && my <= cy + 22.f) {
                            pausado = false;
                            relojCaida.restart();
                            if (flasheandoFilas) relojFlasheo.restart();
                        } else if (my >= cy + 30.f && my <= cy + 72.f) {
                            return ESTADO_MENU;
                        }
                    }
                } else {
                    // Boton de pausa flotante (arriba del tablero).
                    float bx = xTablero + anchoTablero * 0.5f - 23.f;
                    float by = yTablero - 62.f;
                    if (mx >= bx && mx <= bx + 46.f && my >= by && my <= by + 34.f) {
                        pausado = true;
                    }
                }
            }

            if (evento.type == sf::Event::KeyPressed) {
                if (evento.key.code == sf::Keyboard::Escape ||
                    evento.key.code == sf::Keyboard::P) {
                    pausado = !pausado;
                    if (!pausado) {
                        relojCaida.restart();
                        if (flasheandoFilas) relojFlasheo.restart();
                    }
                    continue;
                }

                // En pausa o durante el parpadeo no se manejan piezas.
                if (pausado || flasheandoFilas) {
                    continue;
                }

                Pieza intento = piezaActiva;

                if (evento.key.code == sf::Keyboard::Left) {
                    intento.colOrigen--;
                    if (!piezaColisiona(&tablero, &intento)) piezaActiva = intento;

                } else if (evento.key.code == sf::Keyboard::Right) {
                    intento.colOrigen++;
                    if (!piezaColisiona(&tablero, &intento)) piezaActiva = intento;

                } else if (evento.key.code == sf::Keyboard::Up) {
                    intento.orientacion = (intento.orientacion + 1) % 4;
                    if (!piezaColisiona(&tablero, &intento)) piezaActiva = intento; // sin wall kick

                } else if (evento.key.code == sf::Keyboard::Down) {
                    intento.filaOrigen++;
                    if (!piezaColisiona(&tablero, &intento)) piezaActiva = intento;

                } else if (evento.key.code == sf::Keyboard::C) {
                    if (!huboHoldEstaVez) {
                        if (pilaHoldVacia(&hold)) {
                            pushHold(&hold, crearPieza(piezaActiva.tipo));
                            piezaActiva = desencolarPieza(&colaPiezas);
                            piezasSuficientes(&colaPiezas, 5);
                        } else {
                            Pieza intercambio = popHold(&hold);
                            pushHold(&hold, crearPieza(piezaActiva.tipo));
                            piezaActiva = intercambio;
                        }
                        huboHoldEstaVez = true;
                    }
                }
            }
        }

        // Caida automatica por tiempo (solo si no hay pausa ni parpadeo).
        if (!pausado && !flasheandoFilas &&
            relojCaida.getElapsedTime().asSeconds() >= intervaloCaida) {
            relojCaida.restart();

            Pieza intento = piezaActiva;
            intento.filaOrigen++;

            if (!piezaColisiona(&tablero, &intento)) {
                piezaActiva = intento;
            } else {
                // No puede bajar mas: se fija en el tablero.
                fijarPieza(&tablero, &piezaActiva);

                int indicesCompletos[ALTO_TABLERO];
                int cantidadCompletas = detectarFilasCompletas(&tablero, indicesCompletos);
                if (cantidadCompletas > 0) {
                    // Las filas se eliminan tras una breve animacion.
                    flasheandoFilas = true;
                    cantidadFilasFlasheo = cantidadCompletas;
                    for (int k = 0; k < cantidadCompletas; k++) {
                        filasFlasheo[k] = indicesCompletos[k];
                    }
                    relojFlasheo.restart();
                } else {
                    huboHoldEstaVez = false;
                    piezaActiva = desencolarPieza(&colaPiezas);
                    piezasSuficientes(&colaPiezas, 5);

                    if (piezaColisiona(&tablero, &piezaActiva)) {
                        return ESTADO_GAMEOVER; // la pieza nueva no tiene espacio
                    }
                }
            }
        }

        // Fin de la animacion de filas: se borran y sigue la partida.
        if (!pausado && flasheandoFilas &&
            relojFlasheo.getElapsedTime().asSeconds() >= DURACION_FLASHEO) {
            limpiarFilasCompletas(&tablero, filasFlasheo, cantidadFilasFlasheo);

            const int puntosPorFilas[5] = {0, 100, 300, 500, 800};
            int ganados = cantidadFilasFlasheo < 5 ? puntosPorFilas[cantidadFilasFlasheo] : 800;
            puntaje += ganados;

            flasheandoFilas = false;
            huboHoldEstaVez = false;
            piezaActiva = desencolarPieza(&colaPiezas);
            piezasSuficientes(&colaPiezas, 5);

            if (piezaColisiona(&tablero, &piezaActiva)) {
                return ESTADO_GAMEOVER;
            }
            relojCaida.restart();
        }

        // --- Dibujo ---
        ctx->ventana->clear(sf::Color(4, 6, 12));

        // Fondo (en pixeles reales) y luego vista escalable para la escena.
        dibujarFondoEscenario(ctx);
        ctx->ventana->setView(sf::View(sf::FloatRect(0.f, 0.f, anchoLogico, altoLogico)));

        const int cantidadSiguientes = 5;
        Pieza proximasPreview[cantidadSiguientes];
        proximasPiezas(&colaPiezas, proximasPreview, cantidadSiguientes);

        // Durante el parpadeo la pieza activa ya quedo fijada: no se dibuja.
        const Pieza* piezaParaDibujar = flasheandoFilas ? nullptr : &piezaActiva;
        dibujarTablero(ctx, &tablero, piezaParaDibujar, xTablero, yTablero, ladoCelda);

        // Parpadeo de las filas completas antes de eliminarlas.
        if (flasheandoFilas) {
            float t = relojFlasheo.getElapsedTime().asSeconds();
            bool encendido = (static_cast<int>(t / 0.05f) % 2 == 0);
            sf::Color colorParpadeo = encendido
                ? sf::Color(255, 255, 255, 175)
                : sf::Color(255, 255, 255, 28);
            for (int k = 0; k < cantidadFilasFlasheo; k++) {
                sf::RectangleShape resaltado(sf::Vector2f(anchoTablero, ladoCelda));
                resaltado.setPosition(xTablero, yTablero + filasFlasheo[k] * ladoCelda);
                resaltado.setFillColor(colorParpadeo);
                ctx->ventana->draw(resaltado);
            }
        }

        float xColumnaIzquierda = xTablero - 145.f;
        float xColumnaDerecha = xTablero + anchoTablero + 30.f;

        dibujarPanelHold(ctx, &hold, xColumnaIzquierda, yTablero);
        dibujarPanelPuntaje(ctx, puntaje, xColumnaIzquierda, yTablero + 86.f);
        dibujarPanelSiguientes(ctx, proximasPreview, cantidadSiguientes, xColumnaDerecha, yTablero);

        // Boton de pausa flotante sobre el tablero (icono "II").
        float bx = xTablero + anchoTablero * 0.5f - 23.f;
        float by = yTablero - 62.f;
        {
            sf::RectangleShape boton(sf::Vector2f(46.f, 34.f));
            boton.setPosition(bx, by);
            boton.setFillColor(sf::Color(16, 22, 38));
            boton.setOutlineThickness(2.f);
            boton.setOutlineColor(sf::Color(66, 96, 146));
            ctx->ventana->draw(boton);

            sf::RectangleShape barra1(sf::Vector2f(5.f, 14.f));
            barra1.setPosition(bx + 14.f, by + 10.f);
            barra1.setFillColor(sf::Color(200, 215, 240));
            ctx->ventana->draw(barra1);

            sf::RectangleShape barra2(sf::Vector2f(5.f, 14.f));
            barra2.setPosition(bx + 27.f, by + 10.f);
            barra2.setFillColor(sf::Color(200, 215, 240));
            ctx->ventana->draw(barra2);
        }

        // Menu de pausa.
        if (pausado) {
            float cx = xTablero + anchoTablero * 0.5f;
            float cy = yTablero + 200.f;

            sf::RectangleShape velo(sf::Vector2f(anchoLogico, altoLogico));
            velo.setPosition(0.f, 0.f);
            velo.setFillColor(sf::Color(0, 0, 0, 150));
            ctx->ventana->draw(velo);

            sf::RectangleShape panel(sf::Vector2f(320.f, 210.f));
            panel.setPosition(cx - 160.f, cy - 105.f);
            panel.setFillColor(sf::Color(10, 15, 28, 235));
            panel.setOutlineThickness(2.f);
            panel.setOutlineColor(sf::Color(80, 110, 160));
            ctx->ventana->draw(panel);

            // Posicion actual del mouse para resaltar el boton bajo el cursor.
            sf::Vector2i posicionMouse = sf::Mouse::getPosition(*ctx->ventana);
            float mfx = static_cast<float>(posicionMouse.x) / escala;
            float mfy = static_cast<float>(posicionMouse.y) / escala;

            auto dibujarBotonMenu = [&](const char* texto,
                                        float yBoton, bool resaltado) {
                sf::RectangleShape boton(sf::Vector2f(190.f, 42.f));
                boton.setPosition(cx - 95.f, yBoton);
                boton.setFillColor(resaltado ? sf::Color(42, 60, 100)
                                             : sf::Color(24, 34, 60));
                boton.setOutlineThickness(2.f);
                boton.setOutlineColor(sf::Color(90, 130, 200));
                ctx->ventana->draw(boton);

                if (ctx->fuenteCargada) {
                    sf::Text etiqueta(texto, ctx->fuente, 18);
                    etiqueta.setColor(sf::Color::White);
                    sf::FloatRect limites = etiqueta.getLocalBounds();
                    etiqueta.setPosition(cx - limites.width * 0.5f,
                                         yBoton + 12.f - limites.height * 0.5f);
                    ctx->ventana->draw(etiqueta);
                }
            };

            if (ctx->fuenteCargada) {
                sf::Text titulo("PAUSA", ctx->fuente, 42);
                titulo.setColor(sf::Color(220, 228, 245));
                sf::FloatRect limites = titulo.getLocalBounds();
                titulo.setPosition(cx - limites.width * 0.5f, cy - 96.f);
                ctx->ventana->draw(titulo);

                sf::Text ayuda("ESC / P para continuar", ctx->fuente, 14);
                ayuda.setColor(sf::Color(140, 155, 185));
                sf::FloatRect limitesAyuda = ayuda.getLocalBounds();
                ayuda.setPosition(cx - limitesAyuda.width * 0.5f, cy - 48.f);
                ctx->ventana->draw(ayuda);
            }

            bool sobreContinuar = (mfx >= cx - 95.f && mfx <= cx + 95.f &&
                                   mfy >= cy - 20.f && mfy <= cy + 22.f);
            bool sobreSalir = (mfx >= cx - 95.f && mfx <= cx + 95.f &&
                               mfy >= cy + 30.f && mfy <= cy + 72.f);

            dibujarBotonMenu("Continuar", cy - 20.f, sobreContinuar);
            dibujarBotonMenu("Salir al menu", cy + 30.f, sobreSalir);
        }

        ctx->ventana->display();
    }

    return ESTADO_SALIR;
}
