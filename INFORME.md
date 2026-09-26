# Informe de análisis — Proyecto I: Tetris

**Curso:** EIF207 Estructuras de Datos
**Proyecto:** Tetris en C++ con SFML

Este informe responde las cuatro preguntas de análisis obligatorias, apoyándose
en el código real del proyecto y en mediciones ejecutadas sobre él.

---

## 1. Comparación de tiempos: Inserción O(n²) vs Quicksort O(n log n)

### Aclaración previa sobre la tabla real

La tabla de puntajes guarda como máximo `MAX_PUNTAJES = 10` registros
(`Puntajes.h:6`). Para 10 elementos **ambos algoritmos son instantáneos** y la
diferencia es inapreciable: el orden del top-10 nunca es un problema de
rendimiento. Por eso la comparación se hace con **tamaños crecientes de datos
de prueba**, que es lo que permite observar la tendencia de cada complejidad.
La pantalla de puntajes hace exactamente esto con `medirTiempos()` (`Ordenamiento.cpp`).

### Metodología

- Algoritmos: `ordenarInsercion` (O(n²)) y `quickSortRecursivo` (O(n log n)
  promedio), los mismos que usa el proyecto.
- Datos: `n` registros con puntajes aleatorios en `[0, 1000000)`.
- El reloj del sistema tiene poca resolución (~1 ms), así que cada medición
  repite el ordenamiento varias veces hasta acumular ~40 ms y promedia.
- Para aislar el costo del **ordenamiento** (y no el de copiar el arreglo), se
  mide un lote que solo copia y otro que copia y ordena; el tiempo reportado es
  la diferencia entre ambos, dividida entre las repeticiones.
- Se tomó el mínimo de 3 corridas por tamaño para reducir el ruido del entorno.
- Compilado con `g++ -O2` para aislar el comportamiento algorítmico. Con la
  compilación **Debug (-O0)** que usa el proyecto (`Tetris.zpr`,
  `optimization_level=0`) las constantes son ~2 veces mayores y la medición es
  más ruidosa, pero la forma de las curvas es idéntica.

### Resultados

| n | Inserción (ms) | Quicksort (ms) | Razón ins/quick | ins / n² (×10⁻⁶) | quick / (n·log₂n) (×10⁻⁶) |
|------:|---------------:|---------------:|----------------:|------------------:|--------------------------:|
| 100 | 0.0085 | 0.0045 | 1.9 | 0.85 | 6.80 |
| 250 | 0.0313 | 0.0098 | 3.2 | 0.50 | 4.90 |
| 500 | 0.1231 | 0.0293 | 4.2 | 0.49 | 6.54 |
| 1 000 | 0.6250 | 0.0703 | 8.9 | 0.63 | 7.06 |
| 2 000 | 2.1250 | 0.1875 | 11.3 | 0.53 | 8.55 |
| 4 000 | 9.6250 | 0.4141 | 23.2 | 0.60 | 8.65 |
| 8 000 | 32.0000 | 0.9219 | 34.7 | 0.50 | 8.89 |
| 16 000 | 135.0000 | 2.0938 | 64.5 | 0.53 | 9.37 |
| 32 000 | 583.0000 | 4.5625 | 127.8 | 0.57 | 9.53 |
| 64 000 | 2 328.0000 | 9.6250 | 241.9 | 0.57 | 9.42 |

### Análisis

**¿Coincide con lo que predice la notación asintótica? Sí, claramente.**

1. **Columna `ins / n²`:** se mantiene esencialmente **constante** (≈ 0.5–0.6×10⁻⁶)
   a lo largo de todo el rango. Eso es precisamente la firma de `T(n) = c·n²`:
   al dividir entre n² queda la constante `c`. Si este algoritmo fuera
   `O(n log n)`, esta columna habría caído como `(n log n)/n² = log n / n`.

2. **Columna `quick / (n·log₂n)`:** también se mantiene **acotada** (≈ 7–9.5×10⁻⁶
   para n ≥ 1000), sin crecer con n. Es la firma de `T(n) = c·n·log n`.

3. **Crecimiento de la razón:** al duplicar n, la razón se multiplica por
   aproximadamente 1.9–2.0 (por ejemplo, 64.5 → 127.8 → 241.9). La teoría
   predice que, con constantes parecidas,
   `razón(n) = ins(n)/quick(n) ≈ K·n/log₂n`; al duplicar n esa razón crece por
   `2·log₂n / log₂(2n)`, que para n grande tiende a **2**. Los datos coinciden.

