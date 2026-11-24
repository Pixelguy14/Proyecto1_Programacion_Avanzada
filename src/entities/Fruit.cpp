#include "Fruit.h"

#include "graphics/Renderer.h"

#include <cmath>

/**
 * @file Fruit.cpp
 * @brief Implementación de la fruta: física, rotación y dibujado.
 */

namespace {

/// Color del tallo (verde oscuro).
const glm::vec3 kStemColor{0.32f, 0.48f, 0.20f};

/// Margen bajo la pantalla antes de eliminar una fruta que ya cayó.
constexpr float kKillMargin = 80.0f;

/// El tallo se dibuja a esta fracción del radio desde el centro.
constexpr float kStemOffsetFactor = 0.90f;

/// Tamaño del tallo relativo al radio de la fruta.
constexpr float kStemThicknessFactor = 0.14f;
constexpr float kStemLengthFactor = 0.55f;

}  // namespace

Fruit::Fruit(const glm::vec2& position, const glm::vec2& velocity, float radius,
             float angularVelocity, const glm::vec3& color)
    : m_radius(radius),
      m_angularVelocity(angularVelocity),
      m_color(color) {
    m_body.position = position;
    m_body.velocity = velocity;
}

void Fruit::update(float dt) {
    Physics2D::integrate(m_body, Physics2D::kGravity, dt);
    Physics2D::integrateRotation(m_angle, m_angularVelocity, dt);

    if (hasFallenOffScreen()) {
        m_alive = false;
    }
}

void Fruit::render(Renderer& renderer) {
    renderer.drawCircle(m_body.position, m_radius, m_color);

    // El tallo hace visible la rotación (un círculo solo no la mostraría).
    const glm::vec2 stemCenter = m_body.position + stemOffset();
    const glm::vec2 stemSize{
        m_radius * kStemThicknessFactor,
        m_radius * kStemLengthFactor,
    };
    renderer.drawRect(stemCenter, stemSize, m_angle, kStemColor);
}

bool Fruit::hasFallenOffScreen() const {
    return m_body.velocity.y < 0.0f
        && m_body.position.y + m_radius < -kKillMargin;
}

glm::vec2 Fruit::stemOffset() const {
    // Rotar el vector (0, longitud) por m_angle alrededor del centro.
    const float length = m_radius * kStemOffsetFactor;
    return glm::vec2{
        -length * std::sin(m_angle),
         length * std::cos(m_angle),
    };
}
