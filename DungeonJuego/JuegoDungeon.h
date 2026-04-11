#pragma once
#include "UNIR-2D.h"
#include "Mapa.h"
#include "Jugador.h"
#include "EditorMapa.h"
#include "Hud.h"

// ============================================================
// CLASE PRINCIPAL DEL JUEGO
// ============================================================
// Gestiona:
//   - El mapa y el jugador
//   - El editor in-game (se activa con F1)
//   - La cámara: sigue al jugador en modo juego,
//     es libre con flechas en modo editor
//   - El HUD fijo (paneles de stats y consola de mensajes)
// ============================================================
class JuegoDungeon : public unir2d::JuegoBase {
private:
    Mapa*        mapa    = nullptr;
    Jugador*     jugador = nullptr;
    EditorMapa*  editor  = nullptr;
    Hud*         hud     = nullptr;

    unir2d::Vector offsetCamara = {};
    float zoomNivel = 3.0f;
    bool  editorActivoAntes = false;   // para detectar la transición editor→juego

protected:
    const std::wstring tituloVentana() const override;
    void regionVentana(unir2d::Vector& posicion, unir2d::Vector& tamano) const override;
    void inicia()    override;
    void termina()   override;
    void posactualiza(double tiempo_seg) override;
};
