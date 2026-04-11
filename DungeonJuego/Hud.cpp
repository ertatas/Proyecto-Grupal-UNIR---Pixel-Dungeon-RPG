#include "UNIR-2D.h"
#include "Hud.h"
#include <iostream>
#include <algorithm>

using namespace unir2d;

// ── Resolución de ventana (debe coincidir con JuegoDungeon.cpp) ──────────────
const float Hud::WINDOW_W = 1280.0f;
const float Hud::WINDOW_H =  720.0f;

// ── Geometría de los paneles (en píxeles de pantalla) ────────────────────────
static const float STATS_X    =  10.0f;
static const float STATS_Y    =  10.0f;
static const float STATS_W    = 200.0f;
static const float STATS_H    =  90.0f;

static const float CONSOLA_X  =  10.0f;
static const float CONSOLA_W  = 320.0f;
static const float CONSOLA_H  = 160.0f;
// CONSOLA_Y = WINDOW_H - 170.0f  (calculado en actualizarVisual)

static const float MSG_PADDING   =   8.0f;   // padding interno de la consola
static const float MSG_LINE_H    =  16.0f;   // separación vertical entre mensajes
static const float MSG_FONT_SIZE =  12.0f;   // tamaño de fuente en px de pantalla

// ============================================================
// CONSTRUCTOR
// ============================================================

Hud::Hud(Jugador* jugador_) : jugador(jugador_) {}

// ============================================================
// INICIA
// ============================================================

void Hud::inicia() {
    // --- Panel stats (superior izquierda) -------------------------
    // TODO_HUD_STATS: aquí irán retrato del jugador, vida y contador de llaves.
    // Para añadir texto: usar unir2d::Texto (disponible en el motor).
    // Para añadir icono de llave: cargar PNG desde assets/textures/ui/llave_icon.png
    // Leer número de llaves con: jugador->getLlaves()
    panelStats = new Rectangulo(1.0f, 1.0f);
    panelStats->ponColor(Color(0, 0, 0, 170));
    panelStats->ponIndiceZ(150);
    agregaDibujo(panelStats);

    // --- Panel consola (inferior izquierda) -----------------------
    panelConsola = new Rectangulo(1.0f, 1.0f);
    panelConsola->ponColor(Color(0, 20, 60, 150));
    panelConsola->ponIndiceZ(150);
    agregaDibujo(panelConsola);

    // --- Textos de mensajes ---------------------------------------
    // La fuente se busca en {cwd}/fuentes/DejaVuSans.ttf
    // Si no existe: degradación a solo paneles (sin texto, sin crash).
    bool textoOK = false;
    try {
        for (int i = 0; i < MAX_MENSAJES; i++) {
            textosMsg[i] = new Texto("DejaVuSans");
        }
        textoOK = true;
    } catch (...) {
        std::cerr << "[HUD] Fuente 'DejaVuSans.ttf' no encontrada en fuentes/.\n"
                  << "[HUD] Para activar mensajes: copiar UNIR-2D/fuentes/DejaVuSans.ttf"
                  << " a DungeonJuego/fuentes/\n";
        // Liberar los que se crearon (en la práctica, ninguno: la excepción
        // se lanza en i=0, antes de que se asigne textosMsg[0]).
        for (int i = 0; i < MAX_MENSAJES; i++) {
            delete textosMsg[i];
            textosMsg[i] = nullptr;
        }
    }

    if (textoOK) {
        for (int i = 0; i < MAX_MENSAJES; i++) {
            textosMsg[i]->ponColor(Color(255, 255, 255, 255));
            textosMsg[i]->ponIndiceZ(160);
            textosMsg[i]->ponVisible(false);
            agregaDibujo(textosMsg[i]);
        }
        fuenteCargada = true;
    }

    // --- Mensaje de bienvenida ------------------------------------
    // Verifica que la consola funciona desde el primer frame.
    agregarMensaje("Bienvenido al dungeon. Encuentra las llaves y explora.", 6.0f);
}

// ============================================================
// TERMINA
// ============================================================

void Hud::termina() {
    extraeDibujos();
    panelStats   = nullptr;
    panelConsola = nullptr;
    for (int i = 0; i < MAX_MENSAJES; i++) textosMsg[i] = nullptr;
}

// ============================================================
// ACTUALIZA
// ============================================================

