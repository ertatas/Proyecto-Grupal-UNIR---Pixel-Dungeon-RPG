#include "UNIR-2D.h"
#include "Mapa.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <queue>

using namespace unir2d;
namespace fs = std::filesystem;

static std::string rutaMapa() {
    return (fs::current_path() / "mapa.txt").string();
}


// ============================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================

Mapa::Mapa() {
    // Si existe mapa.txt lo cargamos; si no, usamos el mapa por defecto.
    std::string ruta = rutaMapa();
    if (fs::exists(ruta)) {
        // crearCapas y calcularCoordenadasVisuales se llaman dentro de cargarMapa
        cargarMapaDesdeArchivo(ruta);
    } else {
        construirMapaInicial();
    }
}

Mapa::~Mapa() {}

// ============================================================
// CICLO DE VIDA
// ============================================================

void Mapa::inicia() {
    crearDibujos();
    refrescarDibujos();
}

void Mapa::termina() {
    extraeDibujos();
    limpiarListasDibujos();
}

// ============================================================
// API PÚBLICA – CONSULTA
// ============================================================

int Mapa::getBase(int fila, int col) const {
    if (!dentroRango(fila, col)) return BASE_SUELO;
    return capaBase[fila][col];
}

int Mapa::getProp(int fila, int col) const {
    if (!dentroRango(fila, col)) return PROP_NADA;
    return capaProps[fila][col];
}

bool Mapa::esTransitable(int fila, int col) const {
    if (!dentroRango(fila, col))                        return false;
    if (capaBase[fila][col] == BASE_PARED)              return false;
    if (capaBase[fila][col] == BASE_PUERTA_CERRADA)     return false;
    if (capaEnemigos[fila][col] == ENEMIGO_NORMAL)      return false;
    return true;
}

Vector Mapa::centroCasilla(int fila, int col) const {
    return Vector(col * tamCasilla, fila * tamCasilla);
}

// ============================================================
// API PÚBLICA – MODIFICACIÓN
// ============================================================

void Mapa::setBase(int fila, int col, int tipo) {
    if (!dentroRango(fila, col)) return;
    capaBase[fila][col] = tipo;
    // Recalcular variantes visuales (afecta a vecinos por variantes de pared)
    calcularCoordenadasVisuales();
    refrescarDibujos();
}

void Mapa::setProp(int fila, int col, int tipo) {
    if (!dentroRango(fila, col)) return;
    capaProps[fila][col] = tipo;
    int i = fila * columnas + col;
    dibujosProps[i]->ponColor(colorProp(fila, col));
}

void Mapa::setTrampa(int fila, int col, int tipo) {
    if (!dentroRango(fila, col)) return;
    capaTrampas[fila][col] = tipo;
    int i = fila * columnas + col;
    dibujosTrampas[i]->ponColor(colorTrampa(fila, col));
}

void Mapa::setEnemigo(int fila, int col, int tipo) {
    if (!dentroRango(fila, col)) return;
    capaEnemigos[fila][col] = tipo;
    // Actualizar colisión visual del tile base también (enemigo bloquea paso)
    int i = fila * columnas + col;
    dibujosEnemigos[i]->ponColor(colorEnemigo(fila, col));
}

void Mapa::setLlave(int fila, int col, int tipo) {
    if (!dentroRango(fila, col)) return;
    capaLlaves[fila][col] = tipo;
    int i = fila * columnas + col;
    dibujosLlaves[i]->ponColor(colorLlave(fila, col));
}

void Mapa::setSpawn(int fila, int col) {
    if (!dentroRango(fila, col)) return;
    spawnFila = fila;
    spawnCol  = col;
}

// ============================================================
// PERSISTENCIA
// ============================================================

void Mapa::guardarMapa(const std::string& /*ignorada*/) const {
    std::ofstream f(rutaMapa());
    if (!f.is_open()) return;

    f << "DUNGEON_MAP 1\n";
    f << filas << " " << columnas << "\n";

    auto escribeCapa = [&](const std::string& nombre,
                           const std::vector<std::vector<int>>& capa) {
        f << nombre << "\n";
        for (int r = 0; r < filas; r++) {
            for (int c = 0; c < columnas; c++) {
                f << capa[r][c];
                if (c < columnas - 1) f << " ";
            }
            f << "\n";
        }
    };

    escribeCapa("BASE",     capaBase);
    escribeCapa("PROPS",    capaProps);
    escribeCapa("TRAMPAS",  capaTrampas);
    escribeCapa("ENEMIGOS", capaEnemigos);
    escribeCapa("LLAVES",   capaLlaves);
    f << "SPAWN\n" << spawnFila << " " << spawnCol << "\n";
    f << "END\n";
}

