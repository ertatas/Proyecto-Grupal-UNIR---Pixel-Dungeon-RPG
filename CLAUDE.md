# CLAUDE.md — Contexto del proyecto Pixel Dungeon RPG

Este fichero sirve de memoria persistente para Claude Code.
Contiene todo el contexto técnico y de diseño del proyecto para no tener que repetirlo en cada sesión.

---

## 1. Descripción del proyecto

**Pixel Dungeon RPG** — trabajo grupal para el Grado en Desarrollo de Videojuegos de UNIR.

Demo de un dungeon RPG en vista cenital (top-down) por casillas, estilo roguelike clásico.
El objetivo del demo es mostrar navegación por habitaciones, fog of war, llaves/puertas, enemigos y HUD.

---

## 2. Stack tecnológico

| Elemento | Versión / detalle |
|---|---|
| Lenguaje | C++17 |
| Librería multimedia | SFML 2.5.1 |
| Motor base | UNIR-2D (motor académico propio de UNIR, incluido en el repo) |
| IDE | Visual Studio (Windows) |
| Plataforma destino | Windows |

---

## 3. Estructura de directorios

```
Proyecto-Grupal-UNIR---Pixel-Dungeon-RPG/
├── UNIR-2D/                    ← Motor académico (ActorBase, Imagen, Textura, etc.)
│   ├── Mapa.h / Mapa.cpp       ← Sistema de mapa por capas + fog of war
│   ├── EditorMapa.h / .cpp     ← Editor in-game (F1)
│   ├── Motor.h / .cpp          ← Bucle principal SFML
│   ├── JuegoBase.h / .cpp      ← Clase base del juego
│   ├── ActorBase.h / .cpp      ← Clase base de actores
│   ├── Imagen.h / .cpp         ← Wrapper sf::Sprite
│   ├── Textura.h / .cpp        ← Wrapper sf::Texture
│   ├── Rectangulo.h / .cpp     ← Wrapper sf::RectangleShape
│   ├── Teclado.h / .cpp        ← Input teclado (edge detection)
│   ├── Raton.h / .cpp          ← Input ratón
│   ├── Texto.h / .cpp          ← Wrapper sf::Text (fuente por nombre desde fuentes/)
│   └── Color.h                 ← Color RGBA inmutable (métodos: rojo(), verde(), azul(), alfa())
│
├── DungeonJuego/               ← Código del juego
│   ├── JuegoDungeon.h / .cpp   ← Clase principal: cámara, zoom, actores
│   ├── Jugador.h / .cpp        ← Movimiento, sprite, fog of war, mensajes al HUD
│   ├── Hud.h / .cpp            ← HUD fijo: panel stats + consola de mensajes
│   ├── DungeonJuego.cpp        ← main() — instancia JuegoDungeon y arranca el motor
│   ├── mapa.txt                ← Mapa guardado (cargado automáticamente al arrancar)
│   ├── fuentes/                ← Fuentes TrueType para el HUD
│   │   └── README.txt          ← Instrucciones: copiar DejaVuSans.ttf aquí
│   └── assets/
│       ├── fonts/              ← Carpeta de entrega para artistas (luego mover a fuentes/)
│       │   └── README.txt
│       └── textures/
│           ├── characters/
│           │   └── hero/
│           │       └── hero_idle.png       ← sprite del héroe (YA EXISTE)
│           ├── tiles/
│           │   ├── base/
│           │   │   ├── README.txt          ← especificaciones para artistas
│           │   │   ├── suelo.png           ← pendiente artistas
│           │   │   ├── pared_sur.png       ← YA EXISTE (VAR_SUR)
│           │   │   ├── pared_norte.png     ← YA EXISTE (VAR_NORTE)
│           │   │   ├── pared_este.png      ← YA EXISTE (VAR_ESTE)
│           │   │   ├── pared_oeste.png     ← YA EXISTE (VAR_OESTE)
│           │   │   ├── pared_default.png   ← YA EXISTE (VAR_DEFAULT)
│           │   │   └── puerta/
│           │   │       ├── README.txt
│           │   │       └── puerta_*.png    ← pendiente artistas (4 variantes)
│           │   └── props/
│           │       ├── pared/
│           │       │   ├── README.txt      ← 16×24 px
│           │       │   └── *.png           ← lampara, antorcha (pendiente)
│           │       └── suelo/
│           │           ├── README.txt      ← 48×48 px
│           │           └── *.png           ← charco, mancha, huesos (pendiente)
│           └── ui/             ← Iconos del HUD (pendiente artistas)
│               └── README.txt  ← llave_icon.png, corazon.png, retrato_heroe.png
│
├── GUIA_EDITOR_MAPA.md         ← Documentación para artistas y diseñadores
└── CLAUDE.md                   ← Este fichero
```

