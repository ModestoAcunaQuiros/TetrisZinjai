#include "Juego.h"
#include "ColaEventos.h"
#include <SFML/Audio.hpp>
#include <iostream>
#include <cstdio>
#include <string>
#include <algorithm>

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

// Efecto "pieza bomba": en vez de fijarse, despeja la zona que ocuparia la
// pieza mas un anillo de una celda alrededor.
static void detonarBomba(Tablero* t, const Pieza* p) {
    const int* forma = obtenerFormaPieza(p->tipo, p->orientacion);
    int minFila = ALTO_TABLERO, maxFila = -1;
    int minCol = ANCHO_TABLERO, maxCol = -1;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (forma[r * 4 + c]) {
                int filaAbs = p->filaOrigen + r;
                int colAbs = p->colOrigen + c;
                if (filaAbs < minFila) minFila = filaAbs;
                if (filaAbs > maxFila) maxFila = filaAbs;
                if (colAbs < minCol) minCol = colAbs;
                if (colAbs > maxCol) maxCol = colAbs;
            }
        }
    }
    // Expandir el area afectada en una celda y recortarla al tablero.
    minFila = std::max(0, minFila - 1);
    maxFila = std::min(ALTO_TABLERO - 1, maxFila + 1);
    minCol = std::max(0, minCol - 1);
    maxCol = std::min(ANCHO_TABLERO - 1, maxCol + 1);
    for (int r = minFila; r <= maxFila; r++) {
        NodoFila* fila = obtenerFila(t, r);
        if (fila != nullptr) {
            for (int c = minCol; c <= maxCol; c++) {
                fila->celdas[c] = 0;
            }
        }
    }
}

// Intenta rotar la pieza. Si choca en el sitio, la "empuja" un poco hacia
// los lados o hacia arriba (wall kick) antes de rendirse, que es lo que hace
// que rotar pegado a la pared no se sienta bloqueado.
static bool rotarPieza(const Tablero* t, Pieza* actual) {
    Pieza intento = *actual;
    intento.orientacion = (intento.orientacion + 1) % 4;
    if (!piezaColisiona(t, &intento)) {
        *actual = intento;
        return true;
    }

    static const int DESPLAZES[6][2] = {
        {-1, 0}, {1, 0}, {-2, 0}, {2, 0}, {0, -1}, {0, -2}
    };
    for (int i = 0; i < 6; i++) {
        Pieza empujada = intento;
        empujada.colOrigen += DESPLAZES[i][0];
        empujada.filaOrigen += DESPLAZES[i][1];
        if (!piezaColisiona(t, &empujada)) {
            *actual = empujada;
            return true;
        }
    }
    return false;
}

