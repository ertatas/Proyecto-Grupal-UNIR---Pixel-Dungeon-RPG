#include "UNIR-2D.h"
#include "Hud.h"
#include "Jugador.h"
#include <iostream>
#include <algorithm>
#include <filesystem>

using namespace unir2d;
namespace fs = std::filesystem;

// ── Resolución de ventana (debe coincidir con JuegoDungeon.cpp) ──────────────
const float Hud::WINDOW_W = 1280.0f;
const float Hud::WINDOW_H =  720.0f;

// ── Geometría de los paneles (en píxeles de pantalla) ────────────────────────
static const float STATS_X    =  10.0f;
static const float STATS_Y    =  10.0f;
static const float STATS_W    = 210.0f;
static const float STATS_H    =  80.0f;   // reducido: sin tercer hueco

static const float CONSOLA_X  =  10.0f;
static const float CONSOLA_W  = 500.0f;   // más ancho para caber más texto
static const float CONSOLA_H  = 160.0f;

// Posiciones de elementos en el panel stats (coordenadas de pantalla)
static const float RETRATO_X  =  18.0f;
static const float RETRATO_Y  =  18.0f;
static const float RETRATO_S  =  48.0f;

static const float VIDA_X     =  74.0f;
static const float VIDA_Y     =  22.0f;
static const int   VIDA_PX    =  12;

static const float LLAVE_IX   =  74.0f;
static const float LLAVE_IY   =  42.0f;
static const float LLAVE_IS   =  24.0f;   // 24×24, era 16×16

static const float LLAVE_TX   = 102.0f;   // 74 + 24px icono + 4px gap
static const float LLAVE_TY   =  48.0f;
static const int   LLAVE_PX   =  12;

// Consola
static const float MSG_PADDING      =   8.0f;
static const float MSG_LINE_H       =  18.0f;
static const int   MSG_FONT_PX      =   8;
// (500 - 2*8) / 8px_por_char ≈ 60 chars; usamos 56 de margen
static const int   MSG_CHARS_LINEA  =  56;

// ============================================================
// WORD WRAP
// ============================================================

static std::vector<std::string> wrapLinea(const std::string& texto, int maxChars) {
    std::vector<std::string> result;
    size_t pos = 0;
    while (pos < texto.size()) {
        size_t restante = texto.size() - pos;
        if (static_cast<int>(restante) <= maxChars) {
            result.push_back(texto.substr(pos));
            break;
        }
        // Buscar último espacio dentro del límite
        size_t breakAt = pos + maxChars;
        size_t espacio = texto.rfind(' ', breakAt);
        if (espacio == std::string::npos || espacio <= pos) {
            result.push_back(texto.substr(pos, maxChars));
            pos += maxChars;
        } else {
            result.push_back(texto.substr(pos, espacio - pos));
            pos = espacio + 1;
        }
    }
    return result;
}

// ============================================================
// CONSTRUCTOR
// ============================================================

Hud::Hud(Jugador* jugador_) : jugador(jugador_) {}

// ============================================================
// CARGAR FUENTE
// ============================================================

bool Hud::cargarFuente() {
    fs::path cwd = fs::current_path();

    // Nombres de archivo a probar (en orden de preferencia)
    std::vector<std::string> nombres = { "fuente.ttf", "PressStart2P-Regular.ttf" };

    // Directorios a probar (mismo patrón que Jugador)
    std::vector<fs::path> bases = {
        cwd,
        cwd / "DungeonJuego",
        cwd.parent_path(),
        cwd.parent_path().parent_path()
    };

    for (const auto& base : bases) {
        for (const auto& nombre : nombres) {
            fs::path ruta = base / "assets" / "fonts" / nombre;
            if (fs::exists(ruta) && fuente.loadFromFile(ruta.string())) {
                std::cout << "[HUD] Fuente cargada: " << ruta << "\n";
                return true;
            }
        }
    }

    std::cerr << "[HUD] Fuente no encontrada en assets/fonts/. Texto desactivado.\n";
    return false;
}

// ============================================================
// CARGAR IMAGEN
// ============================================================

bool Hud::cargarImagen(Textura*& tex, Imagen*& img, const std::string& nombreArchivo) {
    fs::path cwd = fs::current_path();

    std::vector<fs::path> rutas = {
        cwd / "assets" / "textures" / "ui" / nombreArchivo,
        cwd / "DungeonJuego" / "assets" / "textures" / "ui" / nombreArchivo,
        cwd.parent_path() / "assets" / "textures" / "ui" / nombreArchivo,
        cwd.parent_path().parent_path() / "assets" / "textures" / "ui" / nombreArchivo
    };

    for (const auto& ruta : rutas) {
        if (fs::exists(ruta)) {
            try {
                tex = new Textura();
                img = new Imagen();
                tex->carga(ruta);
                img->asigna(tex);
                return true;
            } catch (...) {
                delete tex; tex = nullptr;
                delete img; img = nullptr;
            }
        }
    }

    std::cerr << "[HUD] Imagen no encontrada: " << nombreArchivo << ". Usando placeholder.\n";
    return false;
}

// ============================================================
// INICIA
// ============================================================

void Hud::inicia() {
    // --- 1. Cargar fuente SF ----------------------------------------
    fuenteCargada = cargarFuente();

    // --- 2. Panel stats (superior izquierda) -----------------------
    panelStats = new Rectangulo(1.0f, 1.0f);
    panelStats->ponColor(Color(0, 0, 0, 160));
    panelStats->ponIndiceZ(150);
    agregaDibujo(panelStats);

    // --- 3. Retrato del personaje ----------------------------------
    placeholderRetrato = new Rectangulo(1.0f, 1.0f);
    placeholderRetrato->ponColor(Color(80, 80, 120, 200));
    placeholderRetrato->ponIndiceZ(151);
    agregaDibujo(placeholderRetrato);

    retratoCargado = cargarImagen(texturaRetrato, spriteRetrato, "retrato_heroe.png");
    if (retratoCargado) {
        spriteRetrato->ponIndiceZ(151);
        agregaDibujo(spriteRetrato);
        placeholderRetrato->ponVisible(false);
    }

    // --- 4. Icono de llave -----------------------------------------
    placeholderLlave = new Rectangulo(1.0f, 1.0f);
    placeholderLlave->ponColor(Color(200, 180, 0, 220));
    placeholderLlave->ponIndiceZ(151);
    agregaDibujo(placeholderLlave);

    llaveCargada = cargarImagen(texturaLlave, spriteLlave, "llave_icon.png");
    if (llaveCargada) {
        spriteLlave->ponIndiceZ(151);
        agregaDibujo(spriteLlave);
        placeholderLlave->ponVisible(false);
    }

    // --- 5. Textos del panel stats (solo si fuenteCargada) ---------
    if (fuenteCargada) {
        textoVida   = new HudTexto(&fuente);
        textoLlaves = new HudTexto(&fuente);

        textoVida  ->ponColor(Color(255, 255, 255));
        textoLlaves->ponColor(Color(255, 255, 255));

        textoVida  ->ponIndiceZ(160);
        textoLlaves->ponIndiceZ(160);

        agregaDibujo(textoVida);
        agregaDibujo(textoLlaves);
    }

    // --- 6. Panel consola (inferior izquierda) ---------------------
    panelConsola = new Rectangulo(1.0f, 1.0f);
    panelConsola->ponColor(Color(0, 20, 60, 150));
    panelConsola->ponIndiceZ(150);
    agregaDibujo(panelConsola);

    // --- 7. Textos de mensajes (solo si fuenteCargada) -------------
    if (fuenteCargada) {
        for (int i = 0; i < MAX_LINEAS; i++) {
            textosMsg[i] = new HudTexto(&fuente);
            textosMsg[i]->ponColor(Color(255, 255, 255, 255));
            textosMsg[i]->ponIndiceZ(160);
            textosMsg[i]->ponVisible(false);
            agregaDibujo(textosMsg[i]);
        }
    }

    // --- 8. Mensaje de bienvenida ----------------------------------
    agregarMensaje("ESPAÑA, LICÁNTROPO, 876, Tu Madre se ha comido a mi perro. Encuentra las llaves.", 10.0f);
}