---

## 4. Arquitectura de clases

### Jerarquía de herencia principal

```
unir2d::JuegoBase
    └── JuegoDungeon          (bucle del juego, cámara, zoom)

unir2d::ActorBase
    ├── Mapa                  (mapa de tiles, 6 capas, fog of war)
    ├── EditorMapa            (editor in-game, UI de paleta)
    ├── Jugador               (movimiento, sprite, colisiones, mensajes al HUD)
    └── Hud                   (HUD fijo: panel stats + consola de mensajes)
```

### Relaciones entre clases

```
JuegoDungeon
    owns → Mapa*
    owns → Jugador*(Mapa*)
    owns → EditorMapa*(Mapa*)
    owns → Hud*(Jugador*)
```

`Jugador` y `EditorMapa` reciben un puntero a `Mapa` en su constructor.
`Hud` recibe un puntero a `Jugador*` en su constructor (para futuro getLlaves()).
`JuegoDungeon::inicia()` los agrega al motor en este orden: Mapa → EditorMapa → Jugador → Hud.
`jugador->ponHud(hud)` conecta el HUD al jugador para que pueda enviar mensajes.

---

## 5. Sistema de mapa (Mapa.h / Mapa.cpp)

### Capas lógicas

El mapa tiene **7 capas**, cada una es un `std::vector<std::vector<int>>` de tamaño `filas × columnas`:

| Capa | Variable | Tipos |
|---|---|---|
| Base (estructura) | `capaBase` | `BASE_SUELO=0`, `BASE_PARED=1`, `BASE_PUERTA_CERRADA=2`, `BASE_PUERTA_ABIERTA=3` |
| Props (decoración) | `capaProps` | `PROP_NADA=0` · **Pared (1-9):** `PROP_LAMPARA=1`, `PROP_ANTORCHA=2` · **Suelo (10-19):** `PROP_CHARCO=10`, `PROP_MANCHA=11`, `PROP_HUESOS=12` |
| Trampas | `capaTrampas` | `TRAMPA_NADA=0`, `TRAMPA_FIJA=1` |
| Enemigos | `capaEnemigos` | `ENEMIGO_NADA=0`, `ENEMIGO_NORMAL=1` |
| Llaves | `capaLlaves` | `LLAVE_NADA=0`, `LLAVE_NORMAL=1` |
| Visibilidad | `capaVisibilidad` | `VIS_OCULTO=0`, `VIS_VISIBLE=1` |
| Variantes visuales | `capaVariantes` | `VAR_DEFAULT=0`, `VAR_NORTE=1`, `VAR_SUR=2`, `VAR_ESTE=3`, `VAR_OESTE=4`, `VAR_PUERTA_H=5`, `VAR_PUERTA_V=6` |

**Helpers de props (inline en Mapa.h):**
```cpp
static bool esPropDePared(int t) { return t >= 1 && t <= 9;  }
static bool esPropDeSuelo(int t) { return t >= 10 && t <= 19; }
```

También existe `capaVisual` (struct `CoordenadasVisuales`) para metadatos internos de paredes (no afecta al renderizado).

### Constantes clave

