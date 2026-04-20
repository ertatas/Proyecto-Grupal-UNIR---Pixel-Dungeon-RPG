#pragma once
#include "UNIR-2D.h"
#include <vector>
#include <string>

// ============================================================
// CLASE MAPA
// ============================================================
// Capas lógicas:
//
//   capaBase     → suelo, pared, puerta cerrada/abierta
//   capaProps    → decoración (lámpara, antorcha…) – solo visual
//   capaTrampas  → trampas
//   capaEnemigos → spawn de enemigos
//   capaLlaves   → llaves
//
// Las texturas son 64×64 por defecto (tamCasilla).
// El editor (EditorMapa) usa la API pública para pintar tiles
// y guardar / cargar el mapa desde un fichero de texto.
// ============================================================
class Mapa : public unir2d::ActorBase {
public:
    // ========================================================
    // TIPOS PÚBLICOS
    // ========================================================
    // Los artistas y el editor solo necesitan estos valores.

    enum TipoBase {
        BASE_SUELO          = 0,
        BASE_PARED          = 1,
        BASE_PUERTA_CERRADA = 2,
        BASE_PUERTA_ABIERTA = 3
    };

    enum TipoProp {
        // Props de PARED (valores 1-9): se colocan sobre BASE_PARED, offset visual hacia arriba.
        PROP_NADA      = 0,
        PROP_LAMPARA   = 1,
        PROP_ANTORCHA  = 2,
        // Props de SUELO (valores 10-19): se colocan sobre BASE_SUELO, centrados en el tile.
        PROP_CHARCO    = 10,
        PROP_MANCHA    = 11,
        PROP_HUESOS    = 12
    };

    // Helpers para distinguir tipo de prop por rango de valor.
    static bool esPropDePared(int t) { return t >= 1 && t <= 9;  }
    static bool esPropDeSuelo(int t) { return t >= 10 && t <= 19; }

    // --------------------------------------------------------
    // VARIANTES VISUALES
    // --------------------------------------------------------
    // Cada tile puede tener una variante visual independiente
    // de su tipo lógico. Afecta al color placeholder y, cuando
    // se añadan PNGs, al sprite que se dibuja.
    // El editor puede autodetectar la variante (detectarVariante)
    // o permitir sobreescritura manual.
    enum VarianteVisual {
        VAR_DEFAULT  = 0,   // sin orientación definida
        VAR_NORTE    = 1,   // pared cuya cara visible mira al sur  (suelo al norte)
        VAR_SUR      = 2,   // pared cuya cara visible mira al norte (suelo al sur)
        VAR_ESTE     = 3,   // pared cuya cara visible mira al oeste (suelo al este)
        VAR_OESTE    = 4,   // pared cuya cara visible mira al este  (suelo al oeste)
        VAR_PUERTA_H = 5,   // puerta horizontal (jugador la cruza moviéndose E/O)
        VAR_PUERTA_V = 6,   // puerta vertical   (jugador la cruza moviéndose N/S)
    };

    enum TipoTrampaPublic {
        TRAMPA_PUBLICA_NADA = 0,
        TRAMPA_PUBLICA_FIJA = 1
    };

    enum TipoEnemigoPublic {
        ENEMIGO_PUBLICO_NADA   = 0,
        ENEMIGO_PUBLICO_NORMAL = 1
    };

    enum TipoLlavePublic {
        LLAVE_PUBLICA_NADA   = 0,
        LLAVE_PUBLICA_NORMAL = 1
    };

    enum TipoVisibilidad {
        VIS_OCULTO  = 0,   // Negro absoluto — zona no explorada, no se dibuja nada
        VIS_VISIBLE = 1    // Se dibuja todo con normalidad
    };

    // ========================================================
    // CICLO DE VIDA
    // ========================================================
    Mapa();
    ~Mapa();

    void inicia()  override;
    void termina() override;

    // ========================================================
    // API PÚBLICA – consulta
    // ========================================================
    int  numFilas()    const { return filas; }
    int  numColumnas() const { return columnas; }

    int  getBase    (int fila, int col) const;
    int  getProp    (int fila, int col) const;
    int  getLlave   (int fila, int col) const;
    int  getVariante(int fila, int col) const;

    int  getSpawnFila() const { return spawnFila; }
    int  getSpawnCol()  const { return spawnCol;  }

    // Devuelve true si la casilla es pisable (sin pared/puerta/enemigo).
    bool esTransitable(int fila, int col) const;

    // Esquina superior izquierda de una casilla en coordenadas mundo.
    unir2d::Vector centroCasilla(int fila, int col) const;

    float tamanoCasilla() const { return tamCasilla; }

    // ========================================================
    // API PÚBLICA – modificación (usada por el editor)
    // ========================================================
    void setBase    (int fila, int col, int tipo);
    void setProp    (int fila, int col, int tipo);
    void setTrampa  (int fila, int col, int tipo);
    void setEnemigo (int fila, int col, int tipo);
    void setLlave   (int fila, int col, int tipo);
    void setSpawn   (int fila, int col);
    // Establece la variante visual de un tile y actualiza su drawable.
    void setVariante(int fila, int col, int variante);