void Mapa::cargarMapaDesdeArchivo(const std::string& ruta) {
    std::ifstream f(ruta);
    if (!f.is_open()) return;

    std::string linea;

    // Cabecera
    std::getline(f, linea); // DUNGEON_MAP 1
    int nuevasFilas = 0, nuevasColumnas = 0;
    f >> nuevasFilas >> nuevasColumnas;
    std::getline(f, linea); // consume el salto de línea restante

    crearCapas(nuevasFilas, nuevasColumnas);

    auto leeCapa = [&](std::vector<std::vector<int>>& capa) {
        std::getline(f, linea); // nombre de capa (BASE, PROPS…)
        for (int r = 0; r < filas; r++) {
            std::getline(f, linea);
            std::istringstream ss(linea);
            for (int c = 0; c < columnas; c++) {
                ss >> capa[r][c];
            }
        }
    };

    leeCapa(capaBase);
    leeCapa(capaProps);
    leeCapa(capaTrampas);
    leeCapa(capaEnemigos);
    leeCapa(capaLlaves);

    // SPAWN (opcional — compatibilidad con ficheros guardados sin esta sección)
    std::string token;
    if (f >> token && token == "SPAWN") {
        f >> spawnFila >> spawnCol;
    }

    calcularCoordenadasVisuales();
}

void Mapa::cargarMapa(const std::string& /*ignorada*/) {
    cargarMapaDesdeArchivo(rutaMapa());
    // Reconstruir drawables (solo si ya estamos inicializados)
    extraeDibujos();
    limpiarListasDibujos();
    crearDibujos();
    refrescarDibujos();
}

// ============================================================
// CONSTRUCCIÓN INTERNA
// ============================================================

void Mapa::crearCapas(int nuevasFilas, int nuevasColumnas) {
    filas    = nuevasFilas;
    columnas = nuevasColumnas;

    capaBase        .assign(filas, std::vector<int>(columnas, BASE_SUELO));
    capaProps       .assign(filas, std::vector<int>(columnas, PROP_NADA));
    capaTrampas     .assign(filas, std::vector<int>(columnas, TRAMPA_NADA));
    capaEnemigos    .assign(filas, std::vector<int>(columnas, ENEMIGO_NADA));
    capaLlaves      .assign(filas, std::vector<int>(columnas, LLAVE_NADA));
    capaVisibilidad .assign(filas, std::vector<int>(columnas, VIS_OCULTO));
    capaVisual      .assign(filas, std::vector<CoordenadasVisuales>(columnas));
}

// ============================================================
// FOG OF WAR
// ============================================================

void Mapa::revelarZonaDesde(int fila, int col) {
    // BFS que revela la zona completa donde está el jugador.
    //
    // Reglas de propagación:
    //   BASE_SUELO           → se encola (el BFS se propaga)
    //   BASE_PARED           → se revela en el sitio desde la celda de suelo adyacente,
    //                          pero NO se encola (no propaga a través de muros)
    //   BASE_PUERTA_*        → se revela en el sitio, NO se encola (frontera de zona)
    //
    // Esto garantiza que el flood-fill no "sangra" a través de las paredes
    // hacia habitaciones contiguas.

    if (!dentroRango(fila, col)) {
        refrescarDibujos();
        return;
    }

    std::vector<std::vector<bool>> visitado(filas, std::vector<bool>(columnas, false));
    std::queue<std::pair<int,int>> cola;

    // El tile inicial puede ser suelo o puerta (al estar parado sobre una puerta).
    cola.push({fila, col});
    visitado[fila][col] = true;

    const int df[] = { -1, 1,  0, 0 };
    const int dc[] = {  0, 0, -1, 1 };

    while (!cola.empty()) {
        auto [f, c] = cola.front();
        cola.pop();

        capaVisibilidad[f][c] = VIS_VISIBLE;

        // Solo se propaga desde tiles de suelo.
        // Las puertas se revelan pero no expanden la búsqueda.
        if (capaBase[f][c] != BASE_SUELO) continue;

        for (int d = 0; d < 4; d++) {
            int nf = f + df[d];
            int nc = c + dc[d];
            if (!dentroRango(nf, nc)) continue;
            if (visitado[nf][nc])     continue;
            visitado[nf][nc] = true;

            int tipoVecino = capaBase[nf][nc];
            if (tipoVecino == BASE_SUELO) {
                // Suelo contiguo de la misma zona: propagar.
                cola.push({nf, nc});
            } else {
                // Pared o puerta adyacente al suelo: revelar sin propagar.
                capaVisibilidad[nf][nc] = VIS_VISIBLE;
            }
        }
    }

    refrescarDibujos();
}

