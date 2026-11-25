#pragma once

#include "graphics/TextRenderer.h"

class HighScoreTable;
class Renderer;

/**
 * @file GameUi.h
 * @brief Textos, overlays y récords de los estados de juego (Sprints 4–5).
 *
 * GameUi no conoce la lógica de la partida: recibe el puntaje, las vidas y la
 * tabla de récords, y solo decide qué texto dibujar. Los overlays son
 * rectángulos semitransparentes que oscurecen el fondo para que el texto se lea.
 *
 *   main (estado)
 *     └── GameUi
 *           ├── TextRenderer     (Zen Antique)
 *           ├── Renderer         (overlays con alpha)
 *           └── HighScoreTable   (top 10 en menú y game over)
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
     * @brief Dibuja el título, las instrucciones y el top 10 del menú.
     * @param renderer   Renderer compartido.
     * @param highScores Tabla de récords a mostrar en la esquina superior derecha.
     */
    void renderMenu(Renderer& renderer, const HighScoreTable& highScores) const;

    /**
     * @brief Dibuja el overlay de pausa.
     * @param renderer Renderer compartido.
     */
    void renderPaused(Renderer& renderer) const;

    /**
     * @brief Dibuja la entrada de nombre para un nuevo récord.
     * @param renderer Renderer compartido.
     * @param score    Puntaje conseguido.
     * @param name     Nombre tecleado hasta ahora (puede estar vacío).
     */
    void renderNameEntry(Renderer& renderer, int score, const std::string& name) const;

    /**
     * @brief Dibuja el resultado final y el top 10.
     * @param renderer      Renderer compartido.
     * @param score         Puntaje con el que terminó la partida.
     * @param bombExploded  true si el jugador cortó una bomba.
     * @param highScores    Tabla de récords.
     * @param highlightRank Fila a resaltar (0–9), o -1 si no hay ninguna.
     */
    void renderGameOver(Renderer& renderer, int score, bool bombExploded,
                        const HighScoreTable& highScores, int highlightRank = -1) const;

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

    /**
     * @brief Dibuja el top 10 con las columnas de nombre y puntaje alineadas.
     * @param highScores    Tabla de récords.
     * @param anchorX       Borde derecho (si `rightAligned`) o centro del bloque.
     * @param topY          Y del primer renglón.
     * @param scale         Escala del texto.
     * @param lineHeight    Separación vertical entre renglones.
     * @param highlightRank Fila resaltada (0–9), o -1.
     * @param rightAligned  true para alinear a la derecha (menú).
     */
    void renderScoreTable(const HighScoreTable& highScores, float anchorX, float topY,
                          float scale, float lineHeight, int highlightRank,
                          bool rightAligned) const;

private:
    TextRenderer m_text;        ///< Atlas y shader de la tipografía.
    int m_width = 0;            ///< Ancho lógico de la pantalla.
    int m_height = 0;           ///< Alto lógico de la pantalla.
};