4. **Prueba más fina:** `razón / (n/log₂n)` da un valor casi constante
   (≈ 0.056–0.069 para n ≥ 2000), lo que confirma que la razón entre ambos
   tiempos crece exactamente como `n / log₂ n`, tal como se deduce de comparar
   `n²` contra `n log n`.

**¿A partir de qué tamaño se nota la diferencia?**

Hay que distinguir dos sentidos de "notarse":

- **Como razón**: ya desde n ≈ 500–1 000, donde la inserción tarda 9–16 veces
  más que el quicksort.
- **Como tiempo perceptible para una persona**: la inserción supera los ~10 ms
  recién alrededor de **n ≈ 4 000**, y se vuelve claramente molesta desde
  **n ≈ 16 000** (135 ms) y dramática en **n = 64 000** (2.3 segundos contra
  9.6 ms del quicksort).

En resumen: para el tamaño real de la tabla (≤ 10) la diferencia **no existe**;
a partir de unos **miles de registros** la ventaja del `O(n log n)` empieza a
ser evidente, y desde decenas de miles es abismal. Esto es exactamente lo que
predice la notación asintótica.

**Nota sobre el caso peor:** el `O(n log n)` del quicksort es su
**comportamiento promedio**. Con el pivote en el elemento del medio y datos ya
ordenados (o casi), este quicksort sin protección degrada a `O(n²)`. La
comparación anterior usa datos aleatorios, que es su caso promedio.

---

## 2. El tablero como lista enlazada de filas

### Cómo está modelado

El tablero es una **lista enlazada de 20 nodos**, uno por fila (`Tablero.h:10-18`):

```cpp
struct NodoFila {
    int celdas[ANCHO_TABLERO]; // 10 celdas por fila
    NodoFila* siguiente;
};
struct Tablero {
    NodoFila* primeraFila; // fila 0 = arriba
    int cantidadFilas;     // siempre ALTO_TABLERO = 20
};
```

Cada nodo contiene un arreglo fijo de 10 celdas, pero la **estructura principal**
(el conjunto de filas y su orden) es la lista.

### Por qué es razonable para limpiar líneas

Cuando se completa una fila, lo que ocurre lógicamente es:

1. **Eliminar** esa fila.
2. **Insertar** una fila vacía arriba.
3. El resto de filas **baja** una posición.

Con la lista, esos tres pasos se reducen a **reacomodar punteros**:

- Eliminar una fila es *desenlazar* su nodo.
- El "descenso" de las filas superiores es **gratis**: el orden de las filas lo
  determina el encadenamiento, no la posición física de los datos. Las celdas de
  las filas que quedan **no se tocan**.
- Insertar la fila vacía arriba es enlazar un nodo al frente.

Con un arreglo 2D, en cambio, eliminar una fila obliga a **copiar** todas las
celdas de las filas de arriba para "bajarlas" (o reconstruir el arreglo entero),
lo que cuesta `O(ALTO · ANCHO)` copias de celdas.

### Costo concreto en esta implementación (`Tablero.cpp:89`, `limpiarFilasCompletas`)

La función hace **una sola pasada** sobre la lista y arma tres cosas: las filas
que se quedan, las filas completas (recicladas) y la nueva cabeza.

- **Detectar las filas completas** (`detectarFilasCompletas`): una pasada
  revisando las 10 celdas de cada fila → `O(ALTO · ANCHO)`.
- **Eliminar k filas completas + insertar k filas vacías**: en la misma pasada se
  desenlazan los k nodos y se vacían sus celdas
  (`actual->celdas[c] = 0`) para reutilizarlos como las filas nuevas del tope.
  Luego se busca la cola de la cadena reciclada (`O(k)`) y se enlaza con el resto.
  Costo total: **`O(ALTO + k · ANCHO)`**.
- **Insertar una fila vacía al frente**: son **2 operaciones**,
  `nuevo->siguiente = primeraFila; primeraFila = nuevo;` → **O(1)**.
  En el arreglo equivalente, insertar arriba exige desplazar todo → `O(ALTO·ANCHO)`.
- **Eliminar una fila completa**: desenlazarla es **O(1)** si ya se tiene el nodo
  y su predecesor (como dentro de la pasada). Localizarla por índice es `O(ALTO)`
  porque la lista no tiene acceso aleatorio.

