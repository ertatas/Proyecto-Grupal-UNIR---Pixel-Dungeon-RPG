PNGs de tiles base (suelo y paredes).
Tamaño: 64x64 px.

Archivos esperados:
  suelo.png          -- tile de suelo genérico
  pared_default.png  -- pared sin orientación definida (VAR_DEFAULT)
  pared_norte.png    -- pared orientada al norte (VAR_NORTE: suelo al sur)
  pared_sur.png      -- pared orientada al sur   (VAR_SUR:   suelo al norte)
  pared_este.png     -- pared orientada al este  (VAR_ESTE:  suelo al este)
  pared_oeste.png    -- pared orientada al oeste (VAR_OESTE: suelo al oeste)

Una vez entregados, el programador busca TODO_ARTISTA en Mapa.cpp
y sustituye ponColor(...) por la carga de la textura correspondiente.
