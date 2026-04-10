#include "UNIR-2D.h"
#include "EditorMapa.h"

using namespace unir2d;

static const float WINDOW_W       = 1280.0f;
static const float WINDOW_H       =  720.0f;
static const float PANTALLA_ALTO  =  720.0f;
static const float PANEL_X        = 1070.0f;   // posición en pantalla (píxeles)
static const float PANEL_ANCHO    =  210.0f;   // ancho en pantalla (píxeles)
static const float BANNER_H       =   34.0f;
static const float CAJA_TAM       =   54.0f;
static const float CAJA_GAP       =    8.0f;
static const float CAJA_INICIO_Y  =   50.0f;

// ============================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================

EditorMapa::EditorMapa(Mapa* mapa_) : mapa(mapa_) {}
EditorMapa::~EditorMapa() {}

// ============================================================
// CICLO DE VIDA
// ============================================================

void EditorMapa::inicia() {
    float ts = mapa->tamanoCasilla();

    cursor = new Rectangulo(ts, ts);
    cursor->ponColor(Color(80, 255, 180, 130));
    cursor->ponIndiceZ(100);
    cursor->ponVisible(false);
    agregaDibujo(cursor);

    panelFondo = new Rectangulo(PANEL_ANCHO, PANTALLA_ALTO);
    panelFondo->ponColor(Color(18, 18, 28, 220));
    panelFondo->ponPosicion(Vector(PANEL_X, 0.0f));
    panelFondo->ponIndiceZ(50);
    panelFondo->ponVisible(false);
    agregaDibujo(panelFondo);

    bannerCapa = new Rectangulo(PANEL_ANCHO, BANNER_H);
    bannerCapa->ponPosicion(Vector(PANEL_X, 0.0f));
    bannerCapa->ponIndiceZ(51);
    bannerCapa->ponVisible(false);
    agregaDibujo(bannerCapa);

    for (int i = 0; i < MAX_TIPOS; i++) {
        cajasTipo[i] = new Rectangulo(CAJA_TAM, CAJA_TAM);
        float y = CAJA_INICIO_Y + i * (CAJA_TAM + CAJA_GAP);
        cajasTipo[i]->ponPosicion(Vector(0.0f, y));   // posición provisional; se fija en actualizarVisual
        cajasTipo[i]->ponColor(Color(40, 40, 40));
        cajasTipo[i]->ponIndiceZ(52);
        cajasTipo[i]->ponVisible(false);
        agregaDibujo(cajasTipo[i]);
    }

    selIndicador = new Rectangulo(CAJA_TAM + 8.0f, CAJA_TAM + 8.0f);
    selIndicador->ponColor(Color(255, 235, 80, 220));
    selIndicador->ponIndiceZ(51);
    selIndicador->ponVisible(false);
    agregaDibujo(selIndicador);

    float msz = ts * 0.5f;
    spawnMarker = new Rectangulo(msz, msz);
    spawnMarker->ponColor(Color(0, 230, 255, 180));
    spawnMarker->ponIndiceZ(3);
    spawnMarker->ponVisible(true);
    agregaDibujo(spawnMarker);
    actualizarSpawnMarker();
}

void EditorMapa::termina() {
    extraeDibujos();
}

// ============================================================
// CENTRO MUNDO
// ============================================================

Vector EditorMapa::centroMundo() const {
    float ts = mapa->tamanoCasilla();
    return Vector(colActual * ts + ts * 0.5f, filaActual * ts + ts * 0.5f);
}

// ============================================================
// ACTUALIZACIÓN
// ============================================================

void EditorMapa::actualiza(double /*tiempo_seg*/) {
    // F1 → toggle editor, auto-guarda al salir
    bool f1Ahora = Teclado::pulsando(Tecla::F1);
    if (f1Ahora && !f1Prev) {
        activo = !activo;
        if (activo)  mapa->revelarTodo();    // el editor muestra el mapa completo
        if (!activo) mapa->guardarMapa();
    }
    f1Prev = f1Ahora;

    // Enter → guardar sin salir
    bool enterAhora = Teclado::pulsando(Tecla::entrar);
    if (enterAhora && !enterPrev && activo) mapa->guardarMapa();
    enterPrev = enterAhora;

    actualizarSpawnMarker();

    mostrarUI(activo);
    if (!activo) return;

    manejarInput();
    actualizarVisual();
}