Con los valores del proyecto (`ALTO=20`, `ANCHO=10`, como máximo k=4 líneas de un
Tetris), limpiar cuesta ≈ 20–60 operaciones sobre punteros/enteros, **sin mover
ninguna celda de las filas que sobreviven**. Un arreglo 2D costaría ~200 copias
de celdas por limpieza. Además, el costo dominante es `O(ALTO)` y **no depende
del ancho** del tablero para las filas que no se borran.

**El precio que se paga:** el acceso por índice deja de ser `O(1)`.
`obtenerFila(t, indice)` recorre la lista y cuesta `O(indice)`, y como
`celdaLibre`/`ocuparCelda` lo usan, cada comprobación de colisión
(`piezaColisiona`) es `O(ALTO)` en lugar de `O(1)`. Con 20 filas es
irrelevante en la práctica, pero es el *trade-off* honesto de esta
representación: se gana muchísimo en insertar/eliminar filas y se pierde en el
acceso aleatorio.

---

## 3. Por qué el replay necesita una lista **doblemente** enlazada

### Estructura

El replay es una lista doblemente enlazada de movimientos (`Replay.h:33-47`):

```cpp
struct NodoMovimiento {
    TipoMovimiento tipo;
    EstadoReplay   estado;     // foto completa del juego en ese paso
    NodoMovimiento* anterior;  // <-- navegacion hacia atras
    NodoMovimiento* siguiente; // <-- navegacion hacia adelante
};
struct ListaReplay {
    NodoMovimiento* primero;
    NodoMovimiento* ultimo;
    NodoMovimiento* actual;    // nullptr = estado inicial
    EstadoReplay    estadoInicial;
    int cantidad;
};
```

### Por qué no alcanza una lista simplemente enlazada

El replay debe permitir **retroceder y avanzar** libremente por la partida
(teclas `←` y `→`/`Espacio`), y además permite **deshacer/rehacer** durante el
juego (`Z`/`Y`). Eso exige recorrer la línea de tiempo en **ambos sentidos**.

- Con `anterior` y `siguiente`, ir a un paso vecino es directo.
- Una lista simplemente enlazada solo permite avanzar. Para **retroceder un
  paso** habría que recorrer desde `primero` hasta el nodo previo a `actual`,
  lo que cuesta `O(n)` en número de movimientos. Habría que mantener una
  estructura auxiliar (pila/arreglo de historial), lo que complica el diseño y
  gasta memoria extra.

Además, al **registrar un movimiento nuevo después de haber retrocedido**, hay
que **descartar la rama de rehacer** (`Replay.cpp:42`,
`registrarMovimiento` → `descartarNodosDespuesDe`). Con doble enlace, cortar esa
rama es `O(1)` (se hace `desde->siguiente = nullptr` y `ultimo = desde`) y luego
se borran los nodos sobrantes. Con enlace simple, encontrar el nodo anterior a
`actual` para poder cortar costaría `O(n)`.

### Costo de cada paso

- **Retroceder** (`deshacerMovimiento`, `Replay.cpp:63`): `O(1)`.
  Toma `actual->anterior` (o el `estadoInicial` si está al principio), copia el
  estado y mueve `actual`.
- **Avanzar** (`rehacerMovimiento`, `Replay.cpp:73`): `O(1)`.
  Toma `actual->siguiente` (o `primero` si está en el inicio), copia el estado y
  mueve `actual`.
- **Registrar** (`registrarMovimiento`, `Replay.cpp:42`): `O(1)` para agregar al
  final, más `O(m)` para borrar la rama de rehacer si existía (`m` = movimientos
  descartados). Actualizar `actual` y `ultimo` es `O(1)`.

El "paso" es `O(1)` **en la cantidad de movimientos**, pero conviene precisar
que la copia `*destino = nodo->estado` copia un `EstadoReplay` de tamaño
**fijo**: el arreglo del tablero (`20×10` enteros) más la cola de hasta
`REPLAY_MAX_COLA = 32` piezas, el hold y las banderas. Es una constante
independiente del largo de la partida, así que no cambia la complejidad.

**Resumen:** la doblemente enlazada da navegación bidireccional `O(1)` por paso
y poda de la rama de rehacer `O(1)`; una simplemente enlazada obligaría a `O(n)`
para retroceder y para podar.

---

## 4. Garantizar que la cola de eventos tenga al frente el más próximo

### Estructura y lógica

La cola de eventos es una lista simplemente enlazada ordenada de forma
**ascendente** por `momentoEvento` (`ColaEventos.h`, `ColaEventos.cpp:20`):

