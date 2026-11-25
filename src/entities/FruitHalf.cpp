#include "FruitHalf.h"

#include "graphics/Renderer.h"

#include <algorithm>
#include <cmath>
#include <vector>

/**
 * @file FruitHalf.cpp
 * @brief Física y dibujado 2D de las mitades de fruta.
 *
 * La forma de la mitad ya no es un semicírculo genérico: cada fruta tiene una
 * silueta propia (el plátano es alargado y curvado, el limón estirado, etc.).
 * La silueta se dibuja con `Renderer::drawPolygon`, con una capa de cáscara y
 * otra de pulpa más pequeña.
 */

namespace {

/// Margen bajo la pantalla antes de eliminar una mitad que ya cayó.
constexpr float kKillMargin = 80.0f;

/// Fracción del radio que ocupa la pulpa respecto a la cáscara.
constexpr float kFleshScale = 0.82f;

/// Segmentos del arco de la silueta.
constexpr int kShapeSegments = 16;

/// Parámetros de la silueta de una fruta.
struct HalfShapeParams {
    float aspectX;  ///< Longitud a lo largo del corte.
    float aspectY;  ///< Altura del cuerpo de la mitad.
    float exponent; ///< >1 hace las puntas más afiladas.
    float skew;     ///< Asimetría suave (plátano).
};

/**
 * @brief Devuelve los parámetros de silueta de cada tipo de fruta.
 * @param id Tipo de fruta.
 * @return Aspectos y exponente de la mitad.
 */
HalfShapeParams halfShapeParams(FruitMeshId id) {
    switch (id) {
        case FruitMeshId::kMelon:
            return {1.00f, 1.10f, 1.00f, 0.00f};
        case FruitMeshId::kApple:
            return {0.95f, 0.95f, 1.00f, 0.00f};
        case FruitMeshId::kOrange:
            return {1.00f, 0.95f, 1.00f, 0.00f};
        case FruitMeshId::kLemon:
            return {1.30f, 0.85f, 1.15f, 0.00f};
        case FruitMeshId::kBanana:
            return {1.45f, 0.62f, 1.65f, 0.12f};
        case FruitMeshId::kBomb:
            return {1.00f, 1.00f, 1.00f, 0.00f};
    }
    return {1.00f, 1.00f, 1.00f, 0.00f};
}

/**
 * @brief Genera la silueta de una mitad en coordenadas locales.
 * @param id    Tipo de fruta.
 * @param scale Escala de la silueta (1 = cáscara, 0.82 = pulpa).
 * @return Contorno del arco; el borde plano es la recta que cierra el polígono.
 */
std::vector<glm::vec2> makeHalfShape(FruitMeshId id, float scale) {
    const HalfShapeParams params = halfShapeParams(id);
    std::vector<glm::vec2> points;
    points.reserve(kShapeSegments + 1);

    for (int i = 0; i <= kShapeSegments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kShapeSegments);
        const float angle = 3.14159265358979323846f * t;
        // Cerca de π el seno puede quedar ligeramente negativo por el redondeo
        // de float; se recorta para que pow() no genere NaN en las puntas.
        const float sinAngle = std::max(0.0f, std::sin(angle));
        const float x = std::cos(angle) * params.aspectX;
        const float y = std::pow(sinAngle, params.exponent) * params.aspectY
                      * (1.0f + params.skew * std::cos(angle));
        points.push_back(glm::vec2{x * scale, y * scale});
    }

    return points;
}

}  // namespace

FruitHalf::FruitHalf(const Vector2D<float>& position, const Vector2D<float>& velocity,
                     float radius, float angle, float angularVelocity,
                     const glm::vec3& shellColor, const glm::vec3& fleshColor,
                     FruitMeshId meshId)
    : m_radius(radius),
      m_angle(angle),
      m_angularVelocity(angularVelocity),
      m_shellColor(shellColor),
      m_fleshColor(fleshColor),
      m_meshId(meshId) {
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
    const std::vector<glm::vec2> shell = makeHalfShape(m_meshId, 1.0f);
    const std::vector<glm::vec2> flesh = makeHalfShape(m_meshId, kFleshScale);

    renderer.drawPolygon(shell, center, m_angle, m_radius, m_shellColor);
    renderer.drawPolygon(flesh, center, m_angle, m_radius, m_fleshColor);
}

bool FruitHalf::hasFallenOffScreen() const {
    return m_body.velocity.y < 0.0f
        && m_body.position.y + m_radius < -kKillMargin;
}