// ============================================================
// INPUT
// ============================================================

void EditorMapa::manejarInput() {
    int maxF = mapa->numFilas()    - 1;
    int maxC = mapa->numColumnas() - 1;

    // ---- Flechas → mover cursor ----
    bool arrAhora = Teclado::pulsando(Tecla::arriba);
    bool abaAhora = Teclado::pulsando(Tecla::abajo);
    bool izqAhora = Teclado::pulsando(Tecla::izquierda);
    bool derAhora = Teclado::pulsando(Tecla::derecha);

    if (arrAhora && !arrPrev)  { filaActual--; if (filaActual < 0)    filaActual = 0; }
    if (abaAhora && !abaoPrev) { filaActual++; if (filaActual > maxF) filaActual = maxF; }
    if (izqAhora && !izqPrev)  { colActual--;  if (colActual  < 0)    colActual  = 0; }
    if (derAhora && !derPrev)  { colActual++;  if (colActual  > maxC) colActual  = maxC; }

    arrPrev  = arrAhora;
    abaoPrev = abaAhora;
    izqPrev  = izqAhora;
    derPrev  = derAhora;

    // ---- W / S → ciclar tipo ----
    bool wAhora = Teclado::pulsando(Tecla::W);
    bool sAhora = Teclado::pulsando(Tecla::S);
    if (wAhora && !wPrev) { tipoActual--; if (tipoActual < 0)           tipoActual = maxTipos() - 1; }
    if (sAhora && !sPrev) { tipoActual++; if (tipoActual >= maxTipos()) tipoActual = 0; }
    wPrev = wAhora;
    sPrev = sAhora;

    // ---- A / D → capa anterior / siguiente ----
    bool aAhora = Teclado::pulsando(Tecla::A);
    bool dAhora = Teclado::pulsando(Tecla::D);
    if (aAhora && !aPrev) {
        capaActual--;
        if (capaActual < 0) capaActual = NUM_CAPAS - 1;
        tipoActual = 1;
    }
    if (dAhora && !dPrev) {
        capaActual++;
        if (capaActual >= NUM_CAPAS) capaActual = 0;
        tipoActual = 1;
    }
    aPrev = aAhora;
    dPrev = dAhora;

    // ---- Espacio → pintar en el cursor ----
    bool spaceAhora = Teclado::pulsando(Tecla::espacio);
    if (spaceAhora && !spacePrev) pintarEnCursor();
    spacePrev = spaceAhora;

    // ---- P → marcar spawn del jugador en el cursor ----
    bool pAhora = Teclado::pulsando(Tecla::P);
    if (pAhora && !pPrev) mapa->setSpawn(filaActual, colActual);
    pPrev = pAhora;

    // ---- Ratón → pintar / borrar ----
    if (Raton::pulsando(BotonRaton::izquierda)) {
        int f = 0, c = 0;
        if (screenToTile(Raton::posicion(), f, c)) pintarTile(f, c, tipoActual);
    }
    if (Raton::pulsando(BotonRaton::derecha)) {
        int f = 0, c = 0;
        if (screenToTile(Raton::posicion(), f, c)) borrarTile(f, c);
    }
}

// ============================================================
// VISUAL
// ============================================================