```cpp
// Mapa.h
const float tamCasilla = 64.0f;   // tamaño de cada tile en px

// Mapa.cpp → construirMapaInicial()
crearCapas(32, 32);               // 32 filas × 32 columnas

// JuegoDungeon.cpp
static const float ANCHO_VENTANA = 1280.0f;
static const float ALTO_VENTANA  =  720.0f;

// JuegoDungeon.h
float zoomNivel = 3.0f;           // zoom inicial (rango 1.0–4.0)

// EditorMapa.cpp
static const float PANEL_X = 1070.0f;  // posición X del panel del editor
```

### API pública de Mapa

```cpp
// Consulta
int  getBase    (int fila, int col) const;
int  getProp    (int fila, int col) const;
int  getLlave   (int fila, int col) const;
int  getVariante(int fila, int col) const;
int  getSpawnFila() / getSpawnCol() const;
bool esTransitable(int fila, int col) const;   // false en pared, puerta cerrada, enemigo
Vector centroCasilla(int fila, int col) const;
float tamanoCasilla() const;

// Modificación (usada por el editor y el jugador)
void setBase    (int fila, int col, int tipo);
void setProp    (int fila, int col, int tipo);
void setTrampa  (int fila, int col, int tipo);
void setEnemigo (int fila, int col, int tipo);
void setLlave   (int fila, int col, int tipo);
void setSpawn   (int fila, int col);
void setVariante(int fila, int col, int variante);  // actualiza drawable base

// Autodetección de variante: analiza vecinos → devuelve VarianteVisual
VarianteVisual detectarVariante(int fila, int col) const;

// Fog of war
void revelarZonaDesde(int fila, int col);   // BFS desde posición del jugador
void revelarTodo();                          // revela todo el mapa (editor)

// Persistencia
void guardarMapa(const std::string& ruta = "") const;  // escribe DUNGEON_MAP 2
void cargarMapa (const std::string& ruta = "");         // retrocompat. DUNGEON_MAP 1
```

### Persistencia — formato mapa.txt (versión 2)

```
DUNGEON_MAP 2
<filas> <columnas>
BASE
[filas × columnas valores enteros separados por espacios]
PROPS
[ídem]
TRAMPAS
[ídem]
ENEMIGOS
[ídem]
LLAVES
[ídem]
VARIANTES
[ídem]
SPAWN
<fila_spawn> <col_spawn>
END
```

**Retrocompatibilidad:** si se lee `DUNGEON_MAP 1` (sin sección VARIANTES), `capaVariantes` se rellena automáticamente con `detectarVariante()` en todos los tiles.

El fichero `mapa.txt` vive en `DungeonJuego/`. Si existe al arrancar, se carga automáticamente; si no, se usa el mapa por defecto de `construirMapaInicial()`.

---

## 6. Sistema de drawables del mapa

Por cada tile se crean **6 Rectangulo*** + **1 Imagen*** en `crearDibujos()`:

| Índice | Tipo | `ponIndiceZ` | Tamaño | Para qué |
|---|---|---|---|---|
| [0] Base | `Rectangulo` | 0 | 64×64 px | Suelo / puerta / pared sin PNG — color según `capaVariantes`; transparente si [6] está activo |
| [1] Trampa | `Rectangulo` | 1 | 14×14 px centrado | Trampa (roja en editor) |
| [2] Enemigo | `Rectangulo` | 1 | 33×33 px centrado | Spawn enemigo (verde) |
| [3] Llave | `Rectangulo` | 1 | 12×12 px arriba-centro | Llave (amarilla) |
| [4] Prop pared | `Rectangulo` | 2 | 16×24 px centrado-arriba | Lámpara / antorcha (props 1-9) |
| [5] Prop suelo | `Rectangulo` | 1 | 48×48 px centrado | Charco / mancha / huesos (props 10-19) |
| [6] Pared PNG | `Imagen` | 0 | 64×64 px | PNG de pared si `texturasParedCargadas == true`; oculto en caso contrario |

