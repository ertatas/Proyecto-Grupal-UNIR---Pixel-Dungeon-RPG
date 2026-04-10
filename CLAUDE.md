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
│   └── Color.h                 ← Color RGBA inmutable (métodos: rojo(), verde(), azul(), alfa())
│
├── DungeonJuego/               ← Código del juego
│   ├── JuegoDungeon.h / .cpp   ← Clase principal: cámara, zoom, actores
│   ├── Jugador.h / .cpp        ← Movimiento, sprite, fog of war
│   ├── DungeonJuego.cpp        ← main() — instancia JuegoDungeon y arranca el motor
│   ├── mapa.txt                ← Mapa guardado (cargado automáticamente al arrancar)
│   └── assets/
│       └── textures/
│           └── characters/
│               └── hero/
│                   └── hero_idle.png   ← sprite del héroe (YA EXISTE)
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
    └── Jugador               (movimiento, sprite, colisiones)
```

### Relaciones entre clases

```
JuegoDungeon
    owns → Mapa*
    owns → Jugador*(Mapa*)
    owns → EditorMapa*(Mapa*)
```

`Jugador` y `EditorMapa` reciben un puntero a `Mapa` en su constructor.
`JuegoDungeon::inicia()` los agrega al motor en este orden: Mapa → EditorMapa → Jugador.

---

## 5. Sistema de mapa (Mapa.h / Mapa.cpp)

### Capas lógicas

El mapa tiene **6 capas**, cada una es un `std::vector<std::vector<int>>` de tamaño `filas × columnas`:

| Capa | Variable | Tipos |
|---|---|---|
| Base (estructura) | `capaBase` | `BASE_SUELO=0`, `BASE_PARED=1`, `BASE_PUERTA_CERRADA=2`, `BASE_PUERTA_ABIERTA=3` |
| Props (decoración) | `capaProps` | `PROP_NADA=0`, `PROP_LAMPARA=1`, `PROP_ANTORCHA=2` |
| Trampas | `capaTrampas` | `TRAMPA_NADA=0`, `TRAMPA_FIJA=1` |
| Enemigos | `capaEnemigos` | `ENEMIGO_NADA=0`, `ENEMIGO_NORMAL=1` |
| Llaves | `capaLlaves` | `LLAVE_NADA=0`, `LLAVE_NORMAL=1` |
| Visibilidad | `capaVisibilidad` | `VIS_OCULTO=0`, `VIS_VISIBLE=1` |

También existe `capaVisual` (struct `CoordenadasVisuales`) para variantes visuales de paredes.

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
int  getBase(int fila, int col) const;
int  getProp(int fila, int col) const;
int  getSpawnFila() / getSpawnCol() const;
bool esTransitable(int fila, int col) const;   // false en pared, puerta cerrada, enemigo
Vector centroCasilla(int fila, int col) const;
float tamanoCasilla() const;

// Modificación (usada por el editor)
void setBase/setProp/setTrampa/setEnemigo/setLlave(int fila, int col, int tipo);
void setSpawn(int fila, int col);

// Fog of war
void revelarZonaDesde(int fila, int col);   // BFS desde posición del jugador
void revelarTodo();                          // revela todo el mapa (editor)

// Persistencia
void guardarMapa(const std::string& ruta = "") const;
void cargarMapa (const std::string& ruta = "");
```

### Persistencia — formato mapa.txt

```
DUNGEON_MAP 1
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
SPAWN
<fila_spawn> <col_spawn>
END
```

El fichero `mapa.txt` vive en `DungeonJuego/`. Si existe al arrancar, se carga automáticamente; si no, se usa el mapa por defecto de `construirMapaInicial()`.

---

## 6. Sistema de drawables del mapa

Por cada tile se crean **5 Rectangulo**** en `crearDibujos()`:

| Drawable | `ponIndiceZ` | Tamaño | Para qué |
|---|---|---|---|
| Base | 0 | 64×64 px | Suelo / pared / puerta |
| Trampa | 1 | 14×14 px centrado | Trampa (roja en editor) |
| Enemigo | 1 | 33×33 px centrado | Spawn enemigo (verde) |
| Llave | 1 | 12×12 px arriba-centro | Llave (amarilla) |
| Prop | 2 | 16×24 px centrado-arriba | Lámpara / antorcha |

