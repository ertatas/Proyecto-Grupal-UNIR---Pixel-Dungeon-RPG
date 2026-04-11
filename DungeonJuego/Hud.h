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
// Se añade al motor DESPUÉS del jugador para dibujar encima.
//
// PANEL SUPERIOR IZQUIERDA (panelStats):
//   Posición: x=10, y=10 px   Tamaño: 200×90 px
//   TODO_HUD_STATS: aquí irán retrato del jugador, vida y contador de llaves.
//   Para añadir texto:  usar unir2d::Texto (clase ya disponible en el motor).
//   Para añadir icono de llave: cargar PNG desde assets/textures/ui/llave_icon.png
//   Leer número de llaves con: jugador->getLlaves()
//
// PANEL INFERIOR IZQUIERDA (panelConsola):
//   Posición: x=10, y=WINDOW_H-170 px   Tamaño: 320×160 px
//   Cola de hasta 5 mensajes apilados, más recientes abajo.
//   Duración por defecto: 4 s. Fade-out en el último segundo.
//
// FUENTE:
//   Se carga "{cwd}/fuentes/DejaVuSans.ttf".
//   Si no existe, los paneles aparecen pero sin texto (sin crash).
//   Para activar: copiar UNIR-2D/fuentes/DejaVuSans.ttf a DungeonJuego/fuentes/
//
// INTEGRACIÓN:
//   1. JuegoDungeon::inicia() al final: new Hud(jugador) → agregaActor(hud)
//   2. JuegoDungeon::posactualiza():    hud->ponZoom(zoomNivel)
//   3. Para enviar mensajes desde otro actor:
//      a) Recibir Hud* (constructor o setter).
//      b) Llamar hud->agregarMensaje("texto") al ocurrir el evento.
//      c) Opcional: pasar duracion como segundo argumento (defecto 4 s).
// ============================================================
class Hud : public unir2d::ActorBase {
public:
    explicit Hud(Jugador* jugador);
    ~Hud() = default;

    void inicia()  override;
    void termina() override;
    void actualiza(double tiempo_seg) override;

    /// JuegoDungeon llama esto cada frame (igual que EditorMapa::ponZoom).
    void ponZoom(float z) { zoomActual = z; }

    /// Envía un mensaje a la consola inferior.
    /// Si la cola ya tiene MAX_MENSAJES, el más antiguo se descarta.
    void agregarMensaje(const std::string& texto, float duracion = 4.0f);

private:
    struct MensajeConsola {
        std::string texto;
        float tiempoRestante;   // segundos hasta desaparecer
        float tiempoTotal;      // para calcular alpha de fade
    };

    Jugador*  jugador       = nullptr;
    float     zoomActual    = 3.0f;    // sincronizado con JuegoDungeon::zoomNivel
    bool      fuenteCargada = false;
    double    ultimoTiempo  = -1.0;    // para calcular deltaT

    // Panel stats (superior izquierda)
    unir2d::Rectangulo* panelStats   = nullptr;

    // Panel consola (inferior izquierda)
    unir2d::Rectangulo* panelConsola = nullptr;

    // Textos de mensajes — solo válidos si fuenteCargada == true
    static const int MAX_MENSAJES = 5;
    unir2d::Texto*  textosMsg[MAX_MENSAJES] = {};

    // Cola de mensajes activos: push_back = más reciente, front = más antiguo
    std::deque<MensajeConsola> mensajes;

    static const float WINDOW_W;
    static const float WINDOW_H;

    /// Convierte píxeles de pantalla → coordenadas mundo (mismo patrón que EditorMapa).
    /// Con zoom z, un punto de pantalla (sx, sy) se ve en el mundo en la posición devuelta.
    unir2d::Vector screenToWorld(float sx, float sy) const;

    /// Actualiza posiciones, tamaños y texto de todos los drawables según zoomActual.
    void actualizarVisual();
};