// ============================================================
// TERMINA
// ============================================================

void Hud::termina() {
    extraeDibujos();

    // Liberar imágenes
    delete spriteRetrato;    spriteRetrato    = nullptr;
    delete texturaRetrato;   texturaRetrato   = nullptr;
    delete spriteLlave;      spriteLlave      = nullptr;
    delete texturaLlave;     texturaLlave     = nullptr;

    // Liberar textos stats
    delete textoVida;    textoVida    = nullptr;
    delete textoLlaves;  textoLlaves  = nullptr;

    // Liberar paneles y placeholders (extraeDibujos los saca de la lista pero no los destruye)
    delete panelStats;          panelStats         = nullptr;
    delete panelConsola;        panelConsola       = nullptr;
    delete placeholderRetrato;  placeholderRetrato = nullptr;
    delete placeholderLlave;    placeholderLlave   = nullptr;
    for (int i = 0; i < MAX_LINEAS; i++) {
        delete textosMsg[i];
        textosMsg[i] = nullptr;
    }
}

// ============================================================
// ACTUALIZA
// ============================================================

void Hud::actualiza(double tiempo_seg) {
    float dt = 0.0f;
    if (ultimoTiempo >= 0.0) {
        dt = static_cast<float>(tiempo_seg - ultimoTiempo);
        dt = std::min(dt, 0.1f);
    }
    ultimoTiempo = tiempo_seg;

    // Descontar tiempo de cada mensaje; eliminar caducados
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
    if (static_cast<int>(mensajes.size()) >= MAX_MENSAJES) {
        mensajes.pop_front();
    }
    mensajes.push_back({ texto, duracion, duracion });
}

// ============================================================
// ACTUALIZAR VISUAL
// ============================================================