void EditorMapa::actualizarVisual() {
    float ts = mapa->tamanoCasilla();

    // Cursor: posición en mundo y color según capa (en espacio mundo, no toca zoom)
    float cx = colActual  * ts + offsetCamara.x();
    float cy = filaActual * ts + offsetCamara.y();
    cursor->ponPosicion(Vector(cx, cy));
    cursor->ponColor(colorCursor(capaActual));
    cursor->ponVisible(true);

    // El panel UI está diseñado en coordenadas de pantalla (píxeles).
    // Con zoom ≠ 1, hay que convertir esas posiciones a coordenadas mundo:
    //   mundo_x = (pantalla_x - WIN_CX) / zoom + WIN_CX
    // El motor usa la vista por defecto (centro en WIN_CX, WIN_CY) y aplica
    // view.zoom(1/zoom), por lo que esta fórmula es siempre correcta.
    float z   = zoomActual;
    float wCX = WINDOW_W * 0.5f;   // 640
    float wCY = WINDOW_H * 0.5f;   // 360

    // Panel fondo
    float pWX = (PANEL_X - wCX) / z + wCX;
    float pWY = (0.0f    - wCY) / z + wCY;
    panelFondo->ponPosicion(Vector(pWX, pWY));
    panelFondo->ponBase  (PANEL_ANCHO   / z);
    panelFondo->ponAltura(PANTALLA_ALTO / z);

    // Banner de capa
    bannerCapa->ponPosicion(Vector(pWX, pWY));
    bannerCapa->ponBase  (PANEL_ANCHO / z);
    bannerCapa->ponAltura(BANNER_H    / z);
    bannerCapa->ponColor(colorBanner(capaActual));

    // Cajas de paleta (centradas horizontalmente en el panel)
    float cajaSX  = PANEL_X + (PANEL_ANCHO - CAJA_TAM) * 0.5f;   // X en pantalla
    float cajaWX  = (cajaSX - wCX) / z + wCX;
    float cajaDim = CAJA_TAM / z;
    int n = maxTipos();
    for (int i = 0; i < MAX_TIPOS; i++) {
        if (i < n) {
            float cajaSY = CAJA_INICIO_Y + i * (CAJA_TAM + CAJA_GAP);
            float cajaWY = (cajaSY - wCY) / z + wCY;
            cajasTipo[i]->ponPosicion(Vector(cajaWX, cajaWY));
            cajasTipo[i]->ponBase  (cajaDim);
            cajasTipo[i]->ponAltura(cajaDim);
            cajasTipo[i]->ponColor(colorTipoPaleta(capaActual, i));
            cajasTipo[i]->ponVisible(true);
        } else {
            cajasTipo[i]->ponVisible(false);
        }
    }

    // Indicador de selección (borde amarillo alrededor de la caja activa)
    float indicSX  = cajaSX - 4.0f;
    float indicSY  = CAJA_INICIO_Y + tipoActual * (CAJA_TAM + CAJA_GAP) - 4.0f;
    float indicDim = (CAJA_TAM + 8.0f) / z;
    selIndicador->ponPosicion(Vector((indicSX - wCX) / z + wCX,
                                     (indicSY - wCY) / z + wCY));
    selIndicador->ponBase  (indicDim);
    selIndicador->ponAltura(indicDim);
    selIndicador->ponVisible(true);
}

// ============================================================
// HELPERS
// ============================================================

void EditorMapa::pintarEnCursor() {
    pintarTile(filaActual, colActual, tipoActual);
}

void EditorMapa::pintarTile(int fila, int col, int tipo) {
    switch (capaActual) {
        case 0: mapa->setBase   (fila, col, tipo); break;
        case 1: mapa->setProp   (fila, col, tipo); break;
        case 2: mapa->setTrampa (fila, col, tipo); break;
        case 3: mapa->setEnemigo(fila, col, tipo); break;
        case 4: mapa->setLlave  (fila, col, tipo); break;
    }
}

void EditorMapa::borrarTile(int fila, int col) {
    // Tipo 0 es siempre "nada" en todas las capas
    pintarTile(fila, col, 0);
}

void EditorMapa::mostrarUI(bool valor) {
    if (!valor) cursor->ponVisible(false);
    panelFondo  ->ponVisible(valor);
    bannerCapa  ->ponVisible(valor);
    selIndicador->ponVisible(valor);
    for (int i = 0; i < MAX_TIPOS; i++) cajasTipo[i]->ponVisible(valor);
}