```cpp
void programarEvento(ColaEventos* cola, Evento evento){
    NodoEvento* nuevo = new NodoEvento;
    nuevo->dato = evento;

    // Caso 1: lista vacia o el nuevo evento es el mas temprano -> nueva cabeza.
    if(cola->frente == nullptr || evento.momentoEvento < cola->frente->dato.momentoEvento){
        nuevo->siguiente = cola->frente;
        cola->frente = nuevo;
        cola->cantidad++;
        return;
    }

    // Caso 2: avanzar hasta el ultimo evento con tiempo <= al nuevo.
    NodoEvento* actual = cola->frente;
    while(actual->siguiente != nullptr &&
          actual->siguiente->dato.momentoEvento <= evento.momentoEvento){
        actual = actual->siguiente;
    }
    nuevo->siguiente = actual->siguiente;
    actual->siguiente = nuevo;
    cola->cantidad++;
}
```

### Cómo se garantiza el invariante

El **invariante** es: *la lista siempre está ordenada de menor a mayor
`momentoEvento`*. Se mantiene porque:

1. `programarEvento` **inserta cada evento en su lugar ordenado**: si es el más
   temprano lo pone al frente (Caso 1); si no, avanza hasta el último evento cuyo
   tiempo sea menor o igual y lo inserta justo después (Caso 2).
2. Nada más modifica el orden: las únicas otras operaciones son
   `eventolisto` (solo lee `frente`) y `extraeEvento` (saca de `frente`).
3. Como la lista está ordenada, **`frente` es siempre el mínimo**, es decir, el
   evento más próximo a dispararse. `eventolisto(cola, t)` simplemente compara
   `cola->frente->dato.momentoEvento <= t`.
4. Sacar por el frente (`extraeEvento`) preserva el orden del resto.

Detalle fino: la comparación del Caso 2 usa `<=`, así que entre eventos con el
**mismo** tiempo el nuevo queda después de los existentes. Esto da un
comportamiento **estable / FIFO** cuando hay empates, en vez de invertir el orden.

### Complejidad de insertar

- **Mejor caso: `O(1)`** — el evento es el más temprano y va al frente.
- **Peor caso: `O(n)`** — el evento es el más tardío y hay que recorrer toda la
  lista para insertarlo al final.
- En general es `O(k)`, donde `k` es la cantidad de eventos con tiempo menor o
  igual al nuevo.

En este proyecto el costo real es intrascendente: la agenda se arma una sola vez
con 36 eventos (`Juego.cpp`, `for (int i = 0; i < 36; i++)`) con tiempos
estrictamente crecientes (15 s, 30 s, 45 s, …), y durante la partida no se
programan más.

**Observación de mejora:** como esos 36 eventos se insertan en orden creciente,
cada inserción termina recorriendo la lista hasta el final → `O(n)` por evento,
`O(n²)` para construir la agenda completa. Con n=36 son ~1 300 comparaciones
(despreciable), pero si la agenda fuera grande convendría **guardar un puntero al
final** (`fin`) para insertar en `O(1)` en este caso tan común, o usar una **cola
de prioridad / montículo** para `O(log n)` en el peor caso. Se eligió la lista
ordenada porque da acceso `O(1)` al mínimo (lo que más se usa) y el código queda
simple; el costo de inserción no se nota con la agenda real.

---

## Conclusión

Los cuatro componentes cumplen lo que exige el análisis:

- **Ordenamiento:** los datos medidos confirman `O(n²)` para inserción y
  `O(n log n)` para quicksort; la diferencia pasa de imperceptible (n ≤ 500) a
  abrumadora (n ≥ 16 000), y la razón entre ambos crece como `n / log n`, justo
  lo que predice la notación asintótica.
- **Tablero:** la lista de filas permite limpiar líneas con operaciones de
  punteros `O(ALTO + k·ANCHO)` e insertar una fila vacía en `O(1)`, sin mover las
  celdas de las filas que sobreviven; el precio es el acceso por índice `O(ALTO)`.
- **Replay:** la lista doblemente enlazada es necesaria para navegar en ambos
  sentidos y podar la rama de rehacer en `O(1)`; una simplemente enlazada
  costaría `O(n)` en ambas operaciones.
- **Cola de eventos:** se mantiene ordenada por inserción, lo que garantiza que
  `frente` sea siempre el próximo evento; insertar cuesta `O(1)` en el mejor caso
  y `O(n)` en el peor.
