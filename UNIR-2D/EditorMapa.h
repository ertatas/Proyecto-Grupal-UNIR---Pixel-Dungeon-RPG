#pragma once
#include "UNIR-2D.h"
#include "Mapa.h"

// ============================================================
// EDITOR DE MAPA (cursor tile a tile, in-game)
// ============================================================
//
// Activar / desactivar con F1.
// El jugador queda congelado mientras el editor está activo.
//
// CAPAS (ciclar con A / D):
//   0 BASE      — estructura del nivel (azul)
//   1 PROPS     — decoración visual    (morado) — pared 1-9, suelo 10-19
//   2 TRAMPAS   — trampas ocultas      (rojo)
//   3 ENEMIGOS  — spawn de enemigos    (verde)
//   4 LLAVES    — llaves               (dorado)
//   5 VARIANTES — variante visual de pared/puerta (gris plateado)
//
// PARA AÑADIR UN NUEVO PROP EN EL FUTURO:
//   - Rango 1-9  → prop de pared (añadir en Mapa.h: PROP_NUEVO <= 9)
//   - Rango 10-19→ prop de suelo (añadir en Mapa.h: PROP_NUEVO >= 10 y <= 19)
//   El editor detecta el rango automáticamente. Solo hay que:
//   1. Añadir constante en Mapa.h (TipoProp)
//   2. Añadir color en Mapa.cpp → colorPropPared() o colorPropSuelo()
//   3. NO tocar EditorMapa — el rango lo gestiona todo.
//
// CONTROLES:
//   F1         → activar / desactivar editor (auto-guarda al salir)
//   Enter      → guardar sin salir
//   Flechas    → mover cursor tile a tile (atraviesa paredes)
//   A / D      → capa anterior / siguiente
//   W / S      → tipo anterior / siguiente dentro de la capa
//   Espacio    → pintar tile en la posición del cursor
//   Clic izq   → pintar tile bajo el ratón
//   Clic der   → borrar tile bajo el ratón
// ============================================================
class EditorMapa : public unir2d::ActorBase {
public:
    explicit EditorMapa(Mapa* mapa);
    ~EditorMapa();

    void inicia()  override;
    void termina() override;
    void actualiza(double tiempo_seg) override;

    bool           estaActivo()       const { return activo; }
    unir2d::Vector centroMundo()      const;
    void           ponOffsetCamara(unir2d::Vector v) { offsetCamara = v; }
    void           ponZoom(float z)                  { zoomActual   = z; }

private:
    Mapa*          mapa         = nullptr;
    bool           activo       = false;
    unir2d::Vector offsetCamara = {};
    float          zoomActual   = 1.0f;

    int filaActual = 1;
    int colActual  = 1;

    // 0=BASE  1=PROPS  2=TRAMPAS  3=ENEMIGOS  4=LLAVES  5=VARIANTES
    int capaActual = 0;
    int tipoActual = 1;

    static const int NUM_CAPAS = 6;
    static const int MAX_TIPOS = 8;   // máximo de tipos en cualquier capa (7 variantes + 1 margen)

    // Edge-detection
    bool f1Prev    = false;
    bool enterPrev = false;
    bool arrPrev   = false;
    bool abaoPrev  = false;
    bool izqPrev   = false;
    bool derPrev   = false;
    bool wPrev     = false;
    bool sPrev     = false;
    bool aPrev     = false;
    bool dPrev     = false;
    bool spacePrev = false;
    bool pPrev     = false;

    // Drawables
    unir2d::Rectangulo* cursor       = nullptr;
    unir2d::Rectangulo* panelFondo   = nullptr;
    unir2d::Rectangulo* bannerCapa   = nullptr;
    unir2d::Rectangulo* cajasTipo[MAX_TIPOS] = {};
    unir2d::Rectangulo* selIndicador = nullptr;
    unir2d::Rectangulo* spawnMarker  = nullptr;

    // Helpers
    void manejarInput();
    void actualizarVisual();
    void actualizarSpawnMarker();
    void pintarEnCursor();
    bool screenToTile(unir2d::Vector screenPos, int& fila, int& col) const;
    int  maxTipos()                               const;
    unir2d::Color colorBanner(int capa)           const;
    unir2d::Color colorCursor(int capa)           const;
    unir2d::Color colorTipoPaleta(int capa, int tipo) const;
    void mostrarUI(bool valor);
    void pintarTile(int fila, int col, int tipo);
    void borrarTile(int fila, int col);
};
