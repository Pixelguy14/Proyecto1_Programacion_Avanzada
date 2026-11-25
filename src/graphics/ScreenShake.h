#pragma once

#include <glm/glm.hpp>

#include <random>

/**
 * @file ScreenShake.h
 * @brief Temblor de cámara para dar impacto a cortes y explosiones (Sprint 5).
 *
 * El temblor es un desplazamiento 2D que se aplica a la proyección del
 * `Renderer` mientras se dibuja la escena de juego. La magnitud decae de forma
 * lineal y el desplazamiento cambia cada frame en una dirección aleatoria, lo
 * que produce un temblor corto y natural sin acumular estado entre partidas.
 *
 *   GameSession
 *     ├── trigger(3.5)   al cortar una fruta
 *     ├── trigger(24)    al cortar una bomba
 *     └── update(dt)
 *           └── offset() → Renderer::setViewOffset()
 */
class ScreenShake {
public:
    ScreenShake();

    /// @brief Detiene el temblor y centra la cámara.
    void reset();

    /**
     * @brief Añade un impulso a la magnitud actual.
     * @param magnitude Desplazamiento máximo en píxeles (valores <= 0 se ignoran).
     *
     * Los impulsos se acumulan hasta un tope, de modo que una explosión pequeña
     * no queda tapada por un corte, pero una bomba siempre se siente.
     */
    void trigger(float magnitude);

    /**
     * @brief Decae la magnitud y calcula el desplazamiento de este frame.
     * @param dt Delta time en segundos.
     */
    void update(float dt);

    /// @return Desplazamiento actual de la cámara en píxeles.
    const glm::vec2& offset() const { return m_offset; }

    /// @return Magnitud restante del temblor en píxeles.
    float magnitude() const { return m_magnitude; }

private:
    std::mt19937 m_rng;          ///< Generador del jitter.
    glm::vec2 m_offset{0.0f};    ///< Desplazamiento del frame actual.
    float m_magnitude = 0.0f;    ///< Amplitud restante en píxeles.
};