`refrescarDibujos()` aplica la visibilidad:
- `VIS_OCULTO` → Rectangulo[0] negro opaco, Imagen[6] oculta, resto alfa=0.
- `VIS_VISIBLE BASE_PARED` con PNG → Imagen[6] visible con textura de la variante; Rectangulo[0] transparente.
- `VIS_VISIBLE` otros tiles o sin PNG → colores normales; drawables [4] y [5] mutuamente excluyentes.

---

## 7. Fog of War

### Algoritmo `revelarZonaDesde(fila, col)`

BFS desde la posición del jugador con estas reglas por tipo de vecino:

- `BASE_SUELO` → encolar (BFS se propaga).
- `BASE_PARED` → marcar `VIS_VISIBLE` directamente, NO encolar (se ve el muro, no se cruza).
- `BASE_PUERTA_*` → marcar `VIS_VISIBLE` directamente, NO encolar (se ve la puerta, no el otro lado).

Las zonas reveladas nunca vuelven a oscurecerse (`VIS_VISIBLE` es permanente).

### Cuándo se llama

- `Jugador::inicia()` — revela la zona del spawn al iniciar la partida.
- `Jugador::intentarMover()` — revela la zona del nuevo tile tras cada movimiento exitoso.
- `EditorMapa::actualiza()` — llama a `mapa->revelarTodo()` al activar el editor (F1 on).

### Regla de diseño de niveles

Las habitaciones deben estar **completamente cerradas por paredes**, con **puertas como única conexión** entre ellas. Si dos habitaciones comparten suelo sin puerta intermedia, el BFS las trata como una sola zona.

---

## 8. Jugador (Jugador.h / Jugador.cpp)

- Movimiento por casillas, pulsación única (edge detection por flag anterior).
- Sprite: `hero_idle.png` (32×32 px), cargado desde `assets/textures/characters/hero/`.
- Offset de centrado: `(tamCasilla - 32) / 2` en X e Y.
- `esTransitable()` bloquea paredes, puertas cerradas y enemigos.
- `ponBloqueado(true/false)` — el editor lo bloquea mientras está activo.
- `ponHud(Hud*)` — conecta el HUD para recibir mensajes (puede ser nullptr, seguro).
- `enviarMensaje(msg)` — helper privado que llama `hud->agregarMensaje(msg)` si hud != nullptr.
- Mensajes enviados: "Llave recogida", "Puerta abierta", "Necesitas una llave para abrir esta puerta".

---

## 9. Editor de mapa (EditorMapa)

- Se activa con **F1** (toggle).
- Al activar: llama a `mapa->revelarTodo()`.
- Al desactivar: llama a `mapa->guardarMapa()`.
- **Enter** guarda sin salir.
- **6 capas** (A/D para ciclar), tipos dentro de la capa (W/S):

| Capa | Color banner | Qué contiene |
|---|---|---|
| 0 BASE | Azul | Suelo, pared, puerta cerrada, puerta abierta |
| 1 PROPS | Morado | Props de pared (1-9) y suelo (10-19) |
| 2 TRAMPAS | Rojo | Trampa fija |
| 3 ENEMIGOS | Verde | Spawn enemigo |
| 4 LLAVES | Dorado | Llave normal |
| 5 VARIANTES | Gris plateado | Variante visual (DEFAULT/NORTE/SUR/ESTE/OESTE/PUERTA_H/PUERTA_V) |

- **Capa PROPS:** el editor traduce el índice de paleta (0-5) al valor de enum (0,1,2,10,11,12). Valida que props de pared solo se coloquen sobre BASE_PARED y viceversa (log en consola si hay violación).
- **Capa VARIANTES:** W/S cicla entre las 7 variantes; clic pinta, clic der resetea a VAR_DEFAULT. Solo aplicable a BASE_PARED y BASE_PUERTA_*.
- **Autodetección al pintar BASE:** cuando se pinta una pared o puerta, el editor llama automáticamente a `detectarVariante()` para el tile y sus 4 vecinos.
- Panel de paleta en `PANEL_X = 1070` con cajas de colores y banner de color por capa.
- Cursor tile-a-tile con flechas, también ratón (clic izq pinta, clic der borra).
- **P** establece el punto de spawn del jugador.
- `screenToTile()` convierte píxeles de pantalla → tile teniendo en cuenta offset cámara y zoom.

---

## 10. Cámara y zoom (JuegoDungeon)

- Modo juego: `offsetCamara = centroPantalla - jugador->centroMundo()`.
- Modo editor: cámara fija (no sigue al jugador).
- Zoom: **Q** acerca (+0.1), **E** aleja (-0.1), rango [1.0, 4.0], default 3.0×.
- `ponZoom()` aplica `view.zoom(1/zoomNivel)` en el motor SFML.
- El panel del editor usa conversión mundo↔pantalla: `mundo_x = (pantalla_x - WIN_CX) / zoom + WIN_CX`.

---

## 11. Mapa de prueba incluido (DungeonJuego/mapa.txt)

3 habitaciones separadas conectadas por pasillos con llave y puerta cerrada (formato DUNGEON_MAP 1):

| Zona | Posición | Contenido | Conexión |
|---|---|---|---|
| Sala A — inicio | Filas 3–9, cols 2–11 | Spawn (6,6) | Puerta ABIERTA (6,12) → pasillo |
| Pasillo H | Filas 5–7, cols 13–18 | — | Puerta ABIERTA (6,19) → Sala B |
| Sala B — llave | Filas 3–9, cols 20–29 | Llave (6,25) | Puerta ABIERTA (10,24) → pasillo |
| Pasillo V | Filas 11–18, col 24 | — | Puerta CERRADA (19,24) → Sala C |
| Sala C — objetivo | Filas 20–27, cols 12–28 | — | — |

**Flujo del puzle:** el jugador parte en Sala A → recorre el pasillo horizontal hasta Sala B → recoge la llave → baja por el pasillo vertical → abre la puerta cerrada → entra en Sala C.

---

## 12. Estado de implementación de funcionalidades

### ✅ Props ampliados + variantes visuales + fondo negro (IMPLEMENTADO — 2026-04-11)

**Archivos modificados:**
- `UNIR-2D/Rendidor.cpp` — `window->clear(sf::Color::Black)` (antes gris).
- `UNIR-2D/Mapa.h` — `PROP_CHARCO=10`, `PROP_MANCHA=11`, `PROP_HUESOS=12`; helpers `esPropDePared/esPropDeSuelo`; enum `VarianteVisual`; `getVariante/setVariante/detectarVariante`; `dibujosPropsS`.
- `UNIR-2D/Mapa.cpp` — implementaciones de variantes; 6 drawables por tile; `colorBase()` con variantes; `colorPropPared/colorPropSuelo`; guardado v2 + carga retrocompatible.
- `UNIR-2D/EditorMapa.h/.cpp` — 6 capas, capa VARIANTES, validación de props, autodetección al pintar.
- `DungeonJuego/assets/textures/tiles/` — carpetas con README.txt para artistas.

---

### ✅ Sistema de llaves (IMPLEMENTADO — 2026-04-11)

**Archivos modificados:**
- `UNIR-2D/Mapa.h` — añadido `getLlave(int fila, int col) const` a la API pública.
- `UNIR-2D/Mapa.cpp` — implementación de `getLlave()`.
- `DungeonJuego/Jugador.h` — añadido `int llaves = 0` (privado) y `int getLlaves() const` (público).
- `DungeonJuego/Jugador.cpp` — `intentarMover()` ampliado con lógica de recoger llaves y abrir puertas.

