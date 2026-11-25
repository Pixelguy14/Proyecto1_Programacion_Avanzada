#include "Particle.h"

#include "graphics/Renderer.h"

#include <algorithm>

/**
 * @file Particle.cpp
 * @brief Implementación de la física y el desvanecido de las partículas.
 */

namespace {

/// Fracción del radio que conserva la partícula al final de su vida.
constexpr float kEndRadiusFactor = 0.55f;

/// Límite inferior de vida para evitar divisiones por cero.
constexpr float kMinLifetime = 1e-3f;

}  // namespace

Particle::Particle(const Vector2D<float>& position, const Vector2D<float>& velocity,
                   float radius, float lifetime, const glm::vec3& color,
                   float gravityScale, float drag)
    : m_radius(radius),
      m_lifetime(std::max(lifetime, kMinLifetime)),
      m_gravityScale(gravityScale),
      m_drag(std::max(drag, 0.0f)),
      m_color(color) {
    m_body.position = position;
    m_body.velocity = velocity;
}

void Particle::update(float dt) {
    m_age += dt;
    if (m_age >= m_lifetime) {
        m_alive = false;
        return;
    }

    // Gravedad reducida y arrastre exponencial aproximado por frame.
    Physics2D::integrate(m_body, Physics2D::kGravity * m_gravityScale, dt);
    if (m_drag > 0.0f) {
        const float damping = std::max(0.0f, 1.0f - m_drag * dt);
        m_body.velocity *= damping;
    }
}

void Particle::render(Renderer& renderer) {
    const float t = lifeRatio();
    const float alpha = 1.0f - t;
    const float radius = m_radius * (1.0f - (1.0f - kEndRadiusFactor) * t);

    renderer.drawCircle(glm::vec2{m_body.position.x, m_body.position.y}, radius,
                        m_color, alpha);
}

float Particle::lifeRatio() const {
    return std::clamp(m_age / m_lifetime, 0.0f, 1.0f);
}
