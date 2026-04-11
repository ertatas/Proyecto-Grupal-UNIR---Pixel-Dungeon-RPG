Fuentes TrueType para el HUD (consola de mensajes).

El motor UNIR-2D busca las fuentes en esta carpeta: {directorio_trabajo}/fuentes/

PARA ACTIVAR EL TEXTO EN LA CONSOLA:
  Copiar desde UNIR-2D/fuentes/ a esta carpeta:
    DejaVuSans.ttf

Si el archivo no existe aqui, los paneles del HUD aparecen igualmente
pero sin texto (degradacion silenciosa, sin crash).

Tamanho de render usado en codigo: 12 px en pantalla.
Archivo de fuente usado en codigo: "DejaVuSans"  (DejaVuSans.ttf)

Para cambiar la fuente: editar Hud.cpp, linea con new Texto("DejaVuSans").
