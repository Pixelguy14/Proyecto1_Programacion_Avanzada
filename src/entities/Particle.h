#pragma once

#include "core/GameObject.h"
#include "math/Vector2D.h"
#include "physics/Physics2D.h"

#include <glm/glm.hpp>

class Renderer;

/**
 * @file Particle.h
 * @brief Partícula visual de jugo o chispa (Sprint 4, parte 2).
 *
 * Una `Particle` es un pequeño círculo con física propia que se desvanece y
 * encoge con el tiempo. No participa en las reglas de la partida: no suma
 * puntos, no resta vidas y no se puede cortar. Solo decora el impacto de un
 * corte o la explosión de una bomba.
 *
 *   Fruit::slice() / bomba
 *     └── GameSession::emitJuice() / emitExplosion()
 *           └── Particle (varias instancias)
 *                 ├── Physics2D::integrate()
 *                 └── Renderer::drawCircle() con alpha
 *
 * La gravedad y el arrastre se escalan por partícula para que el jugo caiga
 * con un arco suave y las chispas de la bomba salgan más rápido y se frenen
 * antes.
 */
class Particle : public GameObject {
public:
    /**
     * @brief Crea una partícula lista para entrar en la lista de la sesión.
     * @param position     Posición inicial en píxeles de pantalla.
     * @param velocity     Velocidad inicial en píxeles/segundo.
     * @param radius       Radio inicial en píxeles.
     * @param lifetime     Vida total en segundos (debe ser positiva).
     * @param color        Color RGB en el rango [0, 1].
     * @param gravityScale Fracción de la gravedad del juego que se aplica.
     * @param drag         Coeficiente de arrastre lineal (0 = sin frenado).
     */
    Particle(const Vector2D<float>& position, const Vector2D<float>& velocity,
             float radius, float lifetime, const glm::vec3& color,
             float gravityScale = 1.0f, float drag = 0.0f);

    /**
     * @brief Avanza la edad, aplica gravedad/arrastre e integra la posición.
     * @param dt Delta time en segundos.
     *
     * Cuando la edad supera la vida, la partícula se marca como no viva y la
     * sesión la elimina.
     */
    void update(float dt) override;

    /**
     * @brief Dibuja la partícula con alpha y radio decrecientes.
     * @param renderer Renderer compartido.
     */
    void render(Renderer& renderer) override;

    /**
     * @brief Fracción de vida consumida.
     * @return 0 recién creada, 1 a punto de desaparecer.
     */
    float lifeRatio() const;

    /// @return Centro actual en píxeles de pantalla.
    const Vector2D<float>& position() const { return m_body.position; }

private:
    PhysicsBody m_body;         ///< Estado cinemático (posición/velocidad).
    float m_radius;             ///< Radio inicial en píxeles.
    float m_lifetime;           ///< Vida total en segundos.
    float m_age = 0.0f;         ///< Segundos transcurridos.
    float m_gravityScale;       ///< Fracción de la gravedad aplicada.
    float m_drag;               ///< Coeficiente de arrastre lineal.
    glm::vec3 m_color;          ///< Color RGB base.
};