void Mapa::revelarTodo() {
    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < columnas; c++) {
            capaVisibilidad[f][c] = VIS_VISIBLE;
        }
    }
    refrescarDibujos();
}

void Mapa::construirMapaInicial() {
    crearCapas(32, 32);

    // Bordes exteriores
    for (int c = 0; c < columnas; c++) {
        capaBase[0][c]         = BASE_PARED;
        capaBase[filas-1][c]   = BASE_PARED;
    }
    for (int f = 0; f < filas; f++) {
        capaBase[f][0]         = BASE_PARED;
        capaBase[f][columnas-1] = BASE_PARED;
    }

    // Mapa inicial limpio — usa el editor (F1) para construir el nivel.

    calcularCoordenadasVisuales();
}

void Mapa::calcularCoordenadasVisuales() {
    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < columnas; c++) {
            CoordenadasVisuales& vis = capaVisual[f][c];
            vis = CoordenadasVisuales{};

            if (capaBase[f][c] == BASE_SUELO) {
                vis.baseX = (f + c) % 3;
            }
            else if (capaBase[f][c] == BASE_PARED) {
                bool arriba    = esPared(f-1, c);
                bool abajo     = esPared(f+1, c);
                bool izquierda = esPared(f, c-1);
                bool derecha   = esPared(f, c+1);

                if (izquierda && derecha && !arriba && !abajo)  vis.baseX = 0;
                else if (arriba && abajo && !izquierda && !derecha) vis.baseX = 1;
                else if (abajo && derecha)   vis.baseX = 2;
                else if (abajo && izquierda) vis.baseX = 3;
                else if (arriba && derecha)  vis.baseX = 4;
                else if (arriba && izquierda)vis.baseX = 5;
                else                         vis.baseX = 6;
                vis.baseY = 1;
            }
            else if (capaBase[f][c] == BASE_PUERTA_CERRADA) {
                vis.baseX = 0; vis.baseY = 2;
            }
            else if (capaBase[f][c] == BASE_PUERTA_ABIERTA) {
                vis.baseX = 1; vis.baseY = 2;
            }
        }
    }
}

// ============================================================
// DRAWABLES
// ============================================================

void Mapa::crearDibujos() {
    limpiarListasDibujos();

    int total = filas * columnas;
    dibujosBase    .reserve(total);
    dibujosTrampas .reserve(total);
    dibujosEnemigos.reserve(total);
    dibujosLlaves  .reserve(total);
    dibujosProps   .reserve(total);

    // Dimensiones del prop decorativo (lámpara/antorcha):
    // centrado horizontalmente, pegado a la parte superior del tile.
    const float pw = tamCasilla * 0.25f;
    const float ph = tamCasilla * 0.38f;

    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < columnas; c++) {
            float tx = c * tamCasilla;
            float ty = f * tamCasilla;

            // ---- BASE (tile completo) ----
            Rectangulo* rBase = new Rectangulo(tamCasilla, tamCasilla);
            rBase->ponPosicion(Vector(tx, ty));
            rBase->ponIndiceZ(0);
            agregaDibujo(rBase);
            dibujosBase.push_back(rBase);

            // ---- TRAMPA (pequeño, centrado) ----
            Rectangulo* rTrampa = new Rectangulo(tamCasilla * 0.22f, tamCasilla * 0.22f);
            rTrampa->ponPosicion(Vector(tx + tamCasilla * 0.39f, ty + tamCasilla * 0.39f));
            rTrampa->ponIndiceZ(1);
            agregaDibujo(rTrampa);
            dibujosTrampas.push_back(rTrampa);

            // ---- ENEMIGO (grande) ----
            Rectangulo* rEnemigo = new Rectangulo(tamCasilla * 0.52f, tamCasilla * 0.52f);
            rEnemigo->ponPosicion(Vector(tx + tamCasilla * 0.24f, ty + tamCasilla * 0.24f));
            rEnemigo->ponIndiceZ(1);
            agregaDibujo(rEnemigo);
            dibujosEnemigos.push_back(rEnemigo);

            // ---- LLAVE (pequeña, arriba) ----
            Rectangulo* rLlave = new Rectangulo(tamCasilla * 0.18f, tamCasilla * 0.18f);
            rLlave->ponPosicion(Vector(tx + tamCasilla * 0.41f, ty + tamCasilla * 0.12f));
            rLlave->ponIndiceZ(1);
            agregaDibujo(rLlave);
            dibujosLlaves.push_back(rLlave);

            // ---- PROP (decoración sobre pared/suelo) ----
            Rectangulo* rProp = new Rectangulo(pw, ph);
            rProp->ponPosicion(Vector(tx + (tamCasilla - pw) * 0.5f, ty + tamCasilla * 0.06f));
            rProp->ponIndiceZ(2);   // encima de todo lo anterior
            agregaDibujo(rProp);
            dibujosProps.push_back(rProp);
        }
    }
}

