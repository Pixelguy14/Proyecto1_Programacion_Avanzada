#include "Fruit.h"

#include "entities/FruitHalf.h"
#include "graphics/Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

/**
 * @file Fruit.cpp
 * @brief Implementación de la fruta: física, rotación, estados y corte.
 */

namespace {

/// Margen bajo la pantalla antes de eliminar una fruta que ya cayó.
constexpr float kKillMargin = 80.0f;

/// Separación inicial de cada mitad respecto al centro de la fruta (× radio).
constexpr float kHalfOffsetFactor = 0.14f;

/// Velocidad con la que las mitades se separan, en píxeles/segundo.
constexpr float kHalfSeparationSpeed = 170.0f;

/// Giro extra que diferencia a las dos mitades, en radianes/segundo.
constexpr float kHalfSpinSpeed = 2.4f;

/// Inclinación fija para que la malla 3D muestre su volumen.
constexpr float kMeshTiltDegrees = 12.0f;

}  // namespace

Fruit::Fruit(const Vector2D<float>& position, const Vector2D<float>& velocity, float radius,
             float angularVelocity, FruitMeshId meshId)
    : m_radius(radius),
      m_angularVelocity(angularVelocity),
      m_meshId(meshId),
      m_color(fruitAppearance(meshId).shell),
      m_fleshColor(fruitAppearance(meshId).flesh) {
    m_body.position = position;
    m_body.velocity = velocity;

    if (meshId == FruitMeshId::kBomb) {
        addFlag(FruitFlag::kBomb);
    }
}

void Fruit::update(float dt) {
    Physics2D::integrate(m_body, Physics2D::kGravity, dt);
    Physics2D::integrateRotation(m_angle, m_angularVelocity, dt);

    if (hasFallenOffScreen()) {
        m_alive = false;
    }
}

void Fruit::render(Renderer& renderer) {
    const glm::vec2 center{m_body.position.x, m_body.position.y};
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));

    // Las frutas giran sobre Y para mostrar sus facetas; el plátano y la bomba
    // giran sobre Z para no perder su silueta (la mecha se balancea).
    if (isBomb() || m_meshId == FruitMeshId::kBanana) {
        model = glm::rotate(model, m_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    } else {
        model = glm::rotate(model, m_angle, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    model = glm::rotate(model, glm::radians(kMeshTiltDegrees),
                        glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(m_radius));

    renderer.drawMesh(m_meshId, model);
}

bool Fruit::isAlive() const {
    // m_alive cubre la caída fuera de pantalla; kActive cubre el corte.
    return m_alive && hasFlag(FruitFlag::kActive);
}

void Fruit::markSliced() {
    addFlag(FruitFlag::kSliced);
    removeFlag(FruitFlag::kActive);
}

std::pair<std::unique_ptr<FruitHalf>, std::unique_ptr<FruitHalf>>
Fruit::slice(const Vector2D<float>& cutDirection) {
    // Solo se corta una fruta activa que no sea bomba.
    if (!hasFlag(FruitFlag::kActive) || hasFlag(FruitFlag::kBomb)) {
        return {nullptr, nullptr};
    }

    // El tajo define el diámetro de las mitades; la separación es perpendicular.
    Vector2D<float> direction = cutDirection.normalized();
    if (direction.lengthSquared() <= 0.0f) {
        direction = Vector2D<float>{1.0f, 0.0f};  // corte horizontal por defecto
    }

    const Vector2D<float> separation = direction.perpendicular();
    const float diameterAngle = std::atan2(direction.y, direction.x);
    const float offset = m_radius * kHalfOffsetFactor;

    auto first = std::make_unique<FruitHalf>(
        m_body.position + separation * offset,
        m_body.velocity + separation * kHalfSeparationSpeed,
        m_radius, diameterAngle, m_angularVelocity + kHalfSpinSpeed, m_color, m_fleshColor,
        m_meshId);

    // La segunda mitad es la misma geometría girada 180°.
    auto second = std::make_unique<FruitHalf>(
        m_body.position - separation * offset,
        m_body.velocity - separation * kHalfSeparationSpeed,
        m_radius, diameterAngle + 3.14159265358979323846f,
        m_angularVelocity - kHalfSpinSpeed, m_color, m_fleshColor, m_meshId);

    markSliced();
    return {std::move(first), std::move(second)};
}

bool Fruit::hasFallenOffScreen() const {
    return m_body.velocity.y < 0.0f
        && m_body.position.y + m_radius < -kKillMargin;
}
