#pragma once
#include "UNIR-2D.h"
#include <deque>
#include <string>

class Jugador;

// ============================================================
// HUD FIJO — Panel de stats + consola de mensajes
// ============================================================
// Se dibuja en coordenadas de PANTALLA FIJAS, independiente del
// zoom y la cámara. Mismo patrón de coordenadas que EditorMapa.
//
// PANEL SUPERIOR IZQUIERDA (panelStats):
//   Posición: x=10, y=10 px   Tamaño: 210×110 px
//   Layout vertical: retrato (48×48) | vida | llave+contador | reservado
//
// PANEL INFERIOR IZQUIERDA (panelConsola):
//   Posición: x=10, y=WINDOW_H-170 px   Tamaño: 320×160 px
//   Cola de hasta 5 mensajes apilados, más recientes abajo.
//   Duración por defecto: 4 s. Fade-out en el último segundo.
//
// FUENTE:
//   sf::Font cargado desde assets/fonts/ (fuente.ttf o PressStart2P-Regular.ttf).
//   Si no existe, paneles visibles pero sin texto (sin crash).
//
// INTEGRACIÓN:
//   1. JuegoDungeon::inicia() al final: new Hud(jugador) → agregaActor(hud)
//   2. JuegoDungeon::posactualiza():    hud->ponZoom(zoomNivel)
//   3. Para enviar mensajes desde otro actor:
//      a) Recibir Hud* (constructor o setter).
//      b) Llamar hud->agregarMensaje("texto") al ocurrir el evento.
// ============================================================
class Hud : public unir2d::ActorBase {
public:
    explicit Hud(Jugador* jugador);
    ~Hud() = default;

    void inicia()  override;
    void termina() override;
    void actualiza(double tiempo_seg) override;

    /// JuegoDungeon llama esto desde posactualiza(), antes del render.
    void ponZoom(float z) { zoomActual = z; actualizarVisual(); }

    /// Oculta todo el HUD mientras el editor está activo y lo restaura al salir.
    void ponModoEditor(bool b) { enModoEditor = b; actualizarVisual(); }

    /// Envía un mensaje a la consola inferior.
    /// Si la cola ya tiene MAX_MENSAJES, el más antiguo se descarta.
    void agregarMensaje(const std::string& texto, float duracion = 4.0f);

private:
    struct MensajeConsola {
        std::string texto;
        float tiempoRestante;
        float tiempoTotal;
    };

    Jugador*  jugador       = nullptr;
    float     zoomActual    = 3.0f;
    double    ultimoTiempo  = -1.0;
    bool      enModoEditor  = false;

    // --- Fuente SF (PARTE 2) ------------------------------------------
    sf::Font  fuente;
    bool      fuenteCargada = false;    // true si sf::Font cargó bien

    // --- Panel stats (superior izquierda) ----------------------------
    unir2d::Rectangulo* panelStats   = nullptr;

    // Retrato del personaje (48×48 px en pantalla)
    unir2d::Textura*    texturaRetrato      = nullptr;
    unir2d::Imagen*     spriteRetrato       = nullptr;
    bool                retratoCargado      = false;
    unir2d::Rectangulo* placeholderRetrato  = nullptr;

    // Icono de llave (24×24 px en pantalla)
    unir2d::Textura*    texturaLlave        = nullptr;
    unir2d::Imagen*     spriteLlave         = nullptr;
    bool                llaveCargada        = false;
    unir2d::Rectangulo* placeholderLlave    = nullptr;

    // Textos del panel stats (solo si fuenteCargada)
    unir2d::HudTexto*   textoVida           = nullptr;
    unir2d::HudTexto*   textoLlaves         = nullptr;

    // --- Panel consola (inferior izquierda) --------------------------
    unir2d::Rectangulo* panelConsola = nullptr;

    // Textos de mensajes (solo si fuenteCargada)
    static const int MAX_MENSAJES = 5;   // tamaño de la cola de mensajes
    static const int MAX_LINEAS   = 8;   // líneas visibles en el panel (word-wrap)
    unir2d::HudTexto* textosMsg[MAX_LINEAS] = {};

    // Cola de mensajes activos: push_back = más reciente, front = más antiguo
    std::deque<MensajeConsola> mensajes;

    static const float WINDOW_W;
    static const float WINDOW_H;

    /// Convierte píxeles de pantalla → coordenadas mundo (mismo patrón que EditorMapa).
    unir2d::Vector screenToWorld(float sx, float sy) const;

    /// Carga sf::Font desde assets/fonts/ con múltiples rutas de fallback.
    bool cargarFuente();

    /// Carga una imagen con múltiples rutas de fallback. Devuelve true si cargó.
    bool cargarImagen(unir2d::Textura*& tex, unir2d::Imagen*& img,
                      const std::string& nombreArchivo);

    /// Actualiza posiciones, tamaños y texto de todos los drawables según zoomActual.
    void actualizarVisual();
};
