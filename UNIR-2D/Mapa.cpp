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

int Mapa::getLlave(int fila, int col) const {
    if (!dentroRango(fila, col)) return LLAVE_NADA;
    return capaLlaves[fila][col];
}

int Mapa::getVariante(int fila, int col) const {
    if (!dentroRango(fila, col)) return VAR_DEFAULT;
    return capaVariantes[fila][col];
}

void Mapa::setVariante(int fila, int col, int variante) {
    if (!dentroRango(fila, col)) return;
    capaVariantes[fila][col] = variante;
    // Actualizar solo el drawable base de este tile (evita refrescar todo el mapa).
    int i = fila * columnas + col;
    if (!dibujosBase.empty()) {
        dibujosBase[i]->ponColor(colorBase(fila, col));
    }
}

Mapa::VarianteVisual Mapa::detectarVariante(int fila, int col) const {
    if (!dentroRango(fila, col)) return VAR_DEFAULT;

    int tipo = capaBase[fila][col];

    if (tipo == BASE_PUERTA_CERRADA || tipo == BASE_PUERTA_ABIERTA) {
        // Puerta horizontal: tiene suelo/paso al este u oeste
        bool pasoE = dentroRango(fila, col+1) && capaBase[fila][col+1] != BASE_PARED;
        bool pasoO = dentroRango(fila, col-1) && capaBase[fila][col-1] != BASE_PARED;
        bool pasoN = dentroRango(fila-1, col) && capaBase[fila-1][col] != BASE_PARED;
        bool pasoS = dentroRango(fila+1, col) && capaBase[fila+1][col] != BASE_PARED;
        if (pasoE || pasoO) return VAR_PUERTA_H;
        if (pasoN || pasoS) return VAR_PUERTA_V;
        return VAR_DEFAULT;
    }

    if (tipo == BASE_PARED) {
        // La cara visible de la pared es opuesta al lado donde hay suelo.
        // Prioridad N/S sobre E/O en caso de ambigüedad (esquina).
        bool sueloN = dentroRango(fila-1, col) && capaBase[fila-1][col] == BASE_SUELO;
        bool sueloS = dentroRango(fila+1, col) && capaBase[fila+1][col] == BASE_SUELO;
        bool sueloE = dentroRango(fila, col+1) && capaBase[fila][col+1] == BASE_SUELO;
        bool sueloO = dentroRango(fila, col-1) && capaBase[fila][col-1] == BASE_SUELO;
        if (sueloN) return VAR_SUR;    // suelo al norte → cara de la pared mira al sur
        if (sueloS) return VAR_NORTE;  // suelo al sur   → cara mira al norte
        if (sueloE) return VAR_OESTE;
        if (sueloO) return VAR_ESTE;
        return VAR_DEFAULT;
    }

    return VAR_DEFAULT;
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
    static const Color transparente(0, 0, 0, 0);
    if (esPropDePared(tipo)) {
        dibujosProps [i]->ponColor(colorPropPared(fila, col));
        dibujosPropsS[i]->ponColor(transparente);
    } else if (esPropDeSuelo(tipo)) {
        dibujosProps [i]->ponColor(transparente);
        dibujosPropsS[i]->ponColor(colorPropSuelo(fila, col));
    } else {
        // PROP_NADA
        dibujosProps [i]->ponColor(transparente);
        dibujosPropsS[i]->ponColor(transparente);
    }
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

    f << "DUNGEON_MAP 2\n";
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

    escribeCapa("BASE",      capaBase);
    escribeCapa("PROPS",     capaProps);
    escribeCapa("TRAMPAS",   capaTrampas);
    escribeCapa("ENEMIGOS",  capaEnemigos);
    escribeCapa("LLAVES",    capaLlaves);
    escribeCapa("VARIANTES", capaVariantes);
    f << "SPAWN\n" << spawnFila << " " << spawnCol << "\n";
    f << "END\n";
}

