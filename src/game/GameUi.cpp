#include "GameUi.h"

#include "core/AssetPaths.h"
#include "graphics/Renderer.h"

#include <glm/glm.hpp>

#include <iostream>
#include <string>

/**
 * @file GameUi.cpp
 * @brief Composición de textos y overlays por estado.
 */

namespace {

/// Colores de la interfaz.
const glm::vec4 kCream{0.98f, 0.94f, 0.84f, 1.0f};
const glm::vec4 kGold{0.95f, 0.78f, 0.35f, 1.0f};
const glm::vec4 kRed{0.86f, 0.22f, 0.18f, 1.0f};
const glm::vec4 kShadow{0.02f, 0.02f, 0.03f, 0.85f};

/// Color del velo que oscurece el fondo.
const glm::vec3 kOverlayColor{0.02f, 0.02f, 0.03f};

/// Desplazamiento de la sombra del texto, en píxeles.
const glm::vec2 kShadowOffset{4.0f, -4.0f};

/// Escalas de texto habituales (la fuente se rasteriza a 64 px).
constexpr float kTitleScale = 2.6f;
constexpr float kSubtitleScale = 0.95f;
constexpr float kInstructionScale = 0.80f;
constexpr float kHudScale = 0.75f;

/**
 * @brief Dibuja un velo semitransparente a pantalla completa.
 * @param renderer Renderer compartido.
 * @param width    Ancho de la pantalla.
 * @param height   Alto de la pantalla.
 * @param alpha    Opacidad del velo [0, 1].
 */
void drawOverlay(Renderer& renderer, int width, int height, float alpha) {
    const glm::vec2 center{static_cast<float>(width) * 0.5f,
                           static_cast<float>(height) * 0.5f};
    const glm::vec2 size{static_cast<float>(width), static_cast<float>(height)};
    renderer.drawRect(center, size, 0.0f, kOverlayColor, alpha);
}

}  // namespace

bool GameUi::init(int screenWidth, int screenHeight) {
    m_width = screenWidth;
    m_height = screenHeight;

    const std::string fontPath = AssetPaths::resolve("fonts/ZenAntique-Regular.ttf");
    if (!m_text.init(fontPath, screenWidth, screenHeight)) {
        std::cerr << "[GameUi] No se pudo cargar la tipografia: " << fontPath << std::endl;
        return false;
    }
    return true;
}

void GameUi::cleanup() {
    m_text.cleanup();
}

void GameUi::drawCentered(const std::string& text, const glm::vec2& center, float scale,
                          const glm::vec4& color) const {
    if (!m_text.ready()) {
        return;
    }

    const float width = m_text.textWidth(text, scale);
    const glm::vec2 topLeft{
        center.x - width * 0.5f,
        center.y + m_text.ascent(scale) * 0.5f,
    };
    m_text.drawText(text, topLeft, scale, color);
}

void GameUi::drawCenteredWithShadow(const std::string& text, const glm::vec2& center,
                                    float scale, const glm::vec4& color) const {
    drawCentered(text, center + kShadowOffset, scale, kShadow);
    drawCentered(text, center, scale, color);
}

void GameUi::renderHud(Renderer& renderer, int score, int lives) const {
    (void)renderer;
    if (!m_text.ready()) {
        return;
    }

    const float margin = 52.0f;
    const float top = static_cast<float>(m_height) - 48.0f;

    const std::string scoreText = "PUNTOS  " + std::to_string(score);
    m_text.drawText(scoreText, glm::vec2{margin, top}, kHudScale, kCream);

    const std::string livesText = "VIDAS  " + std::to_string(lives);
    const float livesWidth = m_text.textWidth(livesText, kHudScale);
    const glm::vec4 livesColor = lives > 1 ? kCream : kRed;
    m_text.drawText(livesText,
                    glm::vec2{static_cast<float>(m_width) - margin - livesWidth, top},
                    kHudScale, livesColor);
}

void GameUi::renderMenu(Renderer& renderer) const {
    drawOverlay(renderer, m_width, m_height, 0.30f);

    const float centerX = static_cast<float>(m_width) * 0.5f;

    drawCenteredWithShadow("FRUIT NINJA", glm::vec2{centerX, 720.0f}, kTitleScale, kCream);

    // Línea decorativa bajo el título.
    renderer.drawRect(glm::vec2{centerX, 610.0f}, glm::vec2{560.0f, 4.0f}, 0.0f,
                      glm::vec3{kRed});

    drawCenteredWithShadow("Corta las frutas con el mouse", glm::vec2{centerX, 540.0f},
                           kSubtitleScale, kGold);

    drawCenteredWithShadow("EVITA LAS BOMBAS", glm::vec2{centerX, 470.0f},
                           kSubtitleScale, kRed);

    drawCenteredWithShadow("ENTER / ESPACIO   -   JUGAR", glm::vec2{centerX, 400.0f},
                           kInstructionScale, kCream);
    drawCenteredWithShadow("ESC   -   SALIR", glm::vec2{centerX, 330.0f},
                           kInstructionScale, kCream);
}

void GameUi::renderPaused(Renderer& renderer) const {
    drawOverlay(renderer, m_width, m_height, 0.62f);

    const float centerX = static_cast<float>(m_width) * 0.5f;
    drawCenteredWithShadow("PAUSA", glm::vec2{centerX, 660.0f}, 2.2f, kCream);

    drawCenteredWithShadow("P / ESC   -   CONTINUAR", glm::vec2{centerX, 520.0f},
                           kInstructionScale, kCream);
    drawCenteredWithShadow("R   -   REINICIAR", glm::vec2{centerX, 450.0f},
                           kInstructionScale, kCream);
    drawCenteredWithShadow("M   -   MENU", glm::vec2{centerX, 380.0f},
                           kInstructionScale, kCream);
}

void GameUi::renderGameOver(Renderer& renderer, int score, bool bombExploded) const {
    drawOverlay(renderer, m_width, m_height, 0.68f);

    const float centerX = static_cast<float>(m_width) * 0.5f;
    drawCenteredWithShadow("GAME OVER", glm::vec2{centerX, 730.0f}, 2.4f, kRed);

    if (bombExploded) {
        drawCenteredWithShadow("CORTASTE UNA BOMBA", glm::vec2{centerX, 605.0f},
                               kSubtitleScale, kGold);
    }

    const std::string scoreText = "PUNTOS  " + std::to_string(score);
    drawCenteredWithShadow(scoreText, glm::vec2{centerX, 510.0f}, 1.15f, kGold);

    drawCenteredWithShadow("ENTER / ESPACIO   -   JUGAR DE NUEVO",
                           glm::vec2{centerX, 380.0f}, kInstructionScale, kCream);
    drawCenteredWithShadow("ESC   -   MENU", glm::vec2{centerX, 310.0f},
                           kInstructionScale, kCream);
}