EstadoJuego jugarPartida(ContextoInterfaz* ctx) {
    Tablero tablero;
    inicializarTablero(&tablero);

    ColaPiezas colaPiezas;
    inicializarColaPiezas(&colaPiezas);
    piezasSuficientes(&colaPiezas, 6);

    PilaHold hold;
    inicializarPilaHold(&hold, 1); // pila propia de capacidad 1

    Pieza piezaActiva = desencolarPieza(&colaPiezas);
    piezasSuficientes(&colaPiezas, 5);

    bool huboHoldEstaVez = false;
    int puntaje = 0;
    bool pausado = false;
    bool holdEsBomba = false;
    bool piezaEsBomba = false;

    // Estado que depende del reloj. Se declara aca porque los lambdas de
    // captura/restauracion lo usan y deben verlo en su definicion.
    bool bombaPendiente = false;        // la proxima pieza que aparezca sera bomba
    float intervaloCaida = 0.8f;        // segundos entre caidas automaticas
    float tiempoActual = 0.f;           // tiempo de partida (no avanza en pausa)
    float tiempoCongeladoHasta = -1.f;  // controles congelados hasta este momento
    float tiempoEspejoHasta = -1.f;     // modo espejo hasta este momento

    // Replay de la partida
    if (ctx->replay != nullptr) {
        destruirReplay(ctx->replay);
        delete ctx->replay;
        ctx->replay = nullptr;
    }
    ctx->replay = new ListaReplay;

    auto liberarReplay = [&]() {
        if (ctx->replay != nullptr) {
            destruirReplay(ctx->replay);
            delete ctx->replay;
            ctx->replay = nullptr;
        }
    };

    // Guarda/restaura TODO el estado (tablero, pieza, cola, hold y bombas).
    auto capturarEstadoJuego = [&](EstadoReplay* destino) {
        capturarEstado(&tablero, &piezaActiva, puntaje, destino);
        Pieza colaSnapshot[REPLAY_MAX_COLA];
        proximasPiezas(&colaPiezas, colaSnapshot, REPLAY_MAX_COLA);
        int nCola = colaPiezas.cantidad < REPLAY_MAX_COLA ? colaPiezas.cantidad : REPLAY_MAX_COLA;
        destino->cantidadCola = nCola;
        for (int i = 0; i < nCola; i++) {
            destino->cola[i] = colaSnapshot[i];
        }
        destino->holdVacia = pilaHoldVacia(&hold);
        if (!destino->holdVacia) {
            destino->hold = topeHold(&hold);
        }
        destino->holdEsBomba = holdEsBomba;
        destino->piezaEsBomba = piezaEsBomba;
        destino->huboHoldEstaVez = huboHoldEstaVez;

        destino->bombaPendiente = bombaPendiente;
        destino->intervaloCaida = intervaloCaida;
        destino->congeladoRestante = (tiempoActual < tiempoCongeladoHasta)
                                     ? (tiempoCongeladoHasta - tiempoActual) : 0.f;
        destino->espejoRestante = (tiempoActual < tiempoEspejoHasta)
                                  ? (tiempoEspejoHasta - tiempoActual) : 0.f;
    };

    auto restaurarEstadoJuego = [&](const EstadoReplay* origen) {
        aplicarEstado(&tablero, &piezaActiva, &puntaje, origen);

        destruirColaPiezas(&colaPiezas);
        inicializarColaPiezas(&colaPiezas);
        for (int i = 0; i < origen->cantidadCola; i++) {
            encolarPieza(&colaPiezas, origen->cola[i]);
        }

        destruirPilaHold(&hold);
        inicializarPilaHold(&hold, 1);
        if (!origen->holdVacia) {
            pushHold(&hold, origen->hold);
        }
        holdEsBomba = origen->holdEsBomba;
        piezaEsBomba = origen->piezaEsBomba;
        huboHoldEstaVez = origen->huboHoldEstaVez;

        bombaPendiente = origen->bombaPendiente;
        intervaloCaida = origen->intervaloCaida;
        tiempoCongeladoHasta = (origen->congeladoRestante > 0.f)
                               ? (tiempoActual + origen->congeladoRestante) : -1.f;
        tiempoEspejoHasta = (origen->espejoRestante > 0.f)
                            ? (tiempoActual + origen->espejoRestante) : -1.f;
    };

    EstadoReplay estadoInicial;
    capturarEstadoJuego(&estadoInicial);
    inicializarReplay(ctx->replay, &estadoInicial);

    auto registrar = [&](TipoMovimiento tipo) {
        EstadoReplay instantanea;
        capturarEstadoJuego(&instantanea);
        registrarMovimiento(ctx->replay, tipo, &instantanea);
    };

    sf::Clock relojCaida;

    // Sonido al completar lineas (se carga una sola vez por programa).
    static sf::SoundBuffer bufferLinea;
    static bool bufferLineaCargado = false;
    if (!bufferLineaCargado) {
        bufferLineaCargado = bufferLinea.loadFromFile("audio/lineaCompleta.ogg");
        if (!bufferLineaCargado) {
            std::cout << "[juego] Aviso: no se pudo cargar audio/lineaCompleta.ogg.\n";
        }
    }
    sf::Sound sonidoLinea;
    if (bufferLineaCargado) {
        sonidoLinea.setBuffer(bufferLinea);
    }

    // La musica de fondo se pausa durante el parpadeo de lineas para que se
    // oiga bien el efecto de linea completa; vuelve un segundo despues.
    bool musicaPausadaPorLinea = false;
    float momentoReanudarMusica = -1.f;
    auto reanudarMusicaFondo = [&]() {
        if (musicaPausadaPorLinea && ctx->musicaFondo != nullptr) {
            ctx->musicaFondo->play();
            musicaPausadaPorLinea = false;
            momentoReanudarMusica = -1.f;
        }
    };

    // Estado de la animacion de filas completas (parpadeo antes de borrarlas).
    const float DURACION_FLASHEO = 0.32f;
    bool flasheandoFilas = false;
    int filasFlasheo[ALTO_TABLERO];
    int cantidadFilasFlasheo = 0;
    sf::Clock relojFlasheo;

    // --- Animacion de la explosion de la bomba ---
    const float DURACION_EXPLOSION = 0.5f;
    bool explotandoBomba = false;
    sf::Clock relojExplosion;

    // --- Evento "pantalla invertida" (modo espejo durante 10 segundos) ---
    // Nota: al deshacer se restaura el tiempo RESTANTE de este y del
    // congelamiento, pero no la cola de eventos: los disparos ya consumidos
    // no se repiten (la agenda avanza con el reloj real de la partida).

    // Impulso visual para el ecualizador del fondo (se activa con un Tetris).
    float impulsoFondo = 0.f;

    // --- Eventos de partida programados por tiempo (ColaEventos) ---
    ColaEventos colaEventos;
    inicializarColaEventos(&colaEventos);

    sf::Clock relojPartida;

    std::string mensajeEvento;      // ultimo evento disparado (para avisar)
    float tiempoMensaje = 0.f;

    // Agenda inicial: cada 15 segundos ocurre un evento, rotando tipos.
    const float inicioEventos = 15.f;
    const float periodoEventos = 15.f;
    for (int i = 0; i < 36; i++) {
        Evento eventoNuevo;
        eventoNuevo.momentoEvento = inicioEventos + i * periodoEventos;
        switch (i % 4) {
            case 0:  eventoNuevo.evento = aumentarVelocidad; break;
            case 1:  eventoNuevo.evento = piezaBomba;        break;
            case 2:  eventoNuevo.evento = congelarControl;   break;
            default: eventoNuevo.evento = pantallaInvertida; break;
        }
        programarEvento(&colaEventos, eventoNuevo);
    }

    // Pone la proxima pieza en aparecer como bomba si hay una pendiente.
    auto activarBombaSiCorresponde = [&]() {
        if (bombaPendiente && !piezaEsBomba) {
            piezaEsBomba = true;
            bombaPendiente = false;
        }
    };

    // Desencola la siguiente pieza y la prepara (activa bomba si hay).
    // Devuelve true si la pieza nueva no entra en el tablero (game over).
    auto tomarSiguientePieza = [&]() -> bool {
        piezaActiva = desencolarPieza(&colaPiezas);
        piezasSuficientes(&colaPiezas, 5);
        activarBombaSiCorresponde();
        return piezaColisiona(&tablero, &piezaActiva);
    };

    // Dimensiones del diseno logico (igual que en configurarVistaJuego).
    const float DISENO_ANCHO = 720.f;
    const float DISENO_ALTO = 540.f;
    const float ladoCelda = 20.f;
    const float anchoTablero = ANCHO_TABLERO * ladoCelda;
    const float yTablero = 70.f;

    // En game over se conserva el replay para poder reproducirlo.
    auto terminar = [&](bool conservarReplay) {
        destruirPilaHold(&hold);
        destruirColaDeEventos(&colaEventos);
        if (!conservarReplay) liberarReplay();
    };

    sf::Clock relojEntrada;
    const float DURACION_CUENTA = 3.f;
    sf::Clock relojCuenta;

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

        bool enCuenta = relojCuenta.getElapsedTime().asSeconds() < DURACION_CUENTA;

        sf::Event evento;
        while (ctx->ventana->pollEvent(evento)) {
            if (evento.type == sf::Event::Closed) {
                ctx->ventana->close();
                terminar(false);
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
                            reanudarMusicaFondo();
                            terminar(false);
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

                // En pausa, cuenta regresiva, parpadeo, explosion o congelados
                // no se mueve.
                if (pausado || enCuenta || flasheandoFilas || explotandoBomba ||
                    tiempoActual < tiempoCongeladoHasta) {
                    continue;
                }

                Pieza intento = piezaActiva;

                if (evento.key.code == sf::Keyboard::Left) {
                    intento.colOrigen--;
                    if (!piezaColisiona(&tablero, &intento)) { piezaActiva = intento; registrar(MOV_IZQUIERDA); }

                } else if (evento.key.code == sf::Keyboard::Right) {
                    intento.colOrigen++;
                    if (!piezaColisiona(&tablero, &intento)) { piezaActiva = intento; registrar(MOV_DERECHA); }

                } else if (evento.key.code == sf::Keyboard::Up) {
                    if (rotarPieza(&tablero, &piezaActiva)) registrar(MOV_ROTAR);

                } else if (evento.key.code == sf::Keyboard::Down) {
                    intento.filaOrigen++;
                    if (!piezaColisiona(&tablero, &intento)) {
                        piezaActiva = intento;
                        registrar(MOV_BAJAR);
                        relojCaida.restart();
                    }

                } else if (evento.key.code == sf::Keyboard::C) {
                    if (!huboHoldEstaVez) {
                        if (pilaHoldVacia(&hold)) {
                            pushHold(&hold, crearPieza(piezaActiva.tipo));
                            holdEsBomba = piezaEsBomba; // la bomba viaja al hold
                            piezaActiva = desencolarPieza(&colaPiezas);
                            piezasSuficientes(&colaPiezas, 5);
                            piezaEsBomba = false;
                            activarBombaSiCorresponde();
                        } else {
                            Pieza intercambio = popHold(&hold);
                            bool bombaIntercambio = holdEsBomba;
                            pushHold(&hold, crearPieza(piezaActiva.tipo));
                            holdEsBomba = piezaEsBomba;
                            piezaActiva = intercambio;
                            piezaEsBomba = bombaIntercambio; // la bomba vuelve con la pieza
                        }
                        huboHoldEstaVez = true;
                        registrar(MOV_HOLD);

                        // Si la pieza nueva no cabe, termina la partida.
                        if (piezaColisiona(&tablero, &piezaActiva)) {
                            ctx->puntajeUltimaPartida = puntaje;
                            terminar(true);
                            return ESTADO_GAMEOVER;
                        }
                    }
                } else if (evento.key.code == sf::Keyboard::Z) {
                    EstadoReplay destino;
                    if (deshacerMovimiento(ctx->replay, &destino)) {
                        restaurarEstadoJuego(&destino);
                        relojCaida.restart();
                    }
                } else if (evento.key.code == sf::Keyboard::Y) {
                    EstadoReplay destino;
                    if (rehacerMovimiento(ctx->replay, &destino)) {
                        restaurarEstadoJuego(&destino);
                        relojCaida.restart();
                    }
                }
            }
        }

        // Tiempo de partida (no avanza en pausa) y disparo de eventos.
        float dtPartida = relojPartida.restart().asSeconds();
        if (!pausado && !enCuenta) {
            tiempoActual += dtPartida;

            if (tiempoMensaje > 0.f) {
                tiempoMensaje -= dtPartida;
                if (tiempoMensaje < 0.f) tiempoMensaje = 0.f;
            }

            while (!eventosVacio(&colaEventos) && eventolisto(&colaEventos, tiempoActual)) {
                Evento ev = extraeEvento(&colaEventos);
                switch (ev.evento) {
                    case aumentarVelocidad:
                        intervaloCaida = std::max(0.35f, intervaloCaida - 0.05f);
                        mensajeEvento = "VELOCIDAD AUMENTADA";
                        break;
                    case piezaBomba:
                        bombaPendiente = true;
                        mensajeEvento = "PIEZA BOMBA EN CAMINO";
                        break;
                    case congelarControl:
                        tiempoCongeladoHasta = tiempoActual + 4.f;
                        mensajeEvento = "CONTROLES CONGELADOS";
                        break;
                    case pantallaInvertida:
                        tiempoEspejoHasta = tiempoActual + 10.f;
                        mensajeEvento = "PANTALLA INVERTIDA";
                        break;
                }
                tiempoMensaje = 2.2f;
            }
        }

        // Caida automatica por tiempo (solo si no hay pausa, parpadeo ni
        // explosion en curso).
        if (!pausado && !enCuenta && !flasheandoFilas && !explotandoBomba &&
            relojCaida.getElapsedTime().asSeconds() >= intervaloCaida) {
            relojCaida.restart();

            Pieza intento = piezaActiva;
            intento.filaOrigen++;

            if (!piezaColisiona(&tablero, &intento)) {
                piezaActiva = intento;
                registrar(MOV_BAJAR);
            } else {
                // No puede bajar mas.
                if (piezaEsBomba) {
                    // Arranca la animacion de explosion; al terminar se limpia
                    // el area y recien ahi aparece la siguiente pieza.
                    explotandoBomba = true;
                    piezaEsBomba = false;
                    relojExplosion.restart();
                } else {
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

                        // Efecto sonoro en el momento en que arranca la animacion
                        // y pausa breve de la musica de fondo.
                        if (bufferLineaCargado) {
                            sonidoLinea.stop();
                            sonidoLinea.play();
                        }
                        if (!musicaPausadaPorLinea && ctx->musicaFondo != nullptr &&
                            ctx->musicaFondo->getStatus() == sf::SoundSource::Playing) {
                            ctx->musicaFondo->pause();
                            musicaPausadaPorLinea = true;
                        }
                        // La musica vuelve 1 segundo despues del arranque del
                        // parpadeo (asi no pisa el efecto de linea completa).
                        momentoReanudarMusica = tiempoActual + 2.1f;
                    } else {
                        huboHoldEstaVez = false;
                        bool finDePartida = tomarSiguientePieza();
                        registrar(MOV_COLOCAR);
                        if (finDePartida) {
                            ctx->puntajeUltimaPartida = puntaje;
                            terminar(true);
                            return ESTADO_GAMEOVER; // la pieza nueva no tiene espacio
                        }
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

            // Al borrar filas el fondo "reacciona": maximo impulso con un
            // Tetris (4 filas) y un empujon menor con filas sueltas.
            float nuevoImpulso = (cantidadFilasFlasheo >= 4) ? 1.f : 0.45f;
            if (nuevoImpulso > impulsoFondo) impulsoFondo = nuevoImpulso;

            flasheandoFilas = false;
            huboHoldEstaVez = false;
            bool finDePartida = tomarSiguientePieza();
            registrar(MOV_COLOCAR);
            if (finDePartida) {
                ctx->puntajeUltimaPartida = puntaje;
                reanudarMusicaFondo();
                terminar(true);
                return ESTADO_GAMEOVER;
            }
            relojCaida.restart();
        }

        // La musica de fondo vuelve un instante despues del parpadeo, para
        // no tapar el efecto de linea completa.
        if (!pausado && musicaPausadaPorLinea && !flasheandoFilas &&
            momentoReanudarMusica >= 0.f &&
            tiempoActual >= momentoReanudarMusica) {
            reanudarMusicaFondo();
        }

        // Fin de la animacion de explosion: se limpia la zona de la bomba.
        if (!pausado && explotandoBomba &&
            relojExplosion.getElapsedTime().asSeconds() >= DURACION_EXPLOSION) {
            detonarBomba(&tablero, &piezaActiva);
            puntaje += 100;
            explotandoBomba = false;
            huboHoldEstaVez = false;
            bool finDePartida = tomarSiguientePieza();
            registrar(MOV_COLOCAR);
            if (finDePartida) {
                ctx->puntajeUltimaPartida = puntaje;
                reanudarMusicaFondo();
                terminar(true);
                return ESTADO_GAMEOVER;
            }
            relojCaida.restart();
        }

        // Decaimiento suave del impulso del ecualizador.
        if (impulsoFondo > 0.f) {
            impulsoFondo = std::max(0.f, impulsoFondo - dtPartida * 1.4f);
        }

        // --- Dibujo ---
        ctx->ventana->clear(sf::Color(4, 6, 12));

        // Fondo (en pixeles reales) y luego vista escalable para la escena.
        dibujarFondoEscenario(ctx, impulsoFondo);
        ctx->ventana->setView(sf::View(sf::FloatRect(0.f, 0.f, anchoLogico, altoLogico)));

        const int cantidadSiguientes = 5;
        Pieza proximasPreview[cantidadSiguientes];
        proximasPiezas(&colaPiezas, proximasPreview, cantidadSiguientes);

        // Durante el parpadeo o la explosion la pieza activa no se dibuja
        // (la explosion tiene su propia animacion).
        bool pantallaEspejo = (tiempoActual < tiempoEspejoHasta);
        const Pieza* piezaParaDibujar = (flasheandoFilas || explotandoBomba) ? nullptr : &piezaActiva;

        // Interpola la caida de la pieza activa (0..1 celda).
        float desplazamientoCaida = 0.f;
        if (piezaParaDibujar != nullptr && !pausado) {
            Pieza abajo = piezaActiva;
            abajo.filaOrigen++;
            if (!piezaColisiona(&tablero, &abajo)) {
                float progreso = relojCaida.getElapsedTime().asSeconds() / intervaloCaida;
                if (progreso < 0.f) progreso = 0.f;
                if (progreso > 1.f) progreso = 1.f;
                desplazamientoCaida = progreso * ladoCelda;
            }
        }

        dibujarTablero(ctx, &tablero, piezaParaDibujar, xTablero, yTablero, ladoCelda, pantallaEspejo, desplazamientoCaida);

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

        // Animacion de la explosion de la bomba.
        if (explotandoBomba) {
            float progreso = std::min(1.f, relojExplosion.getElapsedTime().asSeconds() / DURACION_EXPLOSION);

            const int* formaBomba = obtenerFormaPieza(piezaActiva.tipo, piezaActiva.orientacion);
            int filaMin = ALTO_TABLERO, filaMax = -1;
            int colMin = ANCHO_TABLERO, colMax = -1;
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    if (formaBomba[r * 4 + c]) {
                        int fa = piezaActiva.filaOrigen + r;
                        int ca = piezaActiva.colOrigen + c;
                        if (fa < filaMin) filaMin = fa;
                        if (fa > filaMax) filaMax = fa;
                        if (ca < colMin) colMin = ca;
                        if (ca > colMax) colMax = ca;
                    }
                }
            }
            float centroCol = (colMin + colMax) * 0.5f;
            float centroFila = (filaMin + filaMax) * 0.5f;
            float colVisual = pantallaEspejo ? (ANCHO_TABLERO - 1 - centroCol) : centroCol;
            float centroPx = xTablero + (colVisual + 0.5f) * ladoCelda;
            float centroPy = yTablero + (centroFila + 0.5f) * ladoCelda;
            float radioMax = ((filaMax - filaMin + colMax - colMin) * 0.5f + 2.2f) * ladoCelda;

            if (progreso < 0.7f) {
                // Destello que crece desde el centro de la bomba.
                float radio = radioMax * (progreso / 0.7f);
                sf::CircleShape destello(radio);
                destello.setOrigin(radio, radio);
                destello.setPosition(centroPx, centroPy);
                destello.setFillColor(sf::Color(255, 240, 180, 150));
                ctx->ventana->draw(destello);

                sf::CircleShape anillo(radio);
                anillo.setOrigin(radio, radio);
                anillo.setPosition(centroPx, centroPy);
                anillo.setOutlineThickness(3.f);
                anillo.setOutlineColor(sf::Color(255, 110, 60, 200));
                ctx->ventana->draw(anillo);
            } else {
                // Parpadeo blanco final antes de limpiar la zona.
                bool encendido = (static_cast<int>(progreso * 22.f) % 2 == 0);
                if (encendido) {
                    sf::CircleShape flash(radioMax);
                    flash.setOrigin(radioMax, radioMax);
                    flash.setPosition(centroPx, centroPy);
                    flash.setFillColor(sf::Color(255, 255, 255, 120));
                    ctx->ventana->draw(flash);
                }
            }
        }

        float xColumnaIzquierda = xTablero - 145.f;
        float xColumnaDerecha = xTablero + anchoTablero + 30.f;

        dibujarPanelHold(ctx, &hold, xColumnaIzquierda, yTablero, holdEsBomba);
        dibujarPanelPuntaje(ctx, puntaje, xColumnaIzquierda, yTablero + 86.f);
        dibujarPanelSiguientes(ctx, proximasPreview, cantidadSiguientes, xColumnaDerecha, yTablero,
                               bombaPendiente && !piezaEsBomba);

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

        // Aviso de estado de eventos: "toast" debajo del tablero.
        if (!pausado && ctx->fuenteCargada) {
            std::string textoBanner;
            sf::Color colorBanner(220, 228, 245);
            char restante[8];

            if (tiempoActual < tiempoEspejoHasta) {
                textoBanner = "PANTALLA INVERTIDA (";
                snprintf(restante, sizeof(restante), "%.1f",
                         tiempoEspejoHasta - tiempoActual);
                textoBanner += restante;
                textoBanner += "s)";
                colorBanner = sf::Color(255, 130, 210);
            } else if (tiempoActual < tiempoCongeladoHasta) {
                textoBanner = "CONTROLES CONGELADOS (";
                snprintf(restante, sizeof(restante), "%.1f",
                         tiempoCongeladoHasta - tiempoActual);
                textoBanner += restante;
                textoBanner += "s)";
                colorBanner = sf::Color(110, 225, 255);
            } else if (bombaPendiente || piezaEsBomba) {
                textoBanner = "BOMBA ACTIVA";
                colorBanner = sf::Color(255, 90, 90);
            } else if (tiempoMensaje > 0.f) {
                textoBanner = mensajeEvento;
            }

            if (!textoBanner.empty()) {
                sf::Text aviso(textoBanner, ctx->fuente, 13);
                sf::FloatRect limitesAviso = aviso.getLocalBounds();

                float anchoToast = limitesAviso.width + 26.f;
                float altoToast = 22.f;
                float cxToast = xTablero + anchoTablero * 0.5f;
                float yToast = yTablero + ALTO_TABLERO * ladoCelda + 16.f;
                float xToast = cxToast - anchoToast * 0.5f;

                dibujarPanelChamfer(ctx, xToast, yToast, anchoToast, altoToast,
                                    sf::Color(12, 17, 30, 235), colorBanner);

                sf::RectangleShape acento(sf::Vector2f(4.f, altoToast - 6.f));
                acento.setPosition(xToast + 3.f, yToast + 3.f);
                acento.setFillColor(colorBanner);
                ctx->ventana->draw(acento);

                sf::Text etiqueta(textoBanner, ctx->fuente, 13);
                etiqueta.setColor(colorBanner);
                sf::FloatRect limites = etiqueta.getLocalBounds();
                etiqueta.setPosition(xToast + 13.f,
                                     yToast + (altoToast - limites.height) * 0.5f);
                ctx->ventana->draw(etiqueta);
            }
        }

        // Menu de pausa.
        if (pausado) {
            float cx = xTablero + anchoTablero * 0.5f;
            float cy = yTablero + 200.f;

            sf::RectangleShape velo(sf::Vector2f(anchoLogico, altoLogico));
            velo.setPosition(0.f, 0.f);
            velo.setFillColor(sf::Color(0, 0, 0, 150));
            ctx->ventana->draw(velo);

            dibujarPanelChamfer(ctx, cx - 160.f, cy - 105.f, 320.f, 210.f,
                                sf::Color(10, 15, 28, 235), sf::Color(80, 110, 160));

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

        // Cuenta regresiva antes de empezar a caer.
        if (enCuenta && ctx->fuenteCargada) {
            float restante = DURACION_CUENTA - relojCuenta.getElapsedTime().asSeconds();
            int numero = static_cast<int>(restante) + 1;
            if (numero < 1) numero = 1;
            if (numero > 3) numero = 3;
            float frac = restante - static_cast<float>(static_cast<int>(restante));

            char texto[8];
            snprintf(texto, sizeof(texto), "%d", numero);
            sf::Text t(texto, ctx->fuente, 120);
            t.setColor(sf::Color(255, 213, 0, static_cast<sf::Uint8>(120 + 135 * frac)));
            sf::FloatRect lb = t.getLocalBounds();
            t.setOrigin(lb.left + lb.width * 0.5f, lb.top + lb.height * 0.5f);
            t.setScale(1.6f - 0.6f * frac, 1.6f - 0.6f * frac);
            t.setPosition(xTablero + anchoTablero * 0.5f,
                          yTablero + ALTO_TABLERO * ladoCelda * 0.5f);
            ctx->ventana->draw(t);

            sf::Text listo("Preparate", ctx->fuente, 20);
            listo.setColor(sf::Color(200, 215, 240));
            sf::FloatRect ll = listo.getLocalBounds();
            listo.setPosition(xTablero + anchoTablero * 0.5f - ll.width * 0.5f,
                              yTablero + ALTO_TABLERO * ladoCelda + 16.f);
            ctx->ventana->draw(listo);
        }

        dibujarFundidoEntrada(ctx, relojEntrada, 0.3f);
        ctx->ventana->display();
    }

    terminar(false);
    return ESTADO_SALIR;
}