void Mapa::cargarMapaDesdeArchivo(const std::string& ruta) {
    std::ifstream f(ruta);
    if (!f.is_open()) return;

    std::string linea;

    // Cabecera: "DUNGEON_MAP <version>"
    std::getline(f, linea);
    int version = 1;
    {
        std::istringstream ss(linea);
        std::string magic;
        ss >> magic >> version;  // magic = "DUNGEON_MAP"
    }

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

    if (version >= 2) {
        // v2: cargar capaVariantes desde el fichero
        leeCapa(capaVariantes);
    } else {
        // v1 → retrocompatibilidad: auto-detectar variantes a partir de la estructura del mapa
        for (int fr = 0; fr < filas; fr++)
            for (int fc = 0; fc < columnas; fc++)
                capaVariantes[fr][fc] = static_cast<int>(detectarVariante(fr, fc));
    }

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
    capaVariantes   .assign(filas, std::vector<int>(columnas, VAR_DEFAULT));
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

void Mapa::resetearFog() {
    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < columnas; c++) {
            capaVisibilidad[f][c] = VIS_OCULTO;
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
    dibujosPropsS  .reserve(total);

    // Prop de PARED (lámpara/antorcha): 16×24 px, centrado horizontalmente, pegado arriba.
    const float ppW = tamCasilla * 0.25f;
    const float ppH = tamCasilla * 0.38f;

    // Prop de SUELO (charco/mancha/huesos): 48×48 px, centrado en el tile.
    const float psW = tamCasilla * 0.75f;
    const float psH = tamCasilla * 0.75f;

    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < columnas; c++) {
            float tx = c * tamCasilla;
            float ty = f * tamCasilla;

            // [0] BASE (tile completo, 64×64)
            Rectangulo* rBase = new Rectangulo(tamCasilla, tamCasilla);
            rBase->ponPosicion(Vector(tx, ty));
            rBase->ponIndiceZ(0);
            agregaDibujo(rBase);
            dibujosBase.push_back(rBase);

            // [1] TRAMPA (14×14, centrado)
            Rectangulo* rTrampa = new Rectangulo(tamCasilla * 0.22f, tamCasilla * 0.22f);
            rTrampa->ponPosicion(Vector(tx + tamCasilla * 0.39f, ty + tamCasilla * 0.39f));
            rTrampa->ponIndiceZ(1);
            agregaDibujo(rTrampa);
            dibujosTrampas.push_back(rTrampa);

            // [2] ENEMIGO (33×33, centrado)
            Rectangulo* rEnemigo = new Rectangulo(tamCasilla * 0.52f, tamCasilla * 0.52f);
            rEnemigo->ponPosicion(Vector(tx + tamCasilla * 0.24f, ty + tamCasilla * 0.24f));
            rEnemigo->ponIndiceZ(1);
            agregaDibujo(rEnemigo);
            dibujosEnemigos.push_back(rEnemigo);

            // [3] LLAVE (12×12, arriba-centro)
            Rectangulo* rLlave = new Rectangulo(tamCasilla * 0.18f, tamCasilla * 0.18f);
            rLlave->ponPosicion(Vector(tx + tamCasilla * 0.41f, ty + tamCasilla * 0.12f));
            rLlave->ponIndiceZ(1);
            agregaDibujo(rLlave);
            dibujosLlaves.push_back(rLlave);

            // [4] PROP PARED (16×24, centrado horizontalmente, pegado arriba)
            // TODO_ARTISTA: reemplazar ponColor(...) por textura PNG:
            //   imagen.ponTextura(textura); imagen.ponPosicion(x, y);
            //   Ruta: assets/textures/tiles/props/pared/lampara.png o antorcha.png
            //   Ver GUIA_EDITOR_MAPA.md sección "Guía para artistas"
            Rectangulo* rPropP = new Rectangulo(ppW, ppH);
            rPropP->ponPosicion(Vector(tx + (tamCasilla - ppW) * 0.5f, ty + tamCasilla * 0.06f));
            rPropP->ponIndiceZ(2);
            agregaDibujo(rPropP);
            dibujosProps.push_back(rPropP);

            // [5] PROP SUELO (48×48, centrado en el tile)
            // TODO_ARTISTA: reemplazar ponColor(...) por textura PNG:
            //   imagen.ponTextura(textura); imagen.ponPosicion(x, y);
            //   Ruta: assets/textures/tiles/props/suelo/charco.png, mancha.png, huesos.png
            //   Ver GUIA_EDITOR_MAPA.md sección "Guía para artistas"
            Rectangulo* rPropS = new Rectangulo(psW, psH);
            rPropS->ponPosicion(Vector(tx + (tamCasilla - psW) * 0.5f, ty + (tamCasilla - psH) * 0.5f));
            rPropS->ponIndiceZ(1);
            agregaDibujo(rPropS);
            dibujosPropsS.push_back(rPropS);
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
                // Zona no explorada: negro absoluto, el resto invisible.
                dibujosBase[i]    ->ponColor(negroOpaco);
                dibujosTrampas[i] ->ponColor(transparente);
                dibujosEnemigos[i]->ponColor(transparente);
                dibujosLlaves[i]  ->ponColor(transparente);
                dibujosProps[i]   ->ponColor(transparente);
                dibujosPropsS[i]  ->ponColor(transparente);
            } else {
                // Zona visible: colores normales según tipo y variante.
                dibujosBase[i]    ->ponColor(colorBase   (f, c));
                dibujosTrampas[i] ->ponColor(colorTrampa (f, c));
                dibujosEnemigos[i]->ponColor(colorEnemigo(f, c));
                dibujosLlaves[i]  ->ponColor(colorLlave  (f, c));

                // Props: solo se muestra el drawable que corresponde al tipo de prop.
                int prop = capaProps[f][c];
                if (esPropDePared(prop)) {
                    dibujosProps [i]->ponColor(colorPropPared(f, c));
                    dibujosPropsS[i]->ponColor(transparente);
                } else if (esPropDeSuelo(prop)) {
                    dibujosProps [i]->ponColor(transparente);
                    dibujosPropsS[i]->ponColor(colorPropSuelo(f, c));
                } else {
                    dibujosProps [i]->ponColor(transparente);
                    dibujosPropsS[i]->ponColor(transparente);
                }
            }
        }
    }
}

