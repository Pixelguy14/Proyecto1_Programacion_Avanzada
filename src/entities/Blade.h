#pragma once

#include "core/GameObject.h"
#include "math/Vector2D.h"

#include <deque>

class Input;
class Renderer;

/**
 * @file Blade.h
 * @brief Espada del jugador: historial de posiciones y estela (Sprint 2).
 *
 * Blade no dibuja directamente: guarda los últimos puntos del recorrido del
 * mouse y los convierte en una cinta con grosor y alpha decrecientes. La
 * geometría de la cinta vive en el Renderer.
 *
 *   Input (callbacks GLFW)
 *     └── Blade::update()   historial en std::deque
 *           └── Blade::render() → Renderer::drawTrail()
 *
 * Los puntos caducan por tiempo (no solo por cantidad), así que al detener el
 * mouse la estela se desvanece sola.
 */
class Blade : public GameObject {
public:
    /**
     * @brief Crea la espada asociada a una fuente de entrada.
     * @param input Entrada de la que se lee la posición del mouse.
     *
     * La referencia debe sobrevivir a la Blade.
     */
    explicit Blade(const Input& input);

    /**
     * @brief Envejece la estela y añade la posición actual si hubo movimiento.
     * @param dt Delta time en segundos.
     */
    void update(float dt) override;

    /**
     * @brief Dibuja la estela y la punta de la espada.
     * @param renderer Renderer compartido.
     */
    void render(Renderer& renderer) override;

private:
    /// Punto del historial con su antigüedad.
    struct TrailPoint {
        Vector2D<float> position;  ///< Posición en coordenadas del juego.
        float age = 0.0f;          ///< Segundos desde que se registró.
    };

private:
    const Input& m_input;       ///< Fuente de la posición del mouse.
    std::deque<TrailPoint> m_trail;  ///< Del punto más antiguo al más reciente.
};
