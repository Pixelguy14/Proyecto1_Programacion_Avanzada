#pragma once

#include "graphics/TextRenderer.h"

class Renderer;

/**
 * @file GameUi.h
 * @brief Textos y overlays de los cuatro estados de juego (Sprint 4).
 *
 * GameUi no conoce la lógica de la partida: recibe el puntaje y las vidas como
 * parámetros y solo decide qué texto dibujar. Los overlays son rectángulos
 * semitransparentes que oscurecen el fondo para que el texto se lea.
 *
 *   main (estado)
 *     └── GameUi
 *           ├── TextRenderer  (Zen Antique)
 *           └── Renderer      (overlays con alpha)
 */
class GameUi {
public:
    GameUi() = default;
    ~GameUi() = default;

    GameUi(const GameUi&) = delete;
    GameUi& operator=(const GameUi&) = delete;

    /**
     * @brief Carga la fuente y prepara el renderer de texto.
     * @param screenWidth  Ancho lógico de la pantalla.
     * @param screenHeight Alto lógico de la pantalla.
     * @return true si la tipografía quedó lista.
     */
    bool init(int screenWidth, int screenHeight);

    /**
     * @brief Libera los recursos de texto.
     */
    void cleanup();

    /**
     * @brief Dibuja el marcador de puntos y vidas.
     * @param renderer Renderer compartido.
     * @param score    Puntaje actual.
     * @param lives    Vidas restantes.
     */
    void renderHud(Renderer& renderer, int score, int lives) const;

    /**
     * @brief Dibuja el título y las instrucciones del menú.
     * @param renderer Renderer compartido.
     */
    void renderMenu(Renderer& renderer) const;

    /**
     * @brief Dibuja el overlay de pausa.
     * @param renderer Renderer compartido.
     */
    void renderPaused(Renderer& renderer) const;

    /**
     * @brief Dibuja el resultado final de la partida.
     * @param renderer      Renderer compartido.
     * @param score         Puntaje con el que terminó la partida.
     * @param bombExploded  true si el jugador cortó una bomba.
     */
    void renderGameOver(Renderer& renderer, int score, bool bombExploded) const;

private:
    /**
     * @brief Dibuja un texto centrado horizontal y verticalmente.
     * @param text   Texto de una sola línea.
     * @param center Centro del texto en píxeles.
     * @param scale  Escala respecto al tamaño rasterizado.
     * @param color  Color RGBA.
     */
    void drawCentered(const std::string& text, const glm::vec2& center, float scale,
                      const glm::vec4& color) const;

    /**
     * @brief Dibuja texto con una sombra desplazada para mejorar el contraste.
     * @param text   Texto de una sola línea.
     * @param center Centro del texto en píxeles.
     * @param scale  Escala respecto al tamaño rasterizado.
     * @param color  Color RGBA del texto.
     */
    void drawCenteredWithShadow(const std::string& text, const glm::vec2& center, float scale,
                                const glm::vec4& color) const;

private:
    TextRenderer m_text;        ///< Atlas y shader de la tipografía.
    int m_width = 0;            ///< Ancho lógico de la pantalla.
    int m_height = 0;           ///< Alto lógico de la pantalla.
};
