## Instalación y configuración

Guía para compilar y ejecutar el proyecto en otra máquina (Windows + ZinjaI + SFML 2.5.1).

### Requisitos

- Windows con **ZinjaI** instalado (instalador estándar, que incluye MinGW).
- La **SFML 2.x (2.5.1)** que trae ZinjaI: dentro de la carpeta de MinGW de ZinjaI deben existir `sfml2\include` y `sfml2\lib`. ZinjaI la referencia con la variable `${MINGW_DIR}`.
- Si esa carpeta no existe, instalar SFML 2.5.1 para MinGW (misma arquitectura que el compilador) y cambiar las dos rutas de la tabla de configuración.

### Qué copiar

- Todos los `.cpp` y `.h`, y el proyecto `Tetris.zpr`.
- Las carpetas `audio` y `data` y los recursos (`.ogg`, `Menu.png`, `sfml.png`), **manteniendo la estructura de carpetas**: el programa los carga con rutas relativas.
- Las DLL: `sfml-graphics-2.dll`, `sfml-window-2.dll`, `sfml-system-2.dll`, `sfml-audio-2.dll` y `openal32.dll`.
- **No** hace falta copiar `Debug`, `Debug_Win32` ni `Tet9234.tmp` (salida de compilación y temporales).
- Conviene que la ruta de la carpeta no tenga tildes ni caracteres especiales.

### Pasos

1. Instalar ZinjaI.
2. Copiar la carpeta del proyecto a la otra máquina.
3. Abrir `Tetris.zpr` con ZinjaI.
4. En **Opciones de Compilación y Ejecución**, elegir la configuración `Debug_Win32` en el desplegable superior.
5. Verificar los valores de la tabla siguiente.
6. Compilar y ejecutar desde el menú **Ejecución**.

Se recomienda ejecutar desde ZinjaI. Si se abre el `.exe` a mano, debe hacerse desde la carpeta del proyecto (o con las DLL y los recursos junto al ejecutable).

### Configuración de Debug_Win32

| Pestaña | Campo | Valor |
|---|---|---|
| Compilación | Constantes de preprocesador | `SFML_STATIC` |
| Compilación | Directorios para cabeceras | `${MINGW_DIR}\sfml2\include` |
| Enlazado | Directorios para bibliotecas | `${MINGW_DIR}\sfml2\lib` |
| Enlazado | Bibliotecas a enlazar | `sfml-graphics sfml-window sfml-audio sfml-system sfml-network` |
| Enlazado | Programa de consola | Marcado |
| Compilación | Estándar de C++ | Predeterminado |

Estas rutas usan `${MINGW_DIR}` y no rutas absolutas, por eso el proyecto se puede mover entre máquinas sin reconfigurar. La biblioteca `sfml-network` no es indispensable si el juego no usa red.

### Problemas comunes

| Síntoma | Causa probable | Solución |
|---|---|---|
| `SFML/Graphics.hpp: No such file` | Las rutas de cabeceras no apuntan a una SFML real, o no está instalada. | Revisar `${MINGW_DIR}\sfml2\include` y que la carpeta exista. |
| `undefined reference to sf::...` | Mezcla de versiones entre `include` y `lib`. | Usar solo `${MINGW_DIR}\sfml2` en ambos campos, sin rutas viejas. |
| Falta una DLL al abrir el juego | Las DLL no están junto al ejecutable ni en la carpeta del proyecto. | Copiar las DLL de la sección "Qué copiar". |
| Abre, pero sin música ni imágenes | Recursos no encontrados por ruta relativa, o falta `openal32.dll`. | Respetar la estructura de carpetas y copiar `openal32.dll`. |