**Comportamiento implementado:**
- Al pisar un tile con `LLAVE_PUBLICA_NORMAL`: `llaves++` + `setLlave(LLAVE_PUBLICA_NADA)` → la llave desaparece del mapa.
- Al intentar entrar en `BASE_PUERTA_CERRADA` con `llaves > 0`: `setBase(BASE_PUERTA_ABIERTA)` + `llaves--` + el jugador se mueve al tile + se revela la nueva zona.
- Al intentar entrar en `BASE_PUERTA_CERRADA` con `llaves == 0`: movimiento cancelado silenciosamente.
- `setBase()` llama internamente a `refrescarDibujos()` → la puerta cambia de color visualmente de forma automática.

**Acceso al contador desde fuera del Jugador:**
```cpp
int llavesTotales = jugador->getLlaves();   // para el HUD
```

---

### ✅ HUD fijo — panel stats + consola de mensajes (IMPLEMENTADO — 2026-04-11)

**Archivos creados:**
- `DungeonJuego/Hud.h` — declaración de la clase Hud.
- `DungeonJuego/Hud.cpp` — implementación (dos paneles + cola de mensajes + font).
- `DungeonJuego/fuentes/README.txt` — instrucciones para añadir DejaVuSans.ttf.
- `DungeonJuego/assets/textures/ui/README.txt` — especificaciones de iconos para artistas.
- `DungeonJuego/assets/fonts/README.txt` — carpeta de entrega para artistas.

**Archivos modificados:**
- `DungeonJuego/JuegoDungeon.h` — añadido `#include "Hud.h"` y `Hud* hud = nullptr`.
- `DungeonJuego/JuegoDungeon.cpp` — `inicia()` crea Hud y lo conecta al jugador; `posactualiza()` pasa el zoom; `termina()` extrae y elimina el Hud.
- `DungeonJuego/Jugador.h` — `class Hud` forward, `ponHud(Hud*)`, `Hud* hud = nullptr`, `enviarMensaje()`.
- `DungeonJuego/Jugador.cpp` — `#include "Hud.h"`, implementación de `enviarMensaje()`, 3 llamadas en `intentarMover()`.

**Comportamiento implementado:**
- `panelStats` (200×90 px, esquina sup-izq): negro semitransparente. Placeholder esperando assets.
- `panelConsola` (320×160 px, esquina inf-izq): azul oscuro semitransparente. Cola de 5 mensajes.
- Mensajes duran 4 s por defecto; fade-out en el último segundo.
- Si hay más de 5 mensajes simultáneos, el más antiguo se descarta.
- Texto renderizado con `unir2d::Texto("DejaVuSans")`. Si la fuente no existe: paneles visibles, texto ausente (sin crash).
- Fuente buscada en `{cwd}/fuentes/DejaVuSans.ttf` (copiar desde `UNIR-2D/fuentes/`).
- Tamaño de carácter escalado por zoom para mantener 12 px aparentes en pantalla.
- Mensaje de bienvenida al inicio: `"Bienvenido al dungeon. Encuentra las llaves y explora."` (6 s).

**Acceso al HUD:**
```cpp
hud->agregarMensaje("texto");                // duración por defecto 4 s
hud->agregarMensaje("texto urgente", 6.0f);  // duración personalizada
```

---

### ✅ Texturas PNG en paredes direccionales (IMPLEMENTADO — 2026-04-20)

**Archivos modificados:**
- `UNIR-2D/Mapa.h` — añadidos `dibujosBaseImg` (vector Imagen*), 5 punteros `texPared*`, `texturasParedCargadas`.
- `UNIR-2D/Mapa.cpp` — carga de 5 texturas en `inicia()` (patrón Jugador, 5 rutas, todo-o-nada); `Imagen*` por tile en `crearDibujos()`; lógica PNG/placeholder en `refrescarDibujos()`; limpieza en `termina()` y `limpiarListasDibujos()`.

