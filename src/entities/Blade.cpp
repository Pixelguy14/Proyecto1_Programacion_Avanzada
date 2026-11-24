#include "Blade.h"

#include "core/Input.h"
#include "graphics/Renderer.h"

#include <algorithm>
#include <vector>

/**
 * @file Blade.cpp
 * @brief Implementación del historial y la estela de la espada.
 */

namespace {

/// Máximo de puntos guardados: suficiente para una estela fluida y barata.
constexpr std::size_t kMaxPoints = 24;

/// Tiempo de vida de un punto, en segundos.
constexpr float kPointLifetime = 0.18f;

/// Distancia mínima entre puntos para no acumular posiciones repetidas.
constexpr float kMinDistance = 2.0f;

/// Grosor de la estela en los extremos (antiguo → reciente), en píxeles.
constexpr float kMinWidth = 2.5f;
constexpr float kMaxWidth = 14.0f;

/// Color de la estela (azul hielo semitransparente).
const glm::vec4 kTrailColor{0.80f, 0.92f, 1.0f, 0.95f};

/// Punta de la espada, visible incluso cuando el mouse está quieto.
constexpr float kTipRadius = 5.0f;
const glm::vec3 kTipColor{0.95f, 0.98f, 1.0f};

}  // namespace

Blade::Blade(const Input& input) : m_input(input) {}

void Blade::update(float dt) {
    for (auto& point : m_trail) {
        point.age += dt;
    }

    while (!m_trail.empty() && m_trail.front().age > kPointLifetime) {
        m_trail.pop_front();
    }

    const Vector2D<float> current = m_input.mousePosition();
    const bool hasMoved = m_trail.empty()
        || current.distanceTo(m_trail.back().position) >= kMinDistance;

    if (!hasMoved) {
        return;
    }

    m_trail.push_back(TrailPoint{current, 0.0f});
    while (m_trail.size() > kMaxPoints) {
        m_trail.pop_front();
    }
}

void Blade::render(Renderer& renderer) {
    std::vector<Renderer::TrailPoint> trailPoints;
    trailPoints.reserve(m_trail.size());

    for (const auto& point : m_trail) {
        // 1 = recién creado, 0 = a punto de desaparecer.
        const float freshness = std::clamp(1.0f - point.age / kPointLifetime, 0.0f, 1.0f);
        const float width = kMinWidth + (kMaxWidth - kMinWidth) * freshness;

        trailPoints.push_back(Renderer::TrailPoint{
            glm::vec2{point.position.x, point.position.y},
            width,
            freshness,
        });
    }

    renderer.drawTrail(trailPoints, kTrailColor);

    const Vector2D<float> mouse = m_input.mousePosition();
    renderer.drawCircle(glm::vec2{mouse.x, mouse.y}, kTipRadius, kTipColor);
}