`refrescarDibujos()` aplica la visibilidad: si `capaVisibilidad[f][c] == VIS_OCULTO` → base negro opaco, resto transparentes.

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

---

## 9. Editor de mapa (EditorMapa)

- Se activa con **F1** (toggle).
- Al activar: llama a `mapa->revelarTodo()`.
- Al desactivar: llama a `mapa->guardarMapa()`.
- **Enter** guarda sin salir.
- 5 capas (A/D para ciclar), tipos dentro de la capa (W/S).
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

3 habitaciones en fila horizontal, filas 3–9 del mapa 32×32:

| Habitación | Cols (suelo) | Contenido | Conexión |
|---|---|---|---|
| A — inicio | 3–11 | Spawn (6,5) · trampa (6,11) · antorcha (4,5) | Puerta ABIERTA (6,12) → B |
| B — peligro | 13–21 | Enemigo (5,17) · lámpara (4,17) | Puerta CERRADA (6,22) → C |
| C — objetivo | 23–29 | Llave (5,26) · antorcha (4,26) | — |

---

## 12. Próximas funcionalidades a implementar

### Sesión siguiente: sistema de llaves + HUD fijo

**Sistema de llaves:**
- El jugador recoge una llave al pisar su tile (capa LLAVES).
- Al pisar una `BASE_PUERTA_CERRADA` con llave en el inventario, la puerta se convierte en `BASE_PUERTA_ABIERTA`.
- `Jugador` necesita un contador de llaves (`int llaves = 0`).
- En `intentarMover()`, si el tile destino es `BASE_PUERTA_CERRADA` y el jugador tiene llaves: abrir puerta y restar llave. Si no tiene llaves: el movimiento falla (puerta bloqueada).
- Tras recoger una llave: `mapa->setLlave(fila, col, LLAVE_NADA)` + `refrescarDibujos()`.
- Tras abrir una puerta: `mapa->setBase(fila, col, BASE_PUERTA_ABIERTA)` + llamar `revelarZonaDesde` desde la nueva posición.

**HUD fijo:**
- Capa de UI dibujada en coordenadas de pantalla fijas (independiente del zoom y la cámara).
- Mostrar al menos: número de llaves recogidas, (opcionalmente: vida, nombre del nivel).
- Implementar como un actor nuevo `Hud` que se añade DESPUÉS del jugador en `JuegoDungeon::inicia()` para que dibuje encima de todo.
- Usar `unir2d::Texto` para mostrar texto o `unir2d::Rectangulo` + `Imagen` para iconos.
- Posición fija en pantalla: esquina superior izquierda o inferior izquierda.

---

## 13. Decisiones de diseño tomadas

| Decisión | Motivo |
|---|---|
| Fog of war por zonas (no por radio) | Más adecuado para dungeon por habitaciones; más fácil de implementar limpiamente |
| Solo 2 estados (oculto/visible, sin "recordado") | Simplifica el sistema; las zonas exploradas se recuerdan permanentemente |
| BFS solo propaga por suelo, revela paredes y puertas sin cruzarlas | Evita que el flood-fill "sangre" a través de muros a habitaciones contiguas |
| Puertas como fronteras de zona | Natural en un dungeon; el jugador debe cruzarlas físicamente para revelar la siguiente sala |
| Editor revela todo el mapa al abrirse | El diseñador necesita ver todos los tiles; al cerrar el editor la niebla se reconstruye con el siguiente movimiento |
| Rectangulos de color como placeholders | Permite trabajar sin assets; se sustituyen por Textura+Imagen cuando el artista entregue los PNGs |
| tamCasilla = 64px | Acordado antes de que los artistas creen assets; cambiar después requiere reescalar todos los PNGs |

---

## 14. Convenciones de código

- Clases del motor en namespace `unir2d::`.
- Métodos en español (camelCase): `ponColor`, `ponPosicion`, `agregaDibujo`, `extraeDibujos`.
- Enums de tipos de tile con prefijo de capa: `BASE_SUELO`, `PROP_LAMPARA`, `TRAMPA_FIJA`, etc.
- Los enums públicos de Mapa tienen sufijo `Public` para los que no coinciden con los internos (`TipoTrampaPublic`, etc.).
- Edge detection de teclado: variable `xxxPrev` por cada tecla relevante.
- `ponIndiceZ(n)`: controla el orden de dibujado (mayor Z = encima). Base=0, overlays=1, props=2, editor UI=50-52, cursor editor=100.
