#include "Fruit.h"

#include "entities/FruitHalf.h"
#include "graphics/Renderer.h"

#include <cmath>

/**
 * @file Fruit.cpp
 * @brief Implementación de la fruta: física, rotación, estados y corte.
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

/// Separación inicial de cada mitad respecto al centro de la fruta (× radio).
constexpr float kHalfOffsetFactor = 0.14f;

/// Velocidad con la que las mitades se separan, en píxeles/segundo.
constexpr float kHalfSeparationSpeed = 170.0f;

/// Giro extra que diferencia a las dos mitades, en radianes/segundo.
constexpr float kHalfSpinSpeed = 2.4f;

}  // namespace

Fruit::Fruit(const Vector2D<float>& position, const Vector2D<float>& velocity, float radius,
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
    const glm::vec2 center{m_body.position.x, m_body.position.y};
    renderer.drawCircle(center, m_radius, m_color);

    // El tallo hace visible la rotación (un círculo solo no la mostraría).
    const Vector2D<float> offset = stemOffset();
    const glm::vec2 stemCenter{
        m_body.position.x + offset.x,
        m_body.position.y + offset.y,
    };
    const glm::vec2 stemSize{
        m_radius * kStemThicknessFactor,
        m_radius * kStemLengthFactor,
    };
    renderer.drawRect(stemCenter, stemSize, m_angle, kStemColor);
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
        m_radius, diameterAngle, m_angularVelocity + kHalfSpinSpeed, m_color);

    // La segunda mitad es la misma geometría girada 180°.
    auto second = std::make_unique<FruitHalf>(
        m_body.position - separation * offset,
        m_body.velocity - separation * kHalfSeparationSpeed,
        m_radius, diameterAngle + 3.14159265358979323846f,
        m_angularVelocity - kHalfSpinSpeed, m_color);

    markSliced();
    return {std::move(first), std::move(second)};
}

bool Fruit::hasFallenOffScreen() const {
    return m_body.velocity.y < 0.0f
        && m_body.position.y + m_radius < -kKillMargin;
}

Vector2D<float> Fruit::stemOffset() const {
    // Rotar el vector (0, longitud) por m_angle alrededor del centro.
    const float length = m_radius * kStemOffsetFactor;
    return Vector2D<float>{
        -length * std::sin(m_angle),
        length * std::cos(m_angle),
    };
}
