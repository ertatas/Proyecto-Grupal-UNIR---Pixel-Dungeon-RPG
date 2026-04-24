#pragma once
#include "UNIR-2D.h"

class Mapa;
class Hud;

// ============================================================
// CLASE JUGADOR
// ============================================================
// El jugador:
//
// - guarda referencia al mapa
// - tiene textura + imagen
// - se mueve por casillas
// - usa pulsación única para no avanzar varias casillas de golpe
// ============================================================
class Jugador : public unir2d::ActorBase {
private:
    // Referencia al mapa para consultar colisiones.
    Mapa* mapa = nullptr;

    // Puntero opcional al HUD para enviar mensajes. Puede ser nullptr.
    Hud*  hud  = nullptr;

    // Sprite del jugador.
    unir2d::Textura* texturaJugador = nullptr;
    unir2d::Imagen* imagenJugador = nullptr;

    // Posición lógica por casillas.
    int fila = 1;
    int col = 1;

    bool bloqueado = false;

    // Llaves recogidas del suelo.
    int llaves  = 0;

    // Vida del jugador.
    int vida    = 100;
    int vidaMax = 100;

    // Estado anterior del teclado.
    bool izquierdaPulsada = false;
    bool derechaPulsada = false;
    bool arribaPulsada = false;
    bool abajoPulsada = false;

public:
    explicit Jugador(Mapa* mapa);
    ~Jugador();

    // El editor llama a esto para que el jugador no reaccione al teclado.
    void ponBloqueado(bool valor) { bloqueado = valor; }

    // Conecta el HUD para que el jugador pueda enviar mensajes a la consola.
    void ponHud(Hud* h) { hud = h; }

    // Número de llaves en el inventario.
    int getLlaves()  const { return llaves; }

    // Vida y vida máxima del jugador.
    int getVida()    const { return vida; }
    int getVidaMax() const { return vidaMax; }

    void inicia() override;
    void termina() override;
    void actualiza(double tiempo_seg) override;

    // Devuelve el centro del jugador en el mundo.
    unir2d::Vector centroMundo() const;

    // Revela el fog desde la casilla actual del jugador.
    // Llamar justo después de resetearFog() al cerrar el editor.
    void revelarDesdeJugador();

private:
    // Carga la textura del jugador.
    void cargarSpriteJugador();

    // Coloca visualmente el sprite en la casilla actual.
    void actualizarPosicionVisual();

    // Intenta mover una casilla.
    void intentarMover(int df, int dc);

    // Envía un mensaje al HUD si está conectado (hud != nullptr).
    void enviarMensaje(const std::string& msg);
};