void Mapa::refrescarDibujos() {
    if (dibujosBase.empty()) return;   // aún no se han creado los drawables
    calcularCoordenadasVisuales();

    static const Color transparente(0, 0, 0, 0);
    static const Color negroOpaco  (0, 0, 0, 255);

    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < columnas; c++) {
            int i    = f * columnas + c;
            float tx = c * tamCasilla;
            float ty = f * tamCasilla;

            dibujosBase[i]->ponPosicion(Vector(tx, ty));

            if (capaVisibilidad[f][c] == VIS_OCULTO) {
                // Zona no explorada: negro absoluto, no se dibuja nada
                dibujosBase[i]    ->ponColor(negroOpaco);
                dibujosTrampas[i] ->ponColor(transparente);
                dibujosEnemigos[i]->ponColor(transparente);
                dibujosLlaves[i]  ->ponColor(transparente);
                dibujosProps[i]   ->ponColor(transparente);
            } else {
                // Zona visible: colores normales
                dibujosBase[i]    ->ponColor(colorBase   (f, c));
                dibujosTrampas[i] ->ponColor(colorTrampa (f, c));
                dibujosEnemigos[i]->ponColor(colorEnemigo(f, c));
                dibujosLlaves[i]  ->ponColor(colorLlave  (f, c));
                dibujosProps[i]   ->ponColor(colorProp   (f, c));
            }
        }
    }
}

void Mapa::limpiarListasDibujos() {
    dibujosBase    .clear();
    dibujosProps   .clear();
    dibujosTrampas .clear();
    dibujosEnemigos.clear();
    dibujosLlaves  .clear();
}

// ============================================================
// UTILIDADES
// ============================================================

bool Mapa::dentroRango(int fila, int col) const {
    return fila >= 0 && fila < filas && col >= 0 && col < columnas;
}

int Mapa::tipoPuertaPorDefecto() const {
    return puertasAbiertasPorDefecto ? BASE_PUERTA_ABIERTA : BASE_PUERTA_CERRADA;
}

bool Mapa::esPared(int fila, int col) const {
    if (!dentroRango(fila, col)) return false;
    return capaBase[fila][col] == BASE_PARED;
}

// ============================================================
// COLORES
// ============================================================

Color Mapa::colorBase(int fila, int col) const {
    switch (capaBase[fila][col]) {
        case BASE_SUELO:          return Color( 90,  90,  90);
        case BASE_PARED:          return Color( 45,  45,  45);
        case BASE_PUERTA_CERRADA: return Color(160, 100,  35);
        case BASE_PUERTA_ABIERTA: return Color(210, 160,  90);
        default:                  return Color(255,   0, 255);
    }
}

Color Mapa::colorProp(int fila, int col) const {
    switch (capaProps[fila][col]) {
        case PROP_LAMPARA:  return Color(255, 220,  50, 255);
        case PROP_ANTORCHA: return Color(255, 140,  30, 255);
        default:            return Color(  0,   0,   0,   0);  // invisible
    }
}

Color Mapa::colorTrampa(int fila, int col) const {
    if (capaTrampas[fila][col] == TRAMPA_NADA) return Color(0, 0, 0, 0);
    return Color(220, 40, 40, 255);
}

Color Mapa::colorEnemigo(int fila, int col) const {
    if (capaEnemigos[fila][col] == ENEMIGO_NADA) return Color(0, 0, 0, 0);
    return Color(40, 210, 60, 255);
}

Color Mapa::colorLlave(int fila, int col) const {
    if (capaLlaves[fila][col] == LLAVE_NADA) return Color(0, 0, 0, 0);
    return Color(240, 220, 40, 255);
}
