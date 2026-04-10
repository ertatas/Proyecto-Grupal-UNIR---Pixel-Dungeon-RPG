# Guía del Editor de Mapas y Texturas — Pixel Dungeon RPG
### Para el equipo de arte y diseño

---

> ## ⚠️ DECISIONES PREVIAS OBLIGATORIAS — leer antes de pintar nada
>
> Antes de que los artistas creen **ningún asset** (tile, personaje, prop…) el equipo debe acordar y fijar estos tres valores. Cambiarlos después obliga a reescalar o rehacer todos los PNGs ya entregados.
>
> | Decisión | Valor actual | Dónde cambiarlo |
> |---|---|---|
> | **Tamaño del tile** | 64×64 px | `UNIR-2D/Mapa.h` → `tamCasilla` |
> | **Tamaño del mapa** | 32×32 casillas | `UNIR-2D/Mapa.cpp` → `construirMapaInicial()` |
> | **Resolución de ventana** | 1280×720 | `DungeonJuego/JuegoDungeon.cpp` → `ANCHO_VENTANA / ALTO_VENTANA` |
>
> Una vez acordados, avisad al programador para que actualice el código **antes** de que empecéis a exportar sprites.

---

## Índice
1. [Cómo acceder al editor](#1-cómo-acceder-al-editor)
2. [Controles del editor](#2-controles-del-editor)
3. [Las 5 capas del mapa](#3-las-5-capas-del-mapa)
4. [Paleta de colores por capa](#4-paleta-de-colores-por-capa)
5. [Punto de spawn del jugador](#5-punto-de-spawn-del-jugador)
6. [Flujo de trabajo recomendado](#6-flujo-de-trabajo-recomendado)
7. [Guardado y carga](#7-guardado-y-carga)
8. [Zoom de cámara](#8-zoom-de-cámara)
9. [Cambiar la resolución de la ventana](#9-cambiar-la-resolución-de-la-ventana)
10. [Cambiar el tamaño del mapa](#10-cambiar-el-tamaño-del-mapa)
11. [Cambiar el tamaño del tile (96×96)](#11-cambiar-el-tamaño-del-tile-9696)
12. [Cómo añadir texturas a los elementos](#12-cómo-añadir-texturas-a-los-elementos)
13. [Niebla de guerra (Fog of War)](#13-niebla-de-guerra-fog-of-war)

---

## 1. Cómo acceder al editor

Lanza el juego normalmente y pulsa **F1**.

El jugador queda congelado. Aparece:
- Un **cursor de color** (cuadrado semitransparente) que puedes mover por el mapa
- Un **panel de paleta** en el lado derecho de la pantalla

Pulsa **F1** de nuevo para salir. El mapa se guarda automáticamente al salir.

---

## 2. Controles del editor

| Tecla | Acción |
|-------|--------|
| **F1** | Activar / desactivar editor (guarda al salir) |
| **Enter** | Guardar sin salir |
| **Flechas** | Mover el cursor tile a tile (atraviesa paredes) |
| **D** | Siguiente capa |
| **A** | Capa anterior |
| **S** | Siguiente tipo dentro de la capa actual |
| **W** | Tipo anterior dentro de la capa actual |
| **Espacio** | Pintar el tile en la posición del cursor |
| **P** | Marcar el tile del cursor como punto de spawn del jugador |
| **Clic izquierdo** | Pintar el tile bajo el ratón |
| **Clic derecho** | Borrar el tile bajo el ratón (lo pone a "nada") |

> El cursor **atraviesa paredes** — puedes colocarlo sobre cualquier tile sin restricciones.

---

## 3. Las 5 capas del mapa

Cambia de capa con **A / D**. El **color del banner** (banda superior del panel) y el **color del cursor** indican en qué capa estás.

| Color banner | Color cursor | Capa | Qué contiene | ¿Afecta al juego? |
|---|---|---|---|---|
| 🟦 Azul | Azul claro | **BASE** | Suelo, paredes, puertas | Sí — colisiones |
| 🟣 Morado | Violeta | **PROPS** | Decoración (lámpara, antorcha) | No — solo visual |
| 🔴 Rojo | Rojo | **TRAMPAS** | Trampas ocultas | Sí — daño al pisarlas |
| 🟢 Verde | Verde | **ENEMIGOS** | Puntos de spawn | Sí — aparece un enemigo |
| 🟡 Dorado | Amarillo | **LLAVES** | Llaves recogibles | Sí — abre puertas |

**Importante:** Las capas se apilan. Puedes tener en la misma casilla, por ejemplo, un suelo (BASE) + una antorcha (PROPS). No se borran entre sí.

---

## 4. Paleta de colores por capa

El **borde amarillo brillante** en el panel indica qué tipo está seleccionado.

### Capa BASE — Azul (estructura del nivel)
| Color | Tipo | Comportamiento en juego |
|---|---|---|
| ⬜ Gris medio | **Suelo** | El jugador camina sobre él |
| ⬛ Gris oscuro | **Pared** | Bloquea el movimiento |
| 🟫 Marrón oscuro | **Puerta cerrada** | Bloquea; se abre con una llave |
| 🟤 Marrón claro | **Puerta abierta** | El jugador puede pasar |

### Capa PROPS — Morado (decoración visual)
| Color | Tipo | Notas |
|---|---|---|
| ⬛ Gris muy oscuro | **Nada** (borrar) | Equivale a clic derecho |
| 🩵 Cian | **Lámpara** | Encima de pared o suelo |
| 🟠 Naranja | **Antorcha** | Encima de pared o suelo |

> **Para añadir un nuevo prop** (barril, cofre, columna…) el programador debe hacer exactamente esto — son 4 pasos, ninguno toca arquitectura:
>
> 1. **`UNIR-2D/Mapa.h`** — añadir una constante al enum `TipoProp`:
>    ```cpp
>    PROP_BARRIL = 3,   // ← ejemplo
>    ```
> 2. **`UNIR-2D/Mapa.cpp`** → función `colorProp()` — añadir un `case` con el color placeholder:
>    ```cpp
>    case PROP_BARRIL: return Color(120, 70, 20, 255);
>    ```
> 3. **`UNIR-2D/EditorMapa.cpp`** → función `maxTipos()` — incrementar el `return` de la capa 1:
>    ```cpp
>    case 1: return 4;   // era 3, ahora 4
>    ```
> 4. **`UNIR-2D/EditorMapa.cpp`** → función `colorTipoPaleta()`, bloque `case 1` — añadir el color de la paleta para el nuevo tipo:
>    ```cpp
>    case 3: return Color(120, 70, 20);   // barril
>    ```
>
> Cuando el artista entregue el PNG, el programador sustituye el `Rectangulo` de ese prop por `Textura + Imagen` en `Mapa.cpp` siguiendo el patrón del jugador (ver sección 11).

### Capa TRAMPAS — Rojo (solo visible en el editor)
| Color | Tipo | Notas |
|---|---|---|
| ⬛ Gris muy oscuro | **Nada** (borrar) | Sin trampa |
| 🔴 Rojo vivo | **Trampa fija** | En el juego real el jugador NO la ve |

> Pon trampas solo sobre tiles de **suelo**. En paredes no tienen efecto.

### Capa ENEMIGOS — Verde
| Color | Tipo | Notas |
|---|---|---|
| ⬛ Gris muy oscuro | **Nada** (borrar) | Sin enemigo |
| 🟢 Verde vivo | **Spawn enemigo** | Aquí aparecerá un enemigo al iniciar |

> Un solo spawn por casilla. Solo sobre **suelo**.

### Capa LLAVES — Dorado
| Color | Tipo | Notas |
|---|---|---|
| ⬛ Gris muy oscuro | **Nada** (borrar) | Sin llave |
| 🟡 Amarillo dorado | **Llave** | El jugador la recoge al pisarla |

> Por cada llave debe haber una **puerta cerrada** en la capa BASE.
> Solo sobre **suelo**.

---

## 5. Punto de spawn del jugador

El **punto de spawn** es la casilla donde el jugador aparece al iniciar el juego.

### Cómo se ve

Un **cuadrado cian semitransparente** (50% del tile, centrado) marca la posición de spawn. Es visible tanto dentro como fuera del editor para que siempre sepas dónde empieza el jugador.

### Cómo cambiarlo

1. Abre el editor con **F1**
2. Mueve el cursor con las flechas hasta la casilla de suelo donde quieres el spawn
3. Pulsa **P** — el marcador cian salta inmediatamente a esa posición
4. Guarda con **Enter** o sal con **F1**

> Pon el spawn solo sobre un tile de **suelo**. Si lo pones sobre una pared el jugador quedará atrapado al iniciar.

### Persistencia

El spawn se guarda en `mapa.txt` junto con el resto del mapa. Si no existe `mapa.txt`, el spawn por defecto es la casilla (fila 1, columna 1).

---

## 6. Flujo de trabajo recomendado

### Diseñar la estructura
1. **F1** → editor activo, banner **azul** (capa BASE)
2. **W/S** para elegir tipo → **Espacio** o **clic izq** para pintar
3. **Clic derecho** para borrar

### Añadir decoración (props)
1. **D** hasta banner **morado** (PROPS)
2. Selecciona lámpara o antorcha con **W/S**
3. Mueve el cursor a la casilla → **Espacio**

### Poner trampas
1. **D** hasta banner **rojo** (TRAMPAS)
2. Selecciona Trampa fija → **Espacio** sobre suelo

### Colocar enemigos y llaves
1. **D** hasta el banner correspondiente (**verde** o **dorado**)
2. **Espacio** sobre el tile de suelo deseado

### Guardar
- **Enter** = guardar en cualquier momento sin salir
- **F1** = salir y guardar

---

## 7. Guardado y carga

El mapa se guarda en:
```
DungeonJuego/mapa.txt
```

- Cada vez que **sales del editor (F1)** o pulsas **Enter** dentro del editor, el mapa se escribe en ese fichero.
- Al **arrancar el juego**, si `mapa.txt` existe se carga automáticamente.
- Si quieres **volver al mapa por defecto**, borra o renombra el `mapa.txt`.
- El fichero es texto plano — puedes abrirlo con el Bloc de Notas si necesitas inspeccionar algo.

---

## 8. Zoom de cámara

El juego arranca con un zoom de **3×** — el personaje de 32 px se renderiza como 96 px en pantalla, lo que da buen detalle visual. Puedes ajustarlo en cualquier momento durante la partida:

| Tecla | Acción |
|-------|--------|
| **Q** | Acercar (zoom in) — pasos de 0.1× |
| **E** | Alejar (zoom out) — pasos de 0.1× |

**Rango permitido:** 1× (tamaño real, sin zoom) hasta 4× (máximo acercamiento).

| Zoom | Personaje (32 px sprite) en pantalla | Tiles (64 px) en pantalla |
|------|--------------------------------------|---------------------------|
| 1×   | 32 px                                | 64 px                     |
| 2×   | 64 px                                | 128 px                    |
| **3× (por defecto)** | **96 px**        | **192 px**                |
| 4×   | 128 px                               | 256 px                    |

El zoom funciona tanto en modo juego como en modo editor. La cámara siempre permanece centrada en el jugador independientemente del nivel de zoom.

### Cambiar el zoom por defecto

**Archivo:** `DungeonJuego/JuegoDungeon.h`

```cpp
float zoomNivel = 3.0f;   // ← cámbialo entre 1.0 y 4.0
```

---

## 9. Cambiar la resolución de la ventana  

**Archivo:** `DungeonJuego/JuegoDungeon.cpp`
**Líneas 6-7:**

```cpp
static const float ANCHO_VENTANA = 1280.0f;
static const float ALTO_VENTANA  =  720.0f;
```

Cambia estos dos números por la resolución que quieras, por ejemplo `1920` y `1080`.

**⚠️ Qué hay que revisar si cambias la resolución:**
- El panel del editor está posicionado en X=1070. Si bajas la resolución por debajo de ~1200 el panel quedará cortado. Habrá que ajustar `PANEL_X` en `UNIR-2D/EditorMapa.cpp` (línea con `static const float PANEL_X`).
- El jugador y la cámara se centran automáticamente; no hace falta tocar más código.

---

## 10. Cambiar el tamaño del mapa

**Archivo:** `UNIR-2D/Mapa.cpp`
**Función:** `construirMapaInicial()`
**Línea:**

```cpp
crearCapas(32, 32);   // ← 32 filas × 32 columnas
```

Cambia los dos números. El primero es **filas** (alto) y el segundo **columnas** (ancho).

**⚠️ Qué hay que revisar si cambias el tamaño del mapa:**
- Si tienes un `mapa.txt` guardado con el tamaño antiguo, **bórralo** antes de lanzar el juego. Si no, al cargarlo el juego usará el tamaño del fichero, no el del código.
- Mapas muy grandes (más de 64×64) pueden ralentizar el arranque porque se crean muchos rectángulos de color. Cuando se cambien los rectángulos por texturas esto mejora.

---

## 11. Cambiar el tamaño del tile (64×64)

**Archivo:** `UNIR-2D/Mapa.h`
**Línea:**

```cpp
const float tamCasilla = 64.0f;   // ← tamaño en píxeles de cada casilla
```

**⚠️ Esto es el cambio más delicado del proyecto. Si lo tocas:**

| Qué afecta | Qué hay que hacer |
|---|---|
| **Todas las texturas** | Deben ser cuadradas y del mismo tamaño que `tamCasilla`. Si cambias a 64, todas las PNGs deben ser 64×64. |
| **Sprite del jugador** | En `Jugador.cpp` hay un offset hardcodeado que asume 32×32 px de sprite. Si cambias el tile habrá que ajustarlo. |
| **Panel del editor** | El panel está en coordenadas de pantalla fijas y no depende del tile — no hace falta tocarlo. |
| **Cámara** | Se recalcula automáticamente — no hace falta tocar nada. |

**Recomendación:** decidid el tamaño de tile definitivo antes de que los artistas entreguen las primeras texturas, para no tener que reescalar todo.

---

## 12. Cómo añadir texturas a los elementos

### Cómo funciona ahora (placeholder de color)

Actualmente todos los tiles son **rectángulos de color** (`Rectangulo`). El motor ya tiene un sistema de texturas (`Textura` + `Imagen`) que funciona exactamente igual que el sprite del jugador.

### Referencia: el personaje ya tiene textura

En `DungeonJuego/Jugador.cpp` puedes ver el patrón completo:

```cpp
texturaJugador = new Textura();
imagenJugador  = new Imagen();
texturaJugador->carga("assets/textures/characters/hero/hero_idle.png");
imagenJugador->asigna(texturaJugador);
agregaDibujo(imagenJugador);
```

El PNG del héroe está en:
```
DungeonJuego/assets/textures/characters/hero/hero_idle.png
```

### Estructura de carpetas para los artistas

Usad esta estructura para entregar los assets. El programador los conectará al código:

```
DungeonJuego/assets/
├── textures/
│   ├── characters/
│   │   ├── hero/
│   │   │   └── hero_idle.png          ← YA EXISTE
│   │   └── enemy/
│   │       └── enemy_idle.png         ← pendiente
│   ├── tiles/
│   │   ├── floor.png                  ← suelo
│   │   ├── wall.png                   ← pared
│   │   ├── door_closed.png            ← puerta cerrada
│   │   └── door_open.png              ← puerta abierta
│   └── props/
│       ├── lamp.png                   ← lámpara
│       ├── torch.png                  ← antorcha
│       ├── key.png                    ← llave
│       └── trap.png                   ← trampa (visible en editor, oculta en juego)
└── fonts/
    └── (fuentes .ttf para el HUD)
```

### Especificaciones para los artistas

| Elemento | Tamaño PNG | Notas |
|---|---|---|
| Tiles (suelo, pared, puerta) | **64×64 px** (o el valor de `tamCasilla`) | Fondo transparente si el tile no es cuadrado |
| Props (lámpara, antorcha) | Libre, máx 64×64 | Se centran dentro del tile |
| Llave | Libre, máx 32×32 | Se coloca en la mitad superior del tile |
| Trampa | Libre, máx 64×64 | Solo visible en el editor |
| Héroe / enemigos | **32×32 px** por frame | Spritesheet horizontal (todos los frames en fila) |

### Spritesheets (animaciones)

El motor soporta spritesheets. Si el héroe tiene 4 frames de idle, el PNG sería:

```
[frame1][frame2][frame3][frame4]   → 128×32 px total
```

En código se usa así (el programador lo conecta):
```cpp
imagen->defineEstampas(1, 4);      // 1 fila, 4 columnas
imagen->seleccionaEstampa(1, 2);   // mostrar frame 2
```

### Proceso de entrega de assets

1. El artista crea el PNG y lo coloca en la carpeta correcta de `assets/`
2. El programador abre el `.cpp` correspondiente, sustituye el `Rectangulo` por `Textura` + `Imagen` apuntando a esa ruta
3. El artista prueba en el juego y ajusta el PNG si hace falta

No hace falta tocar el editor de mapa para nada de esto — los tiles siempre se pintan igual, solo cambia el visual.

---

## Resumen rápido de archivos clave

| ¿Qué quiero cambiar? | Archivo | Variable/Función |
|---|---|---|
| Resolución de ventana | `DungeonJuego/JuegoDungeon.cpp` | `ANCHO_VENTANA`, `ALTO_VENTANA` |
| Tamaño del mapa (filas/columnas) | `UNIR-2D/Mapa.cpp` | `construirMapaInicial()` → `crearCapas(32, 32)` |
| Tamaño del tile | `UNIR-2D/Mapa.h` | `tamCasilla = 64.0f` |
| Spawn del jugador | Editor en juego | Tecla **P** sobre el tile deseado |
| Sprite del héroe | `DungeonJuego/Jugador.cpp` | `cargarSpriteJugador()` |
| Colores de placeholder del mapa | `UNIR-2D/Mapa.cpp` | `colorBase()`, `colorProp()`, etc. |
| Posición del panel del editor | `UNIR-2D/EditorMapa.cpp` | `PANEL_X = 1070.0f` |
| Fog of war | `UNIR-2D/Mapa.cpp` | `revelarZonaDesde()` |

---

## 13. Niebla de guerra (Fog of War)

El juego oculta completamente todo lo que el jugador no ha explorado. La visibilidad se gestiona por **zonas** (habitaciones o pasillos), no por distancia ni radio.

### Los dos estados de visibilidad

| Estado | Visual | Significado |
|---|---|---|
| **VIS_OCULTO** (0) | Negro absoluto — no se dibuja nada | Zona no explorada |
| **VIS_VISIBLE** (1) | Colores normales — se dibuja todo | Zona que el jugador ya ha pisado |

No existe estado intermedio. Una zona es negro absoluto o completamente visible. Una vez revelada, **permanece visible para siempre**.

---

### Concepto de zona

Una **zona** es el conjunto de suelo conectado de una habitación o pasillo, delimitado por puertas. Ejemplos:

- Habitación cuadrada con paredes y una puerta en el norte → una zona.
- Pasillo corto entre dos habitaciones, con puerta en cada extremo → zona propia.

Al **entrar en un tile de suelo** de una zona nueva, ésta se revela al completo — incluidas las paredes que la rodean y las puertas de sus salidas.

---

### Cómo funciona el BFS internamente

`revelarZonaDesde(fila, col)` se llama cada vez que el jugador se mueve:

1. Se encolan los tiles de `BASE_SUELO` empezando desde la posición del jugador.
2. Para cada tile de suelo procesado, sus **vecinos** se tratan así:
   - `BASE_SUELO` → se encola (el BFS sigue propagándose por la zona).
   - `BASE_PARED` → se marca `VIS_VISIBLE` directamente, sin encolar (el BFS no cruza muros).
   - `BASE_PUERTA_*` → se marca `VIS_VISIBLE` directamente, sin encolar (se ve la puerta pero no lo que hay detrás).
3. El BFS termina cuando no quedan suelos por explorar en la zona actual.

> Las paredes se revelan porque son **adyacentes al suelo**, no porque el BFS pase a través de ellas. Esto impide que la niebla se filtre hacia habitaciones contiguas.

---

### Reglas de diseño para que el fog funcione correctamente

Estas reglas son **obligatorias** para el diseñador de niveles. Si no se cumplen, la niebla no se comportará como se espera.

| Regla | Por qué |
|---|---|
| Dos habitaciones siempre separadas por al menos una puerta | Sin puerta, el BFS trata ambas como una sola zona y las revela juntas |
| No dejes suelo "abierto" sin paredes ni puertas en los bordes | El BFS se expande por todo el suelo conectado sin límite |
| Las puertas deben estar EN la pared (reemplazando un tile de pared) | Así actúan como barrera natural entre zonas |
| El spawn del jugador debe estar sobre suelo dentro de una habitación cerrada | Si el spawn está en suelo abierto, la primera revelación puede ser demasiado grande |

**Ejemplo de habitación correcta** (vista en el editor, capa BASE):

```
P P P P P P       P = Pared (gris oscuro)
P . . . . P       . = Suelo (gris medio)
P . . . D P       D = Puerta (marrón)
P . . . . P
P P P P P P
```

El BFS revela todo el interior (suelo + paredes) cuando el jugador entra, y se detiene en la puerta.

---

### Comportamiento en el editor (F1)

Al abrir el editor con **F1**, el mapa se revela completo automáticamente para que puedas ver y editar todos los tiles sin importar si están explorados o no.

Al cerrar el editor, la niebla vuelve a ser la de antes. La próxima vez que el jugador se mueva, la zona actual se recalcula.

---

### Mapa de prueba incluido

Hay un `mapa.txt` de referencia en `DungeonJuego/` con 3 habitaciones:

```
[HAB. A — spawn]  →(puerta abierta)→  [HAB. B — enemigo + trampa]  →(puerta cerrada)→  [HAB. C — llave]
```

- **Hab. A** (cols 3-11, filas 3-9): habitación de inicio, spawn en fila 6 col 5.
- **Hab. B** (cols 13-21): tiene un enemigo en (5,17) y una trampa en (6,11).
- **Hab. C** (cols 23-29): tiene una llave en (5,26) — necesaria para abrir la puerta cerrada.
- Antorchas y lámparas de decoración en fila 4.

Úsalo como referencia de diseño o bórralo y crea el tuyo con el editor.

---

### Archivos clave

| Qué tocar | Archivo | Función/Variable |
|---|---|---|
| Estados de visibilidad (enum) | `UNIR-2D/Mapa.h` | `TipoVisibilidad` |
| Lógica del BFS | `UNIR-2D/Mapa.cpp` | `revelarZonaDesde()` |
| Aplicación visual del fog | `UNIR-2D/Mapa.cpp` | `refrescarDibujos()` |
| Llamada al mover | `DungeonJuego/Jugador.cpp` | `intentarMover()` |
| Llamada al spawn | `DungeonJuego/Jugador.cpp` | `inicia()` |
| Revelar todo (editor) | `UNIR-2D/EditorMapa.cpp` | toggle F1 → `mapa->revelarTodo()` |

---

### Notas para los artistas

Cuando los tiles usen **texturas** en lugar de rectángulos de color, el programador que las conecte debe:

- `VIS_OCULTO` → no dibujar el sprite (o ponerle alpha = 0).
- `VIS_VISIBLE` → dibujar el sprite con color normal (`sf::Color::White`).

Sin gradientes, sin grises intermedios. Negro o visible, nada más.
