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

/// Colores de la bomba (Sprint 4, parte 2).
const glm::vec3 kBombRimColor{0.62f, 0.14f, 0.12f};
const glm::vec3 kBombBodyColor{0.07f, 0.07f, 0.09f};
const glm::vec3 kBombSheenColor{0.34f, 0.35f, 0.40f};
const glm::vec3 kBombFuseColor{0.46f, 0.31f, 0.16f};
const glm::vec3 kBombSparkColor{1.00f, 0.70f, 0.18f};
const glm::vec3 kBombSparkCoreColor{1.00f, 0.96f, 0.80f};

/// Geometría de la bomba, como fracción del radio.
constexpr float kBombRimScale = 1.06f;
constexpr float kBombBodyScale = 0.97f;
constexpr float kBombSheenOffsetFactor = 0.32f;
constexpr float kBombSheenRadiusFactor = 0.22f;
constexpr float kBombFuseThicknessFactor = 0.10f;
constexpr float kBombFuseLengthFactor = 0.42f;
constexpr float kBombFuseDistanceFactor = 0.95f;
constexpr float kBombSparkDistanceFactor = 1.20f;
constexpr float kBombSparkRadiusFactor = 0.18f;
constexpr float kBombSparkCoreScale = 0.55f;

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
    if (hasFlag(FruitFlag::kBomb)) {
        renderBomb(renderer);
        return;
    }

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

void Fruit::renderBomb(Renderer& renderer) {
    const glm::vec2 center{m_body.position.x, m_body.position.y};
    const Vector2D<float> direction = stemOffset().normalized();

    // Borde rojo y cuerpo oscuro: el anillo hace que la bomba se distinga de
    // una fruta incluso cuando el fondo es claro.
    renderer.drawCircle(center, m_radius * kBombRimScale, kBombRimColor);
    renderer.drawCircle(center, m_radius * kBombBodyScale, kBombBodyColor);

    // Brillo desplazado hacia arriba-izquierda (luz fija de la escena).
    const glm::vec2 sheenCenter{
        center.x - m_radius * kBombSheenOffsetFactor,
        center.y + m_radius * kBombSheenOffsetFactor,
    };
    renderer.drawCircle(sheenCenter, m_radius * kBombSheenRadiusFactor,
                        kBombSheenColor, 0.55f);

    // Mecha orientada con la rotación de la fruta.
    const glm::vec2 fuseCenter{
        m_body.position.x + direction.x * m_radius * kBombFuseDistanceFactor,
        m_body.position.y + direction.y * m_radius * kBombFuseDistanceFactor,
    };
    renderer.drawRect(fuseCenter,
                      glm::vec2{m_radius * kBombFuseThicknessFactor,
                                m_radius * kBombFuseLengthFactor},
                      m_angle, kBombFuseColor);

    // Chispa encendida en la punta de la mecha.
    const glm::vec2 sparkCenter{
        m_body.position.x + direction.x * m_radius * kBombSparkDistanceFactor,
        m_body.position.y + direction.y * m_radius * kBombSparkDistanceFactor,
    };
    renderer.drawCircle(sparkCenter, m_radius * kBombSparkRadiusFactor, kBombSparkColor);
    renderer.drawCircle(sparkCenter,
                        m_radius * kBombSparkRadiusFactor * kBombSparkCoreScale,
                        kBombSparkCoreColor);
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
