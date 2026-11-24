#include "FruitHalf.h"

#include "graphics/Renderer.h"

/**
 * @file FruitHalf.cpp
 * @brief Implementación de la física y el dibujado de las mitades de fruta.
 */

namespace {

/// Margen bajo la pantalla antes de eliminar una mitad que ya cayó.
constexpr float kKillMargin = 80.0f;

/// Fracción del radio que ocupa la pulpa (semicírculo interior más claro).
constexpr float kFleshRadiusFactor = 0.82f;

/// Cantidad que se aclara el color de la pulpa respecto a la cáscara.
constexpr float kFleshLighten = 0.35f;

/**
 * @brief Aclara un color hacia el blanco.
 * @param color Color RGB en [0, 1].
 * @param amount Fracción de aclarado [0, 1].
 * @return Color aclarado.
 */
glm::vec3 lighten(const glm::vec3& color, float amount) {
    return color + (glm::vec3{1.0f} - color) * amount;
}

}  // namespace

FruitHalf::FruitHalf(const Vector2D<float>& position, const Vector2D<float>& velocity,
                     float radius, float angle, float angularVelocity,
                     const glm::vec3& color)
    : m_radius(radius),
      m_angle(angle),
      m_angularVelocity(angularVelocity),
      m_color(color) {
    m_body.position = position;
    m_body.velocity = velocity;
}

void FruitHalf::update(float dt) {
    Physics2D::integrate(m_body, Physics2D::kGravity, dt);
    Physics2D::integrateRotation(m_angle, m_angularVelocity, dt);

    if (hasFallenOffScreen()) {
        m_alive = false;
    }
}

void FruitHalf::render(Renderer& renderer) {
    const glm::vec2 center{m_body.position.x, m_body.position.y};

    // Cáscara: el semicírculo completo en el color original.
    renderer.drawSemicircle(center, m_radius, m_angle, m_color);

    // Pulpa: un semicírculo más pequeño y claro, concéntrico.
    renderer.drawSemicircle(center, m_radius * kFleshRadiusFactor, m_angle,
                            lighten(m_color, kFleshLighten));
}

bool FruitHalf::hasFallenOffScreen() const {
    return m_body.velocity.y < 0.0f
        && m_body.position.y + m_radius < -kKillMargin;
}
