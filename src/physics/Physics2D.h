#pragma once

/**
 * @file Physics2D.h
 * @brief Integración de Euler semi-implícita y gravedad.
 *
 * ## Jerarquía
 *   Fruit::update() / FruitHalf::update()
 *     └── Physics2D::integrate()          (traduce aceleración → posición)
 *
 * `PhysicsBody` solo guarda el estado cinemático; no sabe dibujar ni conoce
 * la pantalla. Usa `Vector2D<float>` (la clase del proyecto) para que toda la
 * simulación comparta el mismo tipo de vector; GLM queda para el renderer.
 *
 * ## Integración de Euler semi-implícita
 * Se actualiza primero la velocidad y después la posición con la velocidad ya
 * actualizada (ver *The Nature of Code*, cap. 2.6):
 * @code
 *   velocity += acceleration * dt;
 *   position += velocity * dt;
 * @endcode
 */

#include "math/Vector2D.h"

/**
 * @brief Estado cinemático de un cuerpo puntual en 2D.
 */
struct PhysicsBody {
    Vector2D<float> position{};      ///< Posición en píxeles de pantalla.
    Vector2D<float> velocity{};      ///< Velocidad en píxeles/segundo.
    Vector2D<float> acceleration{};  ///< Aceleración en píxeles/segundo².
};

namespace Physics2D {

/// Gravedad del juego en píxeles/s² (Y positiva hacia arriba, como la pantalla).
inline const Vector2D<float> kGravity{0.0f, -980.0f};

/**
 * @brief Aplica una aceleración constante y avanza el cuerpo un paso de tiempo.
 * @param body         Cuerpo a integrar (se modifica in-place).
 * @param acceleration Aceleración externa, normalmente Physics2D::kGravity.
 * @param dt           Delta time en segundos.
 */
void integrate(PhysicsBody& body, const Vector2D<float>& acceleration, float dt);

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