**Comportamiento:**
- Si los 5 PNGs existen en `assets/textures/tiles/base/`: paredes muestran sprite según variante.
- Si falta cualquier PNG: log en consola + rectangles de color para todas las paredes. Sin crash.
- `VIS_OCULTO` → siempre negro opaco, independientemente de texturas.
- `VAR_OESTE`: usa `texParedOeste` con `ponEscala(-1,1)` + offset X para volteo horizontal (misma textura que `VAR_ESTE` pero en espejo).
- En modo editor (`enModoEditor=true`) todas las paredes muestran placeholder de color para facilitar la edición; al salir del editor se restauran las texturas PNG.
- Suelo y puertas: sin cambios, siguen con Rectangulo placeholder.

---

## 13. HUD (Hud.h / Hud.cpp)

### Dos paneles en coordenadas de pantalla fijas

El HUD usa exactamente el mismo patrón de coordenadas que `EditorMapa`: el actor actor tiene posición (0,0) y cada drawable se posiciona en coordenadas mundo usando la fórmula:
```cpp
mundo_x = (pantalla_x - WIN_CX) / zoom + WIN_CX
mundo_y = (pantalla_y - WIN_CY) / zoom + WIN_CY
```
Los tamaños de los rectángulos se dividen por zoom para que aparezcan del mismo tamaño en pantalla independientemente del nivel de zoom.

| Panel | Posición pantalla | Tamaño pantalla | Color |
|---|---|---|---|
| `panelStats` | x=10, y=10 | 200×90 px | Negro semitransparente (0,0,0,170) |
| `panelConsola` | x=10, y=WINDOW_H-170 | 320×160 px | Azul oscuro (0,20,60,150) |

### Estructura MensajeConsola

```cpp
struct MensajeConsola {
    std::string texto;
    float tiempoRestante;   // segundos hasta desvanecerse
    float tiempoTotal;      // para calcular alpha de fade
};
std::deque<MensajeConsola> mensajes;  // front=más antiguo, back=más reciente
```

### API pública

```cpp
void ponZoom(float z);                                        // llamar cada frame desde JuegoDungeon
void agregarMensaje(const std::string& texto, float duracion = 4.0f);
```

### Cómo conectar nuevos actores al HUD (patrón ponHud)

```cpp
// En el .h del actor nuevo:
class Hud;           // forward declaration
Hud* hud = nullptr;  // miembro privado
void ponHud(Hud* h) { hud = h; }  // setter público

// En el .cpp del actor nuevo:
#include "Hud.h"
void MiActor::algúnEvento() {
    if (hud) hud->agregarMensaje("algo ocurrió");
}

// En JuegoDungeon::inicia() después de crear el actor:
miActor->ponHud(hud);
```

### TODO_HUD_STATS — qué falta en el panel superior

El `panelStats` es actualmente un rectángulo vacío. Para completarlo:
1. Añadir `unir2d::Texto` con `jugador->getLlaves()` formateado.
2. Añadir `unir2d::Imagen` con `llave_icon.png` desde `assets/textures/ui/`.
3. Añadir `unir2d::Imagen` con `retrato_heroe.png` y `corazon.png` cuando se implemente vida.
Buscar el marcador `TODO_HUD_STATS` en `Hud.cpp` para ver el punto exacto de integración.

### Fuente

`unir2d::Texto` carga fuentes desde `{cwd}/fuentes/{nombre}.ttf`. El HUD usa `"DejaVuSans"`:
- **Archivo:** `DungeonJuego/fuentes/DejaVuSans.ttf`
- **Origen:** copiar desde `UNIR-2D/fuentes/DejaVuSans.ttf`
- **Si no existe:** paneles visibles, texto ausente — sin crash (try/catch en `inicia()`).

### Índices Z del HUD

| Drawable | `ponIndiceZ` |
|---|---|
| Paneles (Rectangulo) | 150 |
| Textos de mensajes | 160 |

---

## 14. Decisiones de diseño tomadas