bool EditorMapa::screenToTile(Vector screenPos, int& fila, int& col) const {
    // screenPos está en píxeles de pantalla (cliente de la ventana)
    if (screenPos.x() >= PANEL_X) return false;
    float ts = mapa->tamanoCasilla();
    // Convertir píxeles de pantalla → coordenadas mundo
    // (la vista SFML tiene centro fijo en WIN_CX, WIN_CY y escala 1/zoom)
    float wx = (screenPos.x() - WINDOW_W * 0.5f) / zoomActual + WINDOW_W * 0.5f;
    float wy = (screenPos.y() - WINDOW_H * 0.5f) / zoomActual + WINDOW_H * 0.5f;
    // Restar offset de cámara para obtener la posición local en el mapa
    float tileX = wx - offsetCamara.x();
    float tileY = wy - offsetCamara.y();
    if (tileX < 0.0f || tileY < 0.0f) return false;
    col  = static_cast<int>(tileX / ts);
    fila = static_cast<int>(tileY / ts);
    if (fila < 0 || fila >= mapa->numFilas())    return false;
    if (col  < 0 || col  >= mapa->numColumnas()) return false;
    return true;
}

int EditorMapa::maxTipos() const {
    switch (capaActual) {
        case 0: return 4;   // suelo, pared, puerta cerrada, puerta abierta
        case 1: return 3;   // nada, lámpara, antorcha
        case 2: return 2;   // nada, trampa fija
        case 3: return 2;   // nada, spawn enemigo
        case 4: return 2;   // nada, llave
    }
    return 1;
}

Color EditorMapa::colorBanner(int capa) const {
    switch (capa) {
        case 0: return Color( 50,  90, 210);   // azul     — BASE
        case 1: return Color(110,  40, 170);   // morado   — PROPS
        case 2: return Color(180,  30,  30);   // rojo     — TRAMPAS
        case 3: return Color( 30, 155,  60);   // verde    — ENEMIGOS
        case 4: return Color(170, 130,   0);   // dorado   — LLAVES
    }
    return Color(80, 80, 80);
}

Color EditorMapa::colorCursor(int capa) const {
    switch (capa) {
        case 0: return Color( 80, 180, 255, 130);   // azul claro
        case 1: return Color(200,  80, 255, 130);   // violeta
        case 2: return Color(255,  60,  60, 130);   // rojo
        case 3: return Color( 60, 230,  80, 130);   // verde
        case 4: return Color(255, 220,   0, 130);   // amarillo
    }
    return Color(255, 255, 255, 100);
}

void EditorMapa::actualizarSpawnMarker() {
    float ts     = mapa->tamanoCasilla();
    float msz    = ts * 0.5f;
    float offset = ts * 0.25f;   // centra msz dentro del tile
    float mx = mapa->getSpawnCol()  * ts + offsetCamara.x() + offset;
    float my = mapa->getSpawnFila() * ts + offsetCamara.y() + offset;
    spawnMarker->ponPosicion(Vector(mx, my));
}

Color EditorMapa::colorTipoPaleta(int capa, int tipo) const {
    switch (capa) {
        case 0:  // BASE
            switch (tipo) {
                case 0: return Color( 90,  90,  90);   // suelo
                case 1: return Color( 40,  40,  40);   // pared
                case 2: return Color(140,  80,  20);   // puerta cerrada
                case 3: return Color(200, 150,  70);   // puerta abierta
            }
            break;
        case 1:  // PROPS
            switch (tipo) {
                case 0: return Color( 25,  25,  25);   // nada
                case 1: return Color(  0, 210, 210);   // lámpara  (cian)
                case 2: return Color(255, 110,   0);   // antorcha (naranja)
            }
            break;
        case 2:  // TRAMPAS
            switch (tipo) {
                case 0: return Color( 25,  25,  25);   // nada
                case 1: return Color(220,  25,  25);   // trampa fija (rojo vivo)
            }
            break;
        case 3:  // ENEMIGOS
            switch (tipo) {
                case 0: return Color( 25,  25,  25);   // nada
                case 1: return Color( 30, 200,  60);   // spawn enemigo (verde vivo)
            }
            break;
        case 4:  // LLAVES
            switch (tipo) {
                case 0: return Color( 25,  25,  25);   // nada
                case 1: return Color(240, 210,   0);   // llave (amarillo dorado)
            }
            break;
    }
    return Color(255, 0, 255);
}
