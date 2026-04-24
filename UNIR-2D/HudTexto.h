
// Text drawable for the HUD that renders in screen-space (default SFML view).
// This bypasses the camera zoom entirely so text is always crisp regardless of zoom level.
// Use ponPosicionPantalla(sx, sy) with actual pixel coordinates.
// Use ponTamano() with the desired on-screen pixel size directly (no zoom division needed).

#pragma once

namespace unir2d {

class HudTexto : public Dibujable {
public:
    explicit HudTexto(sf::Font* fuente) {
        m_texto.setFont(*fuente);
    }

    // Screen-space position (pixels, top-left origin). Use this instead of ponPosicion().
    void ponPosicionPantalla(float sx, float sy) { m_sx = sx; m_sy = sy; }

    void ponCadena(const std::string& s)  { m_texto.setString(s); }
    void ponTamano(int sz)                { m_texto.setCharacterSize(static_cast<unsigned>(std::max(1, sz))); }
    void ponColor(Color c)                { m_texto.setFillColor(sf::Color{ c.entero() }); }

private:
    sf::Text m_texto;
    float    m_sx = 0.0f;
    float    m_sy = 0.0f;

    void dibuja(const Transforma& /*contenedor*/, Rendidor* rendidor) override {
        // Switch to the unzoomed default view so the text renders at its exact pixel size.
        sf::View camView = rendidor->window->getView();
        rendidor->window->setView(rendidor->window->getDefaultView());

        m_texto.setPosition(m_sx, m_sy);
        rendidor->window->draw(m_texto);

        // Restore the camera view for subsequent drawables.
        rendidor->window->setView(camView);
    }
};

} // namespace unir2d
