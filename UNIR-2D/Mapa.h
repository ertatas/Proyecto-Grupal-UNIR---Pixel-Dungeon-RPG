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
        PROP_NADA      = 0,
        PROP_LAMPARA   = 1,
        PROP_ANTORCHA  = 2
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

    int  getBase(int fila, int col) const;
    int  getProp(int fila, int col) const;

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
    void setBase   (int fila, int col, int tipo);
    void setProp   (int fila, int col, int tipo);
    void setTrampa (int fila, int col, int tipo);
    void setEnemigo(int fila, int col, int tipo);
    void setLlave  (int fila, int col, int tipo);
    void setSpawn  (int fila, int col);

    // Revela la zona conectada desde la posición del jugador (BFS).
    // Se expande por suelo y paredes; se detiene en puertas (abiertas o cerradas).
    // Llamar cada vez que el jugador se mueve y al iniciar la partida.
    void revelarZonaDesde(int fila, int col);

    // Revela todo el mapa de golpe (usado por el editor al abrirse).
    void revelarTodo();

    // ========================================================
    // PERSISTENCIA
    // ========================================================
    // Guarda y carga usan siempre mapa.txt en la carpeta del exe.
    // El parámetro se ignora (se mantiene por compatibilidad).
    void guardarMapa(const std::string& ruta = "") const;
    void cargarMapa (const std::string& ruta = "");

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
    std::vector<std::vector<int>> capaVisibilidad;   // VIS_OCULTO / VIS_RECORDADO / VIS_VISIBLE
    std::vector<std::vector<CoordenadasVisuales>> capaVisual;

    bool puertasAbiertasPorDefecto = false;

    int  spawnFila = 1;
    int  spawnCol  = 1;

    // ========================================================
    // DRAWABLES
    // ========================================================
    std::vector<unir2d::Rectangulo*> dibujosBase;
    std::vector<unir2d::Rectangulo*> dibujosProps;
    std::vector<unir2d::Rectangulo*> dibujosTrampas;
    std::vector<unir2d::Rectangulo*> dibujosEnemigos;
    std::vector<unir2d::Rectangulo*> dibujosLlaves;

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

    unir2d::Color colorBase   (int fila, int col) const;
    unir2d::Color colorProp   (int fila, int col) const;
    unir2d::Color colorTrampa (int fila, int col) const;
    unir2d::Color colorEnemigo(int fila, int col) const;
    unir2d::Color colorLlave  (int fila, int col) const;
};
