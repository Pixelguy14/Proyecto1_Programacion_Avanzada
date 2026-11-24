#pragma once

/**
 * @file Physics2D.h
 * @brief Integración de Euler semi-implícita y gravedad para el Sprint 1.
 *
 * ## Jerarquía
 *   Fruit::update()
 *     └── Physics2D::integrate()          (traduce aceleración → posición)
 *
 * `PhysicsBody` solo guarda el estado cinemático; no sabe dibujar ni conoce
 * la pantalla. Así la física se puede reutilizar en Fruit, FruitHalf, etc.
 *
 * ## Integración de Euler semi-implícita
 * Se actualiza primero la velocidad y después la posición con la velocidad ya
 * actualizada (ver *The Nature of Code*, cap. 2.6):
 * @code
 *   velocity += acceleration * dt;
 *   position += velocity * dt;
 * @endcode
 * Es estable con dt fijos razonables y es el método que pide el enunciado.
 */

#include <glm/glm.hpp>

/**
 * @brief Estado cinemático de un cuerpo puntual en 2D.
 */
struct PhysicsBody {
    glm::vec2 position{0.0f};      ///< Posición en píxeles de pantalla.
    glm::vec2 velocity{0.0f};      ///< Velocidad en píxeles/segundo.
    glm::vec2 acceleration{0.0f};  ///< Aceleración en píxeles/segundo².
};

namespace Physics2D {

/// Gravedad del juego en píxeles/s² (Y positiva hacia arriba, como la pantalla).
inline const glm::vec2 kGravity{0.0f, -980.0f};

/**
 * @brief Aplica una aceleración constante y avanza el cuerpo un paso de tiempo.
 * @param body         Cuerpo a integrar (se modifica in-place).
 * @param acceleration Aceleración externa, normalmente Physics2D::kGravity.
 * @param dt           Delta time en segundos.
 */
void integrate(PhysicsBody& body, const glm::vec2& acceleration, float dt);

/**
 * @brief Avanza el ángulo de un cuerpo con velocidad angular constante.
 * @param angle           Ángulo en radianes (se modifica in-place).
 * @param angularVelocity Velocidad angular en radianes/segundo.
 * @param dt              Delta time en segundos.
 *
 * El ángulo se mantiene en (-2π, 2π) para no perder precisión en sesiones largas.
 */
void integrateRotation(float& angle, float angularVelocity, float dt);

}  // namespace Physics2D
