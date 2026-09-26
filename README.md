# Tetris — EIF207 Estructuras de Datos (Proyecto I)

Versión simplificada de Tetris en C++ con SFML. Proyecto individual del curso
EIF207 (Universidad Nacional de Costa Rica, Sede Regional Brunca).

## Documentación
- `README.md` (este archivo): compilación, controles y estructura.
- `INFORME.md`: informe de análisis (comparación de ordenamientos, tablero como
  lista de filas, replay y cola de eventos).

## Requisitos
- **ZinjaI** (incluye MinGW GCC 6.3) en Windows.
- **SFML 2.4.2** (viene incluida en la instalación de ZinjaI, carpeta `sfml2/`).
- Las DLLs `sfml-graphics-2.dll`, `sfml-window-2.dll`, `sfml-system-2.dll`,
  `sfml-audio-2.dll` y `openal32.dll` deben estar junto al ejecutable o en el `PATH`.

## Cómo compilar
1. Abrir `Tetris.zpr` en ZinjaI.
2. Seleccionar la configuración **Debug_Win32**.
3. Compilar / Recompilar.

El proyecto compila todos los `.cpp` del directorio y enlaza contra SFML.

## Cómo ejecutar
- Desde ZinjaI con **Ejecutar**, o abriendo `Debug_Win32/Tetris.exe`.
- Debe ejecutarse con el **directorio de trabajo en la carpeta del proyecto**
  (para encontrar `data/` y `audio/`). Desde ZinjaI esto es automático.

## Librería gráfica
**SFML 2.4.2** (módulos `graphics`, `window`, `audio` y `system`).

## Estructuras de datos implementadas
- `ColaPieza` — cola propia (nodos) + generación por bolsas de 7 piezas.
- `PilaHold` — pila propia (LIFO) de capacidad 1 para la pieza en espera.
- `ColaEventos` — cola propia ordenada por momento de disparo de cada evento.
- `Tablero` — lista enlazada de 20 filas (10 celdas por fila).
- `Replay` — lista doblemente enlazada de movimientos (deshacer / rehacer / reproducir).
- `Ordenamiento` — inserción O(n²) y quicksort O(n log n) propios.
- `Puntajes` — persistencia de la tabla de mejores puntajes (top 10).
- `Pieza`, `Juego`, `Interfaz` — tetrominós, lógica del juego e interfaz gráfica.

No se usan `std::stack`, `std::queue`, `std::deque`, `std::list`,
`std::priority_queue` ni `std::sort`.

## Controles
**Jugando**
- `←` / `→`: mover
- `↑`: rotar (con *wall kick*: la pieza se empuja si choca con la pared)
- `↓`: bajar
- `C`: hold
- `Z`: deshacer
- `Y`: rehacer
- `Esc` / `P`: pausa

**Menú**
- `Enter`: jugar
- `H`: tabla de puntajes
- `Esc`: salir

**Fin de partida**
- `R`: reproducir la partida
- `T`: ver tabla de puntajes
- `Enter` / `Esc`: volver al menú

**Tabla de puntajes**
- `1`: ordenar por inserción
- `2`: ordenar por quicksort
- `3`: repetir la comparación de tiempos
- `Enter` / `Esc`: volver

**Replay**
- `←`: retroceder
- `→` / `Espacio`: avanzar
- `P`: reproducción automática
- `Esc`: volver

## Deshacer / rehacer
Cada movimiento (mover, rotar, bajar, colocar y usar el hold) guarda una foto
completa del estado: tablero, pieza activa, puntaje, **cola de piezas**, **hold**,
banderas de bomba y el estado del reloj (velocidad de caída, bomba pendiente y
tiempo restante de congelamiento o pantalla invertida).

Lo único que **no** se rebobina es la agenda de eventos por tiempo: los disparos
ya consumidos no se repiten, así que la agenda sigue avanzando con el reloj real
de la partida.

## Eventos de partida
Cada 15 segundos se dispara un evento que rota entre cuatro tipos:
aumentar velocidad, pieza bomba, controles congelados (4 s) y pantalla
invertida (10 s). La pieza bomba se marca en rojo en el panel *Siguiente* y, si
se guarda en el hold, también en el panel *Hold*.

## Puntaje
- 1 línea: 100 · 2: 300 · 3: 500 · 4: 800
- Pieza bomba: +100

## Archivos de datos
- `data/puntajes.txt` — tabla de mejores puntajes (se crea/actualiza solo).
- `data/deer-diary.ttf`, `data/Menu1.png` — fuente y fondo del menú.
- `audio/*.ogg` — música y efectos de sonido.

Si `data/Menu1.png` no está, el juego dibuja un menú simple con botones y
avisa en la consola, de modo que siempre se puede jugar.

## Autor
- Nombre: _(completar)_
- Cédula: _(completar)_
