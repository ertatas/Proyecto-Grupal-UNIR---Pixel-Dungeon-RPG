#include "UNIR-2D.h"
#include "JuegoDungeon.h"

using namespace unir2d;

static const float ANCHO_VENTANA = 1280.0f;
static const float ALTO_VENTANA  =  720.0f;

// ============================================================
// VENTANA
// 
// ============================================================

const std::wstring JuegoDungeon::tituloVentana() const {
    return L"Dungeon RPG";
}

void JuegoDungeon::regionVentana(Vector& posicion, Vector& tamano) const {
    posicion = Vector(100, 100);
    tamano   = Vector(ANCHO_VENTANA, ALTO_VENTANA);
}

// ============================================================
// INICIO
// ============================================================

void JuegoDungeon::inicia() {
    mapa    = new Mapa();
    jugador = new Jugador(mapa);
    editor  = new EditorMapa(mapa);
    hud     = new Hud(jugador);

    agregaActor(mapa);
    // El editor va ANTES que el jugador para consumir teclas primero.
    agregaActor(editor);
    agregaActor(jugador);
    // El HUD va al final para que dibuje siempre encima de todo.
    agregaActor(hud);

    // Conectar el HUD al jugador para que pueda enviar mensajes.
    jugador->ponHud(hud);
}

// ============================================================
// CÁMARA
// ============================================================

void JuegoDungeon::posactualiza(double /*tiempo_seg*/) {
    if (!mapa || !jugador || !editor || !hud) return;

    if (!editor->estaActivo()) {
        // Modo juego: la cámara sigue al jugador.
        Vector centroPantalla(ANCHO_VENTANA * 0.5f, ALTO_VENTANA * 0.5f);
        offsetCamara = centroPantalla - jugador->centroMundo();
    }
    // Modo editor: offsetCamara NO se modifica → cámara queda fija
    // en la posición en que estaba al activar el editor.
    // El cursor verde se mueve visiblemente por la pantalla.

    // Pasamos siempre el offset y el zoom al editor para que el cursor
    // y el panel UI se dibujen alineados con los tiles del mapa.
    editor->ponOffsetCamara(offsetCamara);
    editor->ponZoom(zoomNivel);
    hud   ->ponZoom(zoomNivel);

    // Detectar la transición editor→juego para restaurar el fog of war.
    bool editorActivoAhora = editor->estaActivo();
    mapa->ponModoEditor(editorActivoAhora);
    hud->ponModoEditor(editorActivoAhora);
    if (editorActivoAntes && !editorActivoAhora) {
        mapa->resetearFog();
        jugador->revelarDesdeJugador();
    }
    editorActivoAntes = editorActivoAhora;

    // Bloquear el jugador mientras el editor está activo
    // para que no reaccione a las mismas teclas que el cursor.
    jugador->ponBloqueado(editorActivoAhora);

    mapa   ->ponPosicion(offsetCamara);
    jugador->ponPosicion(offsetCamara);

    // Zoom: Q acerca, E aleja (rango 1x – 4x, pasos de 0.1)
    if (Teclado::pulsando(Tecla::Q)) {
        Teclado::consume(Tecla::Q);
        zoomNivel = std::min(zoomNivel + 0.1f, 4.0f);
    }
    if (Teclado::pulsando(Tecla::E)) {
        Teclado::consume(Tecla::E);
        zoomNivel = std::max(zoomNivel - 0.1f, 1.0f);
    }
    ponZoom(zoomNivel);
}

// ============================================================
// TERMINA
// ============================================================

void JuegoDungeon::termina() {
    extraeActor(hud);
    delete hud; hud = nullptr;

    extraeActor(editor);
    extraeActor(jugador);
    extraeActor(mapa);

    editor  = nullptr;
    jugador = nullptr;
    mapa    = nullptr;
}
