# Guía del Editor de Mapas — Pixel Dungeon RPG
### Para el equipo de arte y diseño

---

> ## ⚠️ VALORES FIJOS — acordar antes de crear ningún asset
>
> | Valor | Actual | Dónde cambiarlo |
> |---|---|---|
> | **Tamaño del tile** | 64×64 px | `UNIR-2D/Mapa.h` → `tamCasilla` |
> | **Tamaño del mapa** | 32×32 casillas | `UNIR-2D/Mapa.cpp` → `construirMapaInicial()` |
> | **Resolución de ventana** | 1280×720 | `DungeonJuego/JuegoDungeon.cpp` → `ANCHO_VENTANA / ALTO_VENTANA` |
>
> Cambiar cualquiera de estos valores después de que los artistas hayan entregado assets
> obliga a reescalar o rehacer todos los PNGs. Decidid primero.

---

## Índice

1. [¿Qué es este proyecto y cómo funciona el mapa?](#1-qué-es-este-proyecto-y-cómo-funciona-el-mapa)
2. [Cómo abrir el editor](#2-cómo-abrir-el-editor)
3. [Controles del editor](#3-controles-del-editor)
4. [Las capas del mapa — qué es cada una y cuándo usarla](#4-las-capas-del-mapa--qué-es-cada-una-y-cuándo-usarla)
5. [Guía de props — tipos disponibles y reglas de colocación](#5-guía-de-props--tipos-disponibles-y-reglas-de-colocación)
6. [Sistema de variantes de pared y puerta](#6-sistema-de-variantes-de-pared-y-puerta)
7. [El sistema de fog of war — cómo diseñar niveles que funcionen bien](#7-el-sistema-de-fog-of-war--cómo-diseñar-niveles-que-funcionen-bien)
8. [Sistema de llaves y puertas — cómo crear puzles](#8-sistema-de-llaves-y-puertas--cómo-crear-puzles)
9. [Guardar y cargar el mapa](#9-guardar-y-cargar-el-mapa)
10. [Guía para artistas — cómo integrar tus PNGs sin tocar código](#10-guía-para-artistas--cómo-integrar-tus-pngs-sin-tocar-código)
11. [Referencia rápida de colores placeholder](#11-referencia-rápida-de-colores-placeholder)
12. [Errores frecuentes y cómo resolverlos](#12-errores-frecuentes-y-cómo-resolverlos)
13. [El HUD — paneles y consola de mensajes](#13-el-hud--paneles-y-consola-de-mensajes)

---

## 1. ¿Qué es este proyecto y cómo funciona el mapa?

**Pixel Dungeon RPG** es un juego de mazmorras en vista cenital (top-down) por casillas. El personaje se mueve de casilla en casilla, explorandola mazmorra habitación a habitación.

El mapa es una cuadrícula de **32×32 casillas** (ampliable). Cada casilla tiene varias **capas** que se acumulan: la base define si es suelo, pared o puerta; encima van decoraciones, trampas, enemigos o llaves. Todo esto se edita desde dentro del propio juego pulsando **F1**.

El juego arranca con fondo negro. Las zonas no exploradas son completamente negras — sin bordes ni colores de fondo. A medida que el jugador avanza, la mazmorra se revela habitación a habitación (niebla de guerra).

---

## 2. Cómo abrir el editor

1. Lanza el juego normalmente (ejecuta el .exe o desde Visual Studio).
2. Pulsa **F1** — el editor se activa. Verás:
   - Un **cursor de color** sobre el mapa que puedes mover con las flechas.
   - Un **panel de paleta** en el lado derecho con los tipos disponibles.
   - Todo el mapa aparece visible (la niebla se retira para que puedas editar).
3. Pulsa **F1** de nuevo para salir. **El mapa se guarda automáticamente al salir.**

> El jugador queda congelado mientras el editor está activo.

---

## 3. Controles del editor

| Tecla / Acción | Resultado |
|---|---|
| **F1** | Activar / desactivar editor (guarda al salir) |
| **Enter** | Guardar sin salir del editor |
| **Flechas** | Mover el cursor casilla a casilla (atraviesa paredes) |
| **D** | Siguiente capa |
| **A** | Capa anterior |
| **S** | Siguiente tipo dentro de la capa actual |
| **W** | Tipo anterior dentro de la capa actual |
| **Espacio** | Pintar el tipo seleccionado en la casilla del cursor |
| **P** | Marcar la casilla del cursor como punto de inicio del jugador |
| **Clic izquierdo** | Pintar la casilla bajo el ratón |
| **Clic derecho** | Borrar la casilla bajo el ratón |
| **Q** | Aumentar zoom (+0.1×) |
| **E** | Reducir zoom (−0.1×) |

> El cursor **atraviesa paredes** — puedes colocarlo en cualquier casilla sin restricciones.

---

## 4. Las capas del mapa — qué es cada una y cuándo usarla

El mapa tiene **6 capas** independientes que se apilan. Cambia de capa con **A / D**.
El **color del banner** (franja superior del panel) y el **color del cursor** indican en qué capa estás.

| # | Color banner | Capa | Qué contiene | ¿Afecta al juego? |
|---|---|---|---|---|
| 0 | Azul | **BASE** | Estructura: suelo, pared, puerta | Sí — colisiones y navegación |
| 1 | Morado | **PROPS** | Decoración visual (luces, manchas…) | Solo visual |
| 2 | Rojo | **TRAMPAS** | Trampas ocultas en el suelo | Sí — daño al pisarlas |
| 3 | Verde | **ENEMIGOS** | Puntos donde aparecerán enemigos | Sí — spawn de enemigos |
| 4 | Dorado | **LLAVES** | Llaves que el jugador puede recoger | Sí — abre puertas cerradas |
| 5 | Gris plateado | **VARIANTES** | Orientación visual de pared/puerta | Solo visual |

**Regla importante:** las capas se apilan. Puedes tener en la misma casilla, por ejemplo, suelo (BASE) + una antorcha (PROPS) + una trampa (TRAMPAS). Editar una capa no borra las otras.

---

## 5. Guía de props — tipos disponibles y reglas de colocación

Los **props** son decoraciones visuales que no afectan al movimiento. Hay dos tipos según dónde se colocan:

### Props de PARED — se colocan sobre BASE_PARED

| Color en paleta | Prop | Aspecto |
|---|---|---|
| Gris muy oscuro | **Nada** (borrar) | Sin decoración |
| Cian | **Lámpara** | Luz colgante en la pared |
| Naranja | **Antorcha** | Antorcha de pared |

> Los props de pared se renderizan centrados en la parte superior de la casilla, pegados a la pared.

### Props de SUELO — se colocan sobre BASE_SUELO

| Color en paleta | Prop | Aspecto |
|---|---|---|
| Azul oscuro | **Charco** | Charco de agua oscura |
| Marrón oscuro | **Mancha** | Mancha (sangre u otra sustancia) |
| Blanco hueso | **Huesos** | Restos óseos en el suelo |

> Los props de suelo se renderizan centrados dentro de la casilla.

### Reglas de colocación

| Situación | Resultado |
|---|---|
| Prop de PARED sobre BASE_SUELO | Rechazado — mensaje en consola: *"EDITOR: prop de pared requiere BASE_PARED"* |
| Prop de SUELO sobre BASE_PARED | Rechazado — mensaje en consola: *"EDITOR: prop de suelo requiere BASE_SUELO"* |
| Clic derecho en capa PROPS | Borra cualquier prop del tile sin validación |

**Flujo recomendado:**
1. Diseña la estructura primero en capa BASE.
2. Luego ve a capa PROPS y decora.
3. Si ves que el prop no aparece, comprueba que estás sobre el tipo de base correcto.

---

## 6. Sistema de variantes de pared y puerta

### ¿Qué es una variante?

Cada casilla de pared o puerta tiene una **variante visual** que indica su orientación. Esto permite que el artista cree sprites distintos para una pared norte, sur, este u oeste, y puertas horizontales o verticales. Con los placeholders de color, las variantes se distinguen por tonalidades.

### Variantes disponibles

| Variante | Descripción | Color placeholder |
|---|---|---|
| DEFAULT | Sin orientación definida | Gris medio |
| NORTE | Cara visible mira al norte (suelo al sur) | Gris azulado |
| SUR | Cara visible mira al sur (suelo al norte) | Gris rojizo |
| ESTE | Cara visible mira al este (suelo al este) | Gris verdoso |
| OESTE | Cara visible mira al oeste (suelo al oeste) | Gris amarillento |
| PUERTA_H | Puerta horizontal (jugador la cruza yendo izquierda/derecha) | Marrón claro |
| PUERTA_V | Puerta vertical (jugador la cruza yendo arriba/abajo) | Marrón oscuro |

### Autodetección — el flujo normal

**Al pintar cualquier pared o puerta en capa BASE, el editor detecta la variante automáticamente** analizando los vecinos del tile. El diseñador no tiene que hacer nada extra.

Reglas de autodetección:
- Si hay suelo al norte → variante SUR (la cara visible mira al sur)
- Si hay suelo al sur → variante NORTE
- Si hay suelo al este → variante ESTE
- Si hay suelo al oeste → variante OESTE
- Si la puerta tiene paso al este u oeste → PUERTA_H
- Si la puerta tiene paso al norte o sur → PUERTA_V

### Sobreescritura manual

Si la autodetección no da el resultado deseado, ve a capa **VARIANTES** (A/D hasta el banner gris plateado) y:
- **W/S** para ciclar entre las 7 variantes.
- **Espacio** o **clic izquierdo** para aplicar.
- **Clic derecho** para resetear a DEFAULT.

> La capa VARIANTES solo funciona sobre casillas de pared o puerta. En suelo, el editor muestra un mensaje en consola y no hace nada.

---

## 7. El sistema de fog of war — cómo diseñar niveles que funcionen bien

### Concepto básico

El juego usa **niebla de guerra por zonas**, no por radio. Cuando el jugador entra en una habitación, toda esa habitación se revela al instante, incluyendo sus paredes y puertas. Las zonas aún no visitadas son negro absoluto — no hay gris ni estado "recordado".

### Reglas de diseño obligatorias

| Regla | Por qué |
|---|---|
| Cada habitación debe estar **completamente cerrada** por paredes | Sin pared de cierre, la niebla se expande más allá de la habitación |
| Las habitaciones se conectan **solo a través de puertas** | Sin puerta, las dos habitaciones se revelan juntas como si fueran una sola |
| Las puertas deben reemplazar un tile de pared (no estar en suelo abierto) | La puerta actúa como barrera de zona |
| El spawn del jugador debe estar sobre suelo dentro de una habitación cerrada | Si está en suelo abierto, la revelación inicial puede ser demasiado grande |

### Ejemplo de habitación correcta

```
P P P P P P       P = Pared
P . . . . P       . = Suelo
P . . . D P       D = Puerta
P . . . . P
P P P P P P
```

El BFS revela todo el interior (suelo + paredes circundantes) cuando el jugador entra, y se detiene en la puerta D. Lo que hay al otro lado de D permanece oculto hasta que el jugador cruce.

### Comportamiento en el editor

Al abrir el editor (F1), el mapa se revela completamente para que puedas ver y editar todos los tiles. Al cerrar el editor, la niebla vuelve al estado que tenía el jugador.

---

## 8. Sistema de llaves y puertas — cómo crear puzles

### Cómo funciona en partida

| Situación | Resultado |
|---|---|
| El jugador pisa una casilla con llave | La llave desaparece del mapa y se añade al inventario |
| El jugador intenta cruzar una puerta CERRADA con al menos 1 llave | La puerta se abre, se consume 1 llave, el jugador entra y se revela la nueva zona |
| El jugador intenta cruzar una puerta CERRADA sin llaves | El movimiento se cancela silenciosamente |
| El jugador cruza una puerta ABIERTA | Movimiento normal, sin coste |

El inventario de llaves no tiene límite. Se pueden acumular varias llaves antes de usarlas.

### Cómo diseñar un puzle solucionable

**Regla básica:** la llave debe estar en una zona accesible ANTES de la puerta que va a abrir.

**Ejemplo de puzle correcto:**
```
[HAB. INICIO] --(puerta abierta)--> [HAB. LLAVE] --(puerta cerrada)--> [HAB. OBJETIVO]
```
El jugador recoge la llave en la habitación intermedia y puede abrir la puerta que da al objetivo.

**Patrón del mapa de prueba incluido:**
```
[HAB. A — spawn] --(abierta)--> [HAB. B — enemigo] --(cerrada)--> [HAB. C — llave]
```
En el mapa de prueba, la llave está en la habitación C (detrás de la puerta cerrada). Esto crea un puzle circular — hay que rediseñar las habitaciones si se quiere que el puzle sea resoluble.

### Reglas del editor para llaves y puertas

- Las llaves se colocan en capa **LLAVES** (banner dorado) sobre tiles de suelo.
- Las puertas cerradas se colocan en capa **BASE** seleccionando tipo "Puerta cerrada".
- Por cada puerta cerrada que el jugador deba abrir, pon al menos una llave accesible antes.

### Acceso al contador (para el HUD)

```cpp
int llavesActuales = jugador->getLlaves();
```

---

## 9. Guardar y cargar el mapa

El mapa se guarda siempre en:
```
DungeonJuego/mapa.txt
```

| Cuándo se guarda | Cómo |
|---|---|
| Al salir del editor (F1) | Automático |
| En cualquier momento dentro del editor | Pulsa **Enter** |

Al arrancar el juego:
- Si existe `mapa.txt` → se carga automáticamente.
- Si no existe → se genera el mapa por defecto del código.

El archivo es texto plano. Puedes abrirlo con el Bloc de Notas para inspeccionarlo.

**Para volver al mapa por defecto:** borra o renombra `mapa.txt` antes de lanzar el juego.

**Versiones del formato:**
- `DUNGEON_MAP 1` — formato anterior (sin variantes). El juego lo carga y auto-detecta variantes.
- `DUNGEON_MAP 2` — formato actual (incluye capa VARIANTES). El editor siempre guarda en v2.

---

## 10. Guía para artistas — cómo integrar tus PNGs sin tocar código

### Dónde dejar los archivos

Deja tus PNGs en las carpetas que ya existen dentro de `DungeonJuego/assets/textures/`:

```
DungeonJuego/assets/textures/
├── characters/
│   └── hero/
│       └── hero_idle.png        ← YA EXISTE — no tocar
├── tiles/
│   ├── base/
│   │   ├── suelo.png            ← pendiente artistas
│   │   ├── pared_default.png    ✅ ENTREGADO — pared sin orientación
│   │   ├── pared_norte.png      ✅ ENTREGADO
│   │   ├── pared_sur.png        ✅ ENTREGADO
│   │   ├── pared_este.png       ✅ ENTREGADO (se usa también para pared oeste, espejada)
│   │   └── pared_oeste.png      ← no necesario; el motor genera el espejo de pared_este.png
│   │   └── puerta/
│   │       ├── puerta_h_cerrada.png
│   │       ├── puerta_h_abierta.png
│   │       ├── puerta_v_cerrada.png
│   │       └── puerta_v_abierta.png
│   └── props/
│       ├── pared/
│       │   ├── lampara.png      ← 16×24 px
│       │   └── antorcha.png     ← 16×24 px
│       └── suelo/
│           ├── charco.png       ← 48×48 px
│           ├── mancha.png       ← 48×48 px
│           └── huesos.png       ← 48×48 px
```

### Tamaños requeridos

| Tipo de asset | Tamaño del PNG | Notas |
|---|---|---|
| Tiles base (suelo, pared, puerta) | **64×64 px** | Fondo transparente si no ocupa todo el tile |
| Props de pared (lámpara, antorcha) | **16×24 px** | Se pegará a la parte superior del tile |
| Props de suelo (charco, mancha, huesos) | **48×48 px** | Se centrará en el tile |
| Sprite del héroe | **32×32 px por frame** | Spritesheet horizontal si hay animación |
| Enemigos | **32×32 px por frame** | Spritesheet horizontal si hay animación |
| Llaves (objeto en el suelo) | **Libre, máx 32×32 px** | Se centra en la mitad superior del tile |
| Trampas (visibles solo en editor) | **Libre, máx 64×64 px** | Solo visible en editor, oculto en juego |

### Qué NO tocar

- `hero_idle.png` — el sprite del personaje ya está integrado.
- Ningún archivo `.cpp` ni `.h` — eso es trabajo del programador.
- El archivo `mapa.txt` — se genera automáticamente al guardar.

### Qué hace el programador cuando recibe los PNGs

1. Busca `TODO_ARTISTA` en `UNIR-2D/Mapa.cpp` — hay un marcador por cada tipo de tile.
2. Sustituye el `ponColor(...)` del placeholder por la carga de la textura:
   ```cpp
   textura->carga("assets/textures/tiles/base/suelo.png");
   imagen->asigna(textura);
   imagen->ponPosicion(Vector(tx, ty));
   ```
3. Prueba en el juego y ajusta si hace falta.

Para encontrar todos los puntos de integración de una vez:
```
grep TODO_ARTISTA UNIR-2D/Mapa.cpp
```

> **Nota sobre los PNGs de pared:** los archivos `pared_sur.png`, `pared_norte.png`, `pared_este.png` y `pared_default.png` ya están integrados. La pared oeste se genera automáticamente espejando `pared_este.png`, por lo que `pared_oeste.png` no es necesario. Si cambias un PNG, reinicia el juego para ver los cambios. Si el nombre es incorrecto, el juego carga en modo placeholder de colores automáticamente.

---

## 11. Referencia rápida de colores placeholder

### Capa BASE

| Tipo | Color en el editor | Descripción |
|---|---|---|
| Suelo | Gris medio | Zona pisable |
| Pared DEFAULT | Gris medio oscuro | Pared sin orientación |
| Pared NORTE | Gris azulado | Cara visible al norte |
| Pared SUR | Gris rojizo | Cara visible al sur |
| Pared ESTE | Gris verdoso | Cara visible al este |
| Pared OESTE | Gris amarillento | Cara visible al oeste |
| Puerta cerrada H | Marrón claro | Puerta horizontal cerrada |
| Puerta cerrada V | Marrón oscuro | Puerta vertical cerrada |
| Puerta abierta H | Marrón muy claro | Puerta horizontal abierta |
| Puerta abierta V | Marrón claro medio | Puerta vertical abierta |

### Props de pared (capa PROPS, sobre BASE_PARED)

| Prop | Color | Descripción |
|---|---|---|
| Lámpara | Cian (amarillo brillante) | Luz colgante |
| Antorcha | Naranja | Antorcha de pared |

### Props de suelo (capa PROPS, sobre BASE_SUELO)

| Prop | Color | Descripción |
|---|---|---|
| Charco | Azul oscuro semitransparente | Charco de agua |
| Mancha | Marrón oscuro | Mancha orgánica |
| Huesos | Blanco hueso | Restos óseos |

### Otras capas

| Capa | Tipo | Color |
|---|---|---|
| TRAMPAS | Trampa fija | Rojo vivo (solo visible en editor) |
| ENEMIGOS | Spawn enemigo | Verde vivo |
| LLAVES | Llave normal | Amarillo dorado |

---

## 12. Errores frecuentes y cómo resolverlos

### "El prop no aparece en el mapa"

**Causa más probable:** prop de pared colocado sobre suelo, o prop de suelo sobre pared.

**Solución:**
1. Ve a capa BASE y comprueba qué tipo tiene la casilla.
2. Vuelve a capa PROPS y elige el prop correcto para ese tipo de base.
3. Si ves el mensaje *"EDITOR: prop de pared requiere BASE_PARED"* en consola, ese es el problema.

---

### "La habitación entera aparece visible desde el inicio"

**Causa:** no hay puerta entre dos habitaciones. El BFS las trata como una zona única y las revela a la vez.

**Solución:**
1. Ve a capa BASE y coloca una puerta (abierta o cerrada) entre las dos zonas.
2. La puerta debe reemplazar un tile de pared, no estar flotando en el suelo.

---

### "La puerta no se abre al intentar cruzarla"

**Causa:** el jugador no tiene llaves en el inventario.

**Solución:**
1. Comprueba que hay una llave (capa LLAVES, color dorado) accesible antes de la puerta cerrada.
2. Verifica que la llave no está detrás de la misma puerta que bloquea.

---

### "El mapa no se guarda / los cambios se pierden al cerrar"

**Solución:**
- Pulsa **Enter** dentro del editor para guardar manualmente.
- O sal del editor con **F1** — guarda automáticamente al salir.
- Verifica que tienes permisos de escritura en la carpeta `DungeonJuego/`.

---

### "Las variantes de pared no cambian de color visualmente"

**Aclaración:** con los placeholders de color, las diferencias entre variantes son sutiles (grises ligeramente distintos). La diferencia real se verá cuando los artistas entreguen los PNGs de `pared_norte.png`, `pared_sur.png`, etc.

Para verificar que la variante se está aplicando:
1. Entra en capa VARIANTES (A/D hasta el banner gris plateado).
2. Mueve el cursor sobre una pared — el indicador amarillo del panel mostrará qué tipo está seleccionado.

---

### "En el editor (F1) las paredes muestran colores en lugar de PNGs"

**Aclaración:** esto es **intencionado**. El editor usa los colores placeholder para que las variantes de pared (norte/sur/este/oeste) sean visualmente distinguibles durante la edición. Al salir del editor (F1) se muestran los PNGs reales.

---

### "Las paredes siguen siendo rectángulos de color en lugar de mostrar los PNGs"

**Causa:** el juego no encontró los archivos PNG o alguno tiene nombre incorrecto.

**Solución:**
1. Verificar que los 5 archivos están en `DungeonJuego/assets/textures/tiles/base/` con estos nombres exactos:
   - `pared_sur.png` · `pared_norte.png` · `pared_este.png` · `pared_oeste.png` · `pared_default.png`
2. Comprobar la consola al arrancar: si aparece `MAPA: no se pudieron cargar texturas de pared, usando placeholders` indica qué falló.
3. Los nombres son sensibles a mayúsculas en Linux; en Windows no, pero mejor mantenerlos en minúsculas.

---

### "El jugador aparece en una posición incorrecta al iniciar"

**Solución:**
1. Abre el editor (F1).
2. Mueve el cursor con las flechas hasta la casilla de suelo donde quieres el spawn.
3. Pulsa **P** — el marcador cian (cuadrado semitransparente) salta a esa posición.
4. Guarda con **Enter** o sal con **F1**.

> El spawn debe estar sobre un tile de **suelo**. En pared o puerta, el jugador quedará atascado.

---

## 13. El HUD — paneles y consola de mensajes

El HUD es una capa de interfaz fija que se dibuja **siempre en las mismas posiciones de pantalla**, independientemente del zoom y la cámara. No forma parte del mapa; no se puede editar con el editor.

> ⚠️ **El HUD está parcialmente implementado.** Los sistemas de texto y paneles funcionan. La vida siempre vale 100 hasta que se implemente el sistema de combate. Los iconos de retrato y llave muestran placeholders de color hasta que arte entregue los PNGs.
>
> El HUD **se oculta automáticamente** al entrar en el editor (F1) y **vuelve a aparecer** al cerrarlo.

---

### Panel superior izquierda — estadísticas del jugador

- **Posición:** esquina superior izquierda (x=10, y=10 px)
- **Tamaño:** 210×80 px — negro semitransparente
- **Layout de izquierda a derecha / arriba a abajo:**

| Elemento | Posición pantalla | Tamaño | Estado |
|---|---|---|---|
| Retrato personaje | x=18, y=18 | 48×48 px | ⚠️ Placeholder morado hasta que arte entregue `retrato_heroe.png` |
| Vida | x=74, y=22 | 12 px | ✅ Muestra `100/100` — falta sistema de daño |
| Icono llave | x=74, y=42 | 24×24 px | ⚠️ Placeholder amarillo hasta que arte entregue `llave_icon.png` |
| Nº llaves | x=102, y=48 | 12 px | ✅ Se actualiza al recoger/gastar llaves |

---

### Panel inferior izquierda — consola de mensajes

- **Posición:** esquina inferior izquierda (x=10, y≈550 px)
- **Tamaño:** 500×160 px — azul oscuro semitransparente
- **Fuente:** `PressStart2P-Regular.ttf` a 8 px (ya incluida en `assets/fonts/`)
- **Capacidad:** hasta 8 líneas visibles; los mensajes largos hacen **salto de línea automático**
- Los mensajes aparecen **desde arriba** (el primero en la línea superior)
- Cada mensaje dura **4 segundos** por defecto y se desvanece en el último segundo
- Cola interna de 5 mensajes; si llegan más, el más antiguo se descarta

**Mensajes automáticos del juego:**

| Situación | Mensaje | Duración |
|---|---|---|
| Jugador entra al juego | *"Bienvenido al dungeon. Encuentra las llaves."* | 10 s |
| Jugador recoge una llave | *"Llave recogida"* | 4 s |
| Jugador abre una puerta | *"Puerta abierta"* | 4 s |
| Jugador intenta abrir sin llave | *"Necesitas una llave para abrir esta puerta"* | 4 s |

**Para cambiar el mensaje de bienvenida:** `DungeonJuego/Hud.cpp` → última línea de `inicia()`:
```cpp
agregarMensaje("Bienvenido al dungeon. Encuentra las llaves.", 10.0f);
//              ↑ texto                                        ↑ segundos
```

---

### Para artistas — iconos del HUD

Depositar los PNGs en `DungeonJuego/assets/textures/ui/`:

| Archivo | Tamaño **exacto** | Para qué | Placeholder actual |
|---|---|---|---|
| `retrato_heroe.png` | **48×48 px** | Retrato del personaje en el panel de stats | Rectángulo morado (80,80,120) |
| `llave_icon.png` | **24×24 px** | Icono junto al contador de llaves | Rectángulo amarillo (200,180,0) |
| `corazon.png` | 16×16 px | Icono de vida (sesión futura) | Sin implementar aún |

> Los PNGs se cargan automáticamente al arrancar el juego. Si el nombre o la ruta es incorrecta, el placeholder de color sigue apareciendo y el juego no crashea.

---

### Para artistas — fuente

La fuente `PressStart2P-Regular.ttf` **ya está incluida** en `DungeonJuego/assets/fonts/`. No hay que hacer nada.

Si se quiere usar otra fuente, nombrarla `fuente.ttf` y depositarla en esa misma carpeta. El juego intenta cargar `fuente.ttf` primero y usa `PressStart2P-Regular.ttf` como fallback.

---

### Para programadores — cómo enviar mensajes desde un actor nuevo

Para que cualquier actor del juego pueda enviar mensajes a la consola del HUD:

1. **En el `.h` del actor:** añadir forward declaration y setter:
   ```cpp
   class Hud;
   // ...
   private:
   Hud* hud = nullptr;
   public:
   void ponHud(Hud* h) { hud = h; }
   ```

2. **En el `.cpp` del actor:** incluir `"Hud.h"` y llamar cuando ocurra el evento:
   ```cpp
   #include "Hud.h"
   // ...
   if (hud) hud->agregarMensaje("texto del evento");
   if (hud) hud->agregarMensaje("texto importante", 6.0f);  // duración custom
   ```

3. **En `JuegoDungeon::inicia()`**, después de crear el actor:
   ```cpp
   miActor->ponHud(hud);
   ```

El puntero `hud` puede ser `nullptr` — el actor no falla si el HUD no está conectado.