void Hud::actualizarVisual() {
    // En modo editor ocultamos todo el HUD de una vez
    if (enModoEditor) {
        for (auto* d : dibujos()) d->ponVisible(false);
        return;
    }

    float z = zoomActual;

    // Restaurar visibilidad de paneles (pueden haber sido ocultados por el editor)
    panelStats->ponVisible(true);
    panelConsola->ponVisible(true);
    if (placeholderRetrato) placeholderRetrato->ponVisible(!retratoCargado);
    if (retratoCargado && spriteRetrato) spriteRetrato->ponVisible(true);
    if (placeholderLlave)  placeholderLlave->ponVisible(!llaveCargada);
    if (llaveCargada && spriteLlave) spriteLlave->ponVisible(true);
    if (textoVida)   textoVida->ponVisible(fuenteCargada);
    if (textoLlaves) textoLlaves->ponVisible(fuenteCargada);

    // ─── Panel stats ─────────────────────────────────────────────
    panelStats->ponPosicion(screenToWorld(STATS_X, STATS_Y));
    panelStats->ponBase  (STATS_W / z);
    panelStats->ponAltura(STATS_H / z);

    // ─── Retrato (48×48 px pantalla) ─────────────────────────────
    Vector posRetrato = screenToWorld(RETRATO_X, RETRATO_Y);
    float tamRetrato  = RETRATO_S / z;

    if (retratoCargado && spriteRetrato) {
        spriteRetrato->ponPosicion(posRetrato);
        // Asume que la textura es RETRATO_S×RETRATO_S nativa; scale = 1/z por eje
        spriteRetrato->ponEscala(Vector(1.0f / z, 1.0f / z));
    } else if (placeholderRetrato) {
        placeholderRetrato->ponPosicion(posRetrato);
        placeholderRetrato->ponBase  (tamRetrato);
        placeholderRetrato->ponAltura(tamRetrato);
    }

    // ─── Icono llave (24×24 px pantalla) ─────────────────────────
    Vector posLlaveI = screenToWorld(LLAVE_IX, LLAVE_IY);
    float  tamLlave  = LLAVE_IS / z;

    if (llaveCargada && spriteLlave) {
        spriteLlave->ponPosicion(posLlaveI);
        spriteLlave->ponEscala(Vector(1.0f / z, 1.0f / z));
    } else if (placeholderLlave) {
        placeholderLlave->ponPosicion(posLlaveI);
        placeholderLlave->ponBase  (tamLlave);
        placeholderLlave->ponAltura(tamLlave);
    }

    // ─── Textos stats (coordenadas de pantalla, tamaño fijo) ────────
    if (fuenteCargada && jugador) {
        if (textoVida) {
            std::string vida = std::to_string(jugador->getVida()) + "/" + std::to_string(jugador->getVidaMax());
            textoVida->ponCadena(vida);
            textoVida->ponTamano(VIDA_PX);
            textoVida->ponPosicionPantalla(VIDA_X, VIDA_Y);
        }
        if (textoLlaves) {
            textoLlaves->ponCadena(std::to_string(jugador->getLlaves()));
            textoLlaves->ponTamano(LLAVE_PX);
            textoLlaves->ponPosicionPantalla(LLAVE_TX, LLAVE_TY);
        }
    }

    // ─── Panel consola ───────────────────────────────────────────
    float consolaY = WINDOW_H - 170.0f;
    panelConsola->ponPosicion(screenToWorld(CONSOLA_X, consolaY));
    panelConsola->ponBase  (CONSOLA_W / z);
    panelConsola->ponAltura(CONSOLA_H / z);

    // ─── Textos de mensajes con word-wrap ────────────────────────
    if (!fuenteCargada) return;

    // Aplanar mensajes en líneas de pantalla respetando el ancho del panel
    struct LineaDisplay { std::string texto; int alpha; };
    std::vector<LineaDisplay> lineas;
    for (const auto& msg : mensajes) {
        int a = 255;
        if (msg.tiempoRestante < 1.0f)
            a = std::max(0, std::min(255, static_cast<int>(msg.tiempoRestante * 255.0f)));
        for (const auto& l : wrapLinea(msg.texto, MSG_CHARS_LINEA))
            lineas.push_back({ l, a });
    }

    // Si hay más líneas que slots, mostrar las más recientes
    int startLine = std::max(0, static_cast<int>(lineas.size()) - MAX_LINEAS);

    for (int slot = 0; slot < MAX_LINEAS; slot++) {
        if (textosMsg[slot] == nullptr) continue;

        int lineIdx = startLine + slot;
        if (lineIdx >= static_cast<int>(lineas.size())) {
            textosMsg[slot]->ponVisible(false);
            continue;
        }

        const LineaDisplay& ld = lineas[static_cast<size_t>(lineIdx)];
        textosMsg[slot]->ponCadena(ld.texto);
        textosMsg[slot]->ponColor(Color(255, 255, 255, ld.alpha));
        textosMsg[slot]->ponTamano(MSG_FONT_PX);

        float screenX = CONSOLA_X + MSG_PADDING;
        float screenY = consolaY  + MSG_PADDING + slot * MSG_LINE_H;
        textosMsg[slot]->ponPosicionPantalla(screenX, screenY);
        textosMsg[slot]->ponVisible(true);
    }
}

// ============================================================
// SCREEN TO WORLD
// ============================================================

Vector Hud::screenToWorld(float sx, float sy) const {
    float wCX = WINDOW_W * 0.5f;
    float wCY = WINDOW_H * 0.5f;
    return Vector(
        (sx - wCX) / zoomActual + wCX,
        (sy - wCY) / zoomActual + wCY
    );
}