void Hud::actualiza(double tiempo_seg) {
    // --- Calcular deltaT -----------------------------------------
    float dt = 0.0f;
    if (ultimoTiempo >= 0.0) {
        dt = static_cast<float>(tiempo_seg - ultimoTiempo);
        dt = std::min(dt, 0.1f);   // clamp: evita saltos en el primer frame o tras pausa
    }
    ultimoTiempo = tiempo_seg;

    // --- Descontar tiempo de cada mensaje; eliminar caducados ----
    for (auto it = mensajes.begin(); it != mensajes.end(); ) {
        it->tiempoRestante -= dt;
        if (it->tiempoRestante <= 0.0f) it = mensajes.erase(it);
        else ++it;
    }

    actualizarVisual();
}

// ============================================================
// AGREGAR MENSAJE
// ============================================================

void Hud::agregarMensaje(const std::string& texto, float duracion) {
    // Si la cola está llena, descartamos el mensaje más antiguo inmediatamente.
    if (static_cast<int>(mensajes.size()) >= MAX_MENSAJES) {
        mensajes.pop_front();
    }
    mensajes.push_back({ texto, duracion, duracion });
}

// ============================================================
// ACTUALIZAR VISUAL
// ============================================================

void Hud::actualizarVisual() {
    float z = zoomActual;

    // --- Panel stats (superior izquierda) ------------------------
    Vector statsWPos = screenToWorld(STATS_X, STATS_Y);
    panelStats->ponPosicion(statsWPos);
    panelStats->ponBase  (STATS_W / z);
    panelStats->ponAltura(STATS_H / z);

    // --- Panel consola (inferior izquierda) ----------------------
    float consolaY = WINDOW_H - 170.0f;
    Vector consolaWPos = screenToWorld(CONSOLA_X, consolaY);
    panelConsola->ponPosicion(consolaWPos);
    panelConsola->ponBase  (CONSOLA_W / z);
    panelConsola->ponAltura(CONSOLA_H / z);

    // --- Textos de mensajes --------------------------------------
    if (!fuenteCargada) return;

    int n = std::min(static_cast<int>(mensajes.size()), MAX_MENSAJES);
    // Tamaño de carácter en mundo = MSG_FONT_SIZE / z
    // → al renderizarse con zoom z queda como MSG_FONT_SIZE px en pantalla.
    int tamChar = std::max(1, static_cast<int>(MSG_FONT_SIZE / z));

    for (int slot = 0; slot < MAX_MENSAJES; slot++) {
        if (textosMsg[slot] == nullptr) continue;

        // Los últimos n mensajes del deque ocupan los slots finales (más abajo).
        // Slot 0 = arriba (mensaje más antiguo visible).
        // Slot MAX_MENSAJES-1 = abajo (mensaje más reciente).
        int msgIdx = static_cast<int>(mensajes.size()) - MAX_MENSAJES + slot;
        if (msgIdx < 0) {
            textosMsg[slot]->ponVisible(false);
            continue;
        }

        const MensajeConsola& msg = mensajes[static_cast<size_t>(msgIdx)];

        // Fade out: cuando queda menos de 1 segundo, el alpha baja progresivamente.
        float alpha = 255.0f;
        if (msg.tiempoRestante < 1.0f) {
            alpha = (msg.tiempoRestante / 1.0f) * 255.0f;
        }
        int a = std::max(0, std::min(255, static_cast<int>(alpha)));

        textosMsg[slot]->ponCadena(msg.texto);
        textosMsg[slot]->ponColor(Color(255, 255, 255, a));
        textosMsg[slot]->ponTamano(tamChar);

        float screenX = CONSOLA_X  + MSG_PADDING;
        float screenY = consolaY   + MSG_PADDING + slot * MSG_LINE_H;
        textosMsg[slot]->ponPosicion(screenToWorld(screenX, screenY));
        textosMsg[slot]->ponVisible(true);
    }
}

// ============================================================
// SCREEN TO WORLD
// ============================================================

Vector Hud::screenToWorld(float sx, float sy) const {
    // El motor SFML aplica view.zoom(1/z), con centro de vista en (WIN_CX, WIN_CY).
    // Para que un punto de pantalla (sx, sy) se renderice en esa posición,
    // hay que colocarlo en el mundo en:
    //   mundo_x = (sx - WIN_CX) / z + WIN_CX
    //   mundo_y = (sy - WIN_CY) / z + WIN_CY
    // (mismo cálculo que usa EditorMapa para su panel lateral)
    float wCX = WINDOW_W * 0.5f;   // 640
    float wCY = WINDOW_H * 0.5f;   // 360
    return Vector(
        (sx - wCX) / zoomActual + wCX,
        (sy - wCY) / zoomActual + wCY
    );
}
