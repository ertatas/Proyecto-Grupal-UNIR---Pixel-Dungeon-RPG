#pragma once
#include "UNIR-2D.h"

class Mapa;

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

    // Sprite del jugador.
    unir2d::Textura* texturaJugador = nullptr;
    unir2d::Imagen* imagenJugador = nullptr;

    // Posición lógica por casillas.
    int fila = 1;
    int col = 1;

    bool bloqueado = false;

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

    void inicia() override;
    void termina() override;
    void actualiza(double tiempo_seg) override;

    // Devuelve el centro del jugador en el mundo.
    unir2d::Vector centroMundo() const;

private:
    // Carga la textura del jugador.
    void cargarSpriteJugador();

    // Coloca visualmente el sprite en la casilla actual.
    void actualizarPosicionVisual();

    // Intenta mover una casilla.
    void intentarMover(int df, int dc);
};