| Decisión | Motivo |
|---|---|
| Fog of war por zonas (no por radio) | Más adecuado para dungeon por habitaciones; más fácil de implementar limpiamente |
| Solo 2 estados (oculto/visible, sin "recordado") | Simplifica el sistema; las zonas exploradas se recuerdan permanentemente |
| BFS solo propaga por suelo, revela paredes y puertas sin cruzarlas | Evita que el flood-fill "sangre" a través de muros a habitaciones contiguas |
| Puertas como fronteras de zona | Natural en un dungeon; el jugador debe cruzarlas físicamente para revelar la siguiente sala |
| Editor revela todo el mapa al abrirse | El diseñador necesita ver todos los tiles; al cerrar el editor la niebla se reconstruye con el siguiente movimiento |
| Rectangulos de color como placeholders + `TODO_ARTISTA` | Permite trabajar sin assets; los marcadores `TODO_ARTISTA` en Mapa.cpp indican exactamente dónde sustituir por texturas |
| tamCasilla = 64px | Acordado antes de que los artistas creen assets; cambiar después requiere reescalar todos los PNGs |
| Props separados por rango de valor (1-9 pared, 10-19 suelo) | El rango distingue el tipo sin campos extra; los helpers `esPropDePared/esPropDeSuelo` encapsulan la lógica |
| Variantes autodetectadas al pintar + sobreescritura manual | El flujo normal es automático; el diseñador puede ir a capa VARIANTES para ajustar casos especiales |
| Fondo del motor negro (`sf::Color::Black`) | Los tiles `VIS_OCULTO` son negro opaco; el fondo gris anterior creaba bordes visibles en los extremos del mapa |
| `TODO_ARTISTA` como marcador estándar | Permite hacer `grep TODO_ARTISTA Mapa.cpp` para encontrar todos los puntos de integración de assets |
| HUD en coordenadas de pantalla fijas usando el mismo patrón que EditorMapa | Los drawables del HUD se posicionan con `(sx-WIN_CX)/zoom+WIN_CX`; ni el HUD ni el editor reciben `ponPosicion(offsetCamara)` |
| Mensajes con doble expiración: empuje por cola llena + fade por tiempo | Si llegan >5 mensajes simultáneos el más antiguo se descarta; los mensajes con tiempo restante <1s hacen fade |
| Jugador desacoplado del HUD mediante puntero opcional (puede ser nullptr) | `enviarMensaje()` comprueba `if (hud)` antes de llamar; se puede instanciar Jugador sin HUD sin romper nada |
| Texturas de pared con fallback a placeholder: si falta cualquier PNG se usa el sistema de colores completo sin crashear | Todo-o-nada: cargar los 5 PNGs o ninguno; el juego siempre es funcional |
| Mismo patrón de carga de assets que Jugador (5 rutas, try/catch, log si falla) | Coherencia; Jugador::cargarSpriteJugador() es el único precedente en el proyecto. La 5ª ruta cubre el caso exe en UNIR-2D/x64/Debug/ |
| VAR_OESTE usa la misma textura que VAR_ESTE pero con flip horizontal (ponEscala -1,1) | pared_este.png y pared_oeste.png son la misma geometría de ladrillo; el espejo evita entregar un PNG adicional |
| Editor muestra placeholders de color aunque haya PNGs cargados | Los tonos de variante (gris azulado, rojizo…) son más útiles para editar que la textura real |

---

## 15. Convenciones de código

- Clases del motor en namespace `unir2d::`.
- Métodos en español (camelCase): `ponColor`, `ponPosicion`, `agregaDibujo`, `extraeDibujos`.
- Enums de tipos de tile con prefijo de capa: `BASE_SUELO`, `PROP_LAMPARA`, `TRAMPA_FIJA`, etc.
- Los enums públicos de Mapa tienen sufijo `Public` para los que no coinciden con los internos (`TipoTrampaPublic`, etc.).
- Edge detection de teclado: variable `xxxPrev` por cada tecla relevante.
- `ponIndiceZ(n)`: controla el orden de dibujado (mayor Z = encima). Base=0, overlays=1, props=2, editor UI=50-52, cursor editor=100.