void Mapa::limpiarListasDibujos() {
    dibujosBase    .clear();
    dibujosTrampas .clear();
    dibujosEnemigos.clear();
    dibujosLlaves  .clear();
    dibujosProps   .clear();
    dibujosPropsS  .clear();
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
    // TODO_ARTISTA: reemplazar cada ponColor por textura PNG según tipo y variante.
    //   Ruta base: assets/textures/tiles/base/
    //   Archivos: suelo.png, pared_default.png, pared_norte.png, etc.
    //   Ver GUIA_EDITOR_MAPA.md sección "Guía para artistas"
    int base = capaBase[fila][col];
    int var  = capaVariantes[fila][col];

    switch (base) {
        case BASE_SUELO:
            return Color(90, 90, 90);
        case BASE_PARED:
            switch (var) {
                case VAR_NORTE:   return Color( 80,  80, 120);   // gris azulado
                case VAR_SUR:     return Color(120,  80,  80);   // gris rojizo
                case VAR_ESTE:    return Color( 80, 120,  80);   // gris verdoso
                case VAR_OESTE:   return Color(120, 120,  80);   // gris amarillento
                default:          return Color( 80,  80,  80);   // VAR_DEFAULT gris medio
            }
        case BASE_PUERTA_CERRADA:
            if (var == VAR_PUERTA_H) return Color(139,  90,  43);   // marrón claro
            else                     return Color(100,  60,  20);   // marrón oscuro (V)
        case BASE_PUERTA_ABIERTA:
            if (var == VAR_PUERTA_H) return Color(210, 160,  90);
            else                     return Color(180, 130,  65);
        default:
            return Color(255, 0, 255);
    }
}

Color Mapa::colorPropPared(int fila, int col) const {
    // TODO_ARTISTA: reemplazar por textura PNG.
    //   Ruta: assets/textures/tiles/props/pared/
    //   Archivos: lampara.png (16×24 px), antorcha.png (16×24 px)
    switch (capaProps[fila][col]) {
        case PROP_LAMPARA:  return Color(255, 220,  50, 255);
        case PROP_ANTORCHA: return Color(255, 140,  30, 255);
        default:            return Color(  0,   0,   0,   0);
    }
}

Color Mapa::colorPropSuelo(int fila, int col) const {
    // TODO_ARTISTA: reemplazar por textura PNG.
    //   Ruta: assets/textures/tiles/props/suelo/
    //   Archivos: charco.png, mancha.png, huesos.png (48×48 px)
    switch (capaProps[fila][col]) {
        case PROP_CHARCO:  return Color( 20,  40, 100, 180);   // azul oscuro semitransparente
        case PROP_MANCHA:  return Color( 60,  30,  10, 200);   // marrón oscuro
        case PROP_HUESOS:  return Color(220, 210, 180, 220);   // blanco hueso
        default:           return Color(  0,   0,   0,   0);
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
