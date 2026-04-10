#include "UNIR-2D.h"
#include "Jugador.h"
#include "Mapa.h"
#include <iostream>
#include <filesystem>

using namespace unir2d;
namespace fs = std::filesystem;

// ============================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================

Jugador::Jugador(Mapa* mapa) : mapa(mapa) {
}

Jugador::~Jugador() {
    // No hacemos delete aquí.
    // El cierre lo controlamos en termina().
}

// ============================================================
// INICIA
// ============================================================

void Jugador::inicia() {
    // Posición inicial desde el punto de spawn guardado en el mapa.
    fila = mapa->getSpawnFila();
    col  = mapa->getSpawnCol();

    // Cargamos la textura del jugador.
    cargarSpriteJugador();

    // Si todo ha ido bien, añadimos la imagen al motor.
    if (imagenJugador != nullptr) {
        agregaDibujo(imagenJugador);
        actualizarPosicionVisual();
    }

    // Revelar el área inicial alrededor del spawn.
    mapa->revelarZonaDesde(fila, col);
}

// ============================================================
// TERMINA
// ============================================================

void Jugador::termina() {
    // Sacamos los dibujos del actor.
    extraeDibujos();

    // No hacemos delete manual para evitar dobles liberaciones.
    imagenJugador = nullptr;
    texturaJugador = nullptr;
}

// ============================================================
// ACTUALIZA
// ============================================================

void Jugador::actualiza(double tiempo_seg) {
    (void)tiempo_seg;

    // Cuando el editor está activo el jugador no hace nada.
    if (bloqueado) return;

    // Leemos el estado actual del teclado.
    bool izq = Teclado::pulsando(Tecla::izquierda);
    bool der = Teclado::pulsando(Tecla::derecha);
    bool arr = Teclado::pulsando(Tecla::arriba);
    bool aba = Teclado::pulsando(Tecla::abajo);

    // Movimiento por pulsación única.
    if (izq && !izquierdaPulsada) {
        intentarMover(0, -1);
    }

    if (der && !derechaPulsada) {
        intentarMover(0, 1);
    }

    if (arr && !arribaPulsada) {
        intentarMover(-1, 0);
    }

    if (aba && !abajoPulsada) {
        intentarMover(1, 0);
    }

    // Guardamos el estado para el siguiente frame.
    izquierdaPulsada = izq;
    derechaPulsada = der;
    arribaPulsada = arr;
    abajoPulsada = aba;
}

// ============================================================
// CENTRO DEL JUGADOR EN EL MUNDO
// ============================================================

Vector Jugador::centroMundo() const {
    if (mapa == nullptr) {
        return Vector(0.0f, 0.0f);
    }

    float tam = mapa->tamanoCasilla();

    return Vector(
        col * tam + tam * 0.5f,
        fila * tam + tam * 0.5f
    );
}

// ============================================================
// CARGAR SPRITE DEL JUGADOR
// ============================================================

void Jugador::cargarSpriteJugador() {
    // Creamos textura e imagen.
    texturaJugador = new Textura();
    imagenJugador = new Imagen();

    // Probamos varias rutas posibles.
    fs::path ruta1 = fs::current_path() / "assets" / "textures" / "characters" / "hero" / "hero_idle.png";
    fs::path ruta2 = fs::current_path() / "DungeonJuego" / "assets" / "textures" / "characters" / "hero" / "hero_idle.png";
    fs::path ruta3 = fs::current_path().parent_path() / "assets" / "textures" / "characters" / "hero" / "hero_idle.png";
    fs::path ruta4 = fs::current_path().parent_path().parent_path() / "assets" / "textures" / "characters" / "hero" / "hero_idle.png";

    fs::path rutaFinal;

    if (fs::exists(ruta1)) {
        rutaFinal = ruta1;
    }
    else if (fs::exists(ruta2)) {
        rutaFinal = ruta2;
    }
    else if (fs::exists(ruta3)) {
        rutaFinal = ruta3;
    }
    else if (fs::exists(ruta4)) {
        rutaFinal = ruta4;
    }
    else {
        std::cerr << "No se encuentra hero_idle.png en ninguna ruta esperada.\n";
        std::cerr << "Current path: " << fs::current_path() << std::endl;

        imagenJugador = nullptr;
        texturaJugador = nullptr;
        return;
    }

    // Cargamos la textura y la asignamos a la imagen.
    texturaJugador->carga(rutaFinal);
    imagenJugador->asigna(texturaJugador);
}

// ============================================================
// INTENTAR MOVER
// ============================================================

void Jugador::intentarMover(int df, int dc) {
    if (mapa == nullptr) {
        return;
    }

    int nuevaFila = fila + df;
    int nuevaCol = col + dc;

    if (mapa->esTransitable(nuevaFila, nuevaCol)) {
        fila = nuevaFila;
        col = nuevaCol;
        actualizarPosicionVisual();
        mapa->revelarZonaDesde(fila, col);
    }
}

// ============================================================
// ACTUALIZAR POSICIÓN VISUAL
// ============================================================

void Jugador::actualizarPosicionVisual() {
    if (imagenJugador == nullptr || mapa == nullptr) {
        return;
    }

    float tam = mapa->tamanoCasilla();

    // El sprite es 32x32.
    // Lo centramos dentro de la casilla.
    float offsetX = (tam - 32.0f) / 2.0f;
    float offsetY = (tam - 32.0f) / 2.0f;

    imagenJugador->ponPosicion(
        Vector(
            col * tam + offsetX,
            fila * tam + offsetY
        )
    );
}