    // Analiza los vecinos del tile y devuelve la variante más apropiada
    // para una pared o puerta. El editor la llama al pintar; el diseñador
    // puede sobreescribirla después desde la capa VARIANTES.
    VarianteVisual detectarVariante(int fila, int col) const;

    // Revela la zona conectada desde la posición del jugador (BFS).
    // Se expande por suelo y paredes; se detiene en puertas (abiertas o cerradas).
    // Llamar cada vez que el jugador se mueve y al iniciar la partida.
    void revelarZonaDesde(int fila, int col);

    // Revela todo el mapa de golpe (usado por el editor al abrirse).
    void revelarTodo();

    // Oculta todo el mapa (resetea el fog). Llamar al cerrar el editor
    // y antes de re-revelar desde la posición del jugador.
    void resetearFog();

    // ========================================================
    // PERSISTENCIA
    // ========================================================
    // Guarda y carga usan siempre mapa.txt en la carpeta del exe.
    // El parámetro se ignora (se mantiene por compatibilidad).
    void guardarMapa(const std::string& ruta = "") const;
    void cargarMapa (const std::string& ruta = "");

    // Activa/desactiva el modo editor: en modo editor se usan placeholders de color
    // para facilitar la edición (las variantes se distinguen por tono).
    void ponModoEditor(bool valor) {
        if (enModoEditor == valor) return;
        enModoEditor = valor;
        if (!dibujosBase.empty()) refrescarDibujos();
    }

private:
    // ========================================================
    // ENUMS INTERNOS
    // ========================================================
    enum TipoTrampa  { TRAMPA_NADA = 0,   TRAMPA_FIJA    = 1 };
    enum TipoEnemigo { ENEMIGO_NADA = 0,  ENEMIGO_NORMAL = 1 };
    enum TipoLlave   { LLAVE_NADA = 0,    LLAVE_NORMAL   = 1 };

    // ========================================================
    // COORDENADAS VISUALES (variantes por vecindad)
    // ========================================================
    struct CoordenadasVisuales {
        int baseX = 0, baseY = 0;
        int trampaX = 0, trampaY = 0;
        int enemigoX = 0, enemigoY = 0;
        int llaveX = 0, llaveY = 0;
    };

    // ========================================================
    // DATOS
    // ========================================================
    int   filas     = 0;
    int   columnas  = 0;
    const float tamCasilla = 64.0f;   // ← texturas deben ser múltiplo de esto

    std::vector<std::vector<int>> capaBase;
    std::vector<std::vector<int>> capaProps;
    std::vector<std::vector<int>> capaTrampas;
    std::vector<std::vector<int>> capaEnemigos;
    std::vector<std::vector<int>> capaLlaves;
    std::vector<std::vector<int>> capaVisibilidad;   // VIS_OCULTO / VIS_VISIBLE
    std::vector<std::vector<int>> capaVariantes;     // VarianteVisual por tile
    std::vector<std::vector<CoordenadasVisuales>> capaVisual;

    bool puertasAbiertasPorDefecto = false;

    int  spawnFila = 1;
    int  spawnCol  = 1;

    // ========================================================
    // DRAWABLES
    // ========================================================
    std::vector<unir2d::Rectangulo*> dibujosBase;
    std::vector<unir2d::Rectangulo*> dibujosTrampas;
    std::vector<unir2d::Rectangulo*> dibujosEnemigos;
    std::vector<unir2d::Rectangulo*> dibujosLlaves;
    std::vector<unir2d::Rectangulo*> dibujosProps;    // props de PARED (índiceZ=2)
    std::vector<unir2d::Rectangulo*> dibujosPropsS;   // props de SUELO (índiceZ=1)

    // Imagen por tile para mostrar el PNG de pared (oculta si no hay textura o no es pared)
    std::vector<unir2d::Imagen*>     dibujosBaseImg;

    // Texturas de pared direccionales — cargadas una vez en inicia(), nullptr si no se encontraron
    unir2d::Textura* texParedSur     = nullptr;
    unir2d::Textura* texParedNorte   = nullptr;
    unir2d::Textura* texParedEste    = nullptr;
    unir2d::Textura* texParedOeste   = nullptr;
    unir2d::Textura* texParedDefault = nullptr;
    bool texturasParedCargadas       = false;
    bool enModoEditor                = false;

    // ========================================================
    // CONSTRUCCIÓN INTERNA
    // ========================================================
    void crearCapas(int nuevasFilas, int nuevasColumnas);
    void construirMapaInicial();
    void cargarMapaDesdeArchivo(const std::string& ruta);  // carga sin reconstruir drawables
    void calcularCoordenadasVisuales();

    void crearDibujos();
    void refrescarDibujos();
    void limpiarListasDibujos();

    // ========================================================
    // UTILIDADES
    // ========================================================
    bool dentroRango(int fila, int col) const;
    int  tipoPuertaPorDefecto() const;
    bool esPared(int fila, int col) const;

    unir2d::Color colorBase      (int fila, int col) const;  // usa capaVariantes
    unir2d::Color colorPropPared (int fila, int col) const;  // props de pared (1-9)
    unir2d::Color colorPropSuelo (int fila, int col) const;  // props de suelo (10-19)
    unir2d::Color colorTrampa    (int fila, int col) const;
    unir2d::Color colorEnemigo   (int fila, int col) const;
    unir2d::Color colorLlave     (int fila, int col) const;
};
