#include "ScreenShake.h"

#include <algorithm>
#include <chrono>

/**
 * @file ScreenShake.cpp
 * @brief Implementación del decaimiento y el jitter del temblor de cámara.
 */

namespace {

/// Amplitud máxima acumulable, en píxeles.
constexpr float kMaxMagnitude = 32.0f;

/// Velocidad de decaimiento, en píxeles por segundo.
constexpr float kDecayPerSecond = 70.0f;

/// Por debajo de esta amplitud se considera terminado (evita jitter residual).
constexpr float kMinMagnitude = 0.08f;

}  // namespace

ScreenShake::ScreenShake() {
    const auto seed = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    m_rng.seed(seed);
}

void ScreenShake::reset() {
    m_magnitude = 0.0f;
    m_offset = glm::vec2{0.0f};
}

void ScreenShake::trigger(float magnitude) {
    if (magnitude <= 0.0f) {
        return;
    }
    m_magnitude = std::min(m_magnitude + magnitude, kMaxMagnitude);
}

void ScreenShake::update(float dt) {
    if (m_magnitude <= kMinMagnitude) {
        reset();
        return;
    }

    m_magnitude = std::max(0.0f, m_magnitude - kDecayPerSecond * dt);

    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    m_offset = glm::vec2{
        distribution(m_rng) * m_magnitude,
        distribution(m_rng) * m_magnitude,
    };
}
