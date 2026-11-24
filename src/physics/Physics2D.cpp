#include "Physics2D.h"

#include <cmath>

/**
 * @file Physics2D.cpp
 * @brief Implementación de la integración de Euler semi-implícita.
 */

namespace {
/// 2π, usado para mantener el ángulo dentro de un rango manejable.
constexpr float kTwoPi = 6.28318530718f;
}  // namespace

void Physics2D::integrate(PhysicsBody& body, const Vector2D<float>& acceleration, float dt) {
    body.acceleration = acceleration;

    // Semi-implícito: la velocidad se actualiza antes que la posición.
    body.velocity += body.acceleration * dt;
    body.position += body.velocity * dt;
}

void Physics2D::integrateRotation(float& angle, float angularVelocity, float dt) {
    angle = std::fmod(angle + angularVelocity * dt, kTwoPi);
}
