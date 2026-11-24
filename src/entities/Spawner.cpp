#include "Spawner.h"

#include <chrono>
#include <cmath>

/**
 * @file Spawner.cpp
 * @brief Implementación de la generación aleatoria de frutas.
 */

namespace {

/// Intervalo entre lanzamientos, en segundos.
constexpr float kMinSpawnInterval = 0.55f;
constexpr float kMaxSpawnInterval = 1.15f;

/// Radio de la fruta como fracción de la altura de la pantalla.
constexpr float kMinRadiusFactor = 0.035f;
constexpr float kMaxRadiusFactor = 0.065f;

/// Altura máxima del vuelo como fracción de la altura de la pantalla.
constexpr float kMinApexFactor = 0.45f;
constexpr float kMaxApexFactor = 0.80f;

/// Velocidad horizontal máxima como fracción del ancho de la pantalla.
constexpr float kMaxHorizontalSpeedFactor = 0.10f;

/// Velocidad angular aleatoria, en radianes/segundo.
constexpr float kMinAngularSpeed = 1.5f;
constexpr float kMaxAngularSpeed = 3.0f;

/// La fruta nace en la franja central de la pantalla (20 %–80 % del ancho).
constexpr float kSpawnXMinFactor = 0.20f;
constexpr float kSpawnXMaxFactor = 0.80f;

/// Paleta de colores de fruta.
const glm::vec3 kFruitPalette[] = {
    {0.90f, 0.22f, 0.21f},  // sandía
    {0.95f, 0.55f, 0.15f},  // naranja
    {0.98f, 0.85f, 0.20f},  // limón
    {0.45f, 0.75f, 0.25f},  // manzana verde
    {0.72f, 0.30f, 0.60f},  // uva
};
constexpr int kFruitColorCount =
    static_cast<int>(sizeof(kFruitPalette) / sizeof(kFruitPalette[0]));

/**
 * @brief Calcula la velocidad inicial necesaria para alcanzar una altura dada.
 * @param apex Altura máxima sobre el punto de lanzamiento, en píxeles.
 * @return Velocidad vertical en píxeles/segundo.
 *
 * De la conservación de energía, v² = 2·g·h, con g = -Physics2D::kGravity.y.
 */
float launchSpeedForApex(float apex) {
    const float gravity = -Physics2D::kGravity.y;
    return std::sqrt(2.0f * gravity * apex);
}

}  // namespace

Spawner::Spawner(float screenWidth, float screenHeight)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight) {
    const auto seed = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    m_rng.seed(seed);
}

std::unique_ptr<Fruit> Spawner::trySpawn(float dt) {
    m_timeUntilSpawn -= dt;
    if (m_timeUntilSpawn > 0.0f) {
        return nullptr;
    }

    m_timeUntilSpawn = randomFloat(kMinSpawnInterval, kMaxSpawnInterval);
    return spawnFruit();
}

std::unique_ptr<Fruit> Spawner::spawnFruit() {
    const float radius = randomFloat(m_screenHeight * kMinRadiusFactor,
                                     m_screenHeight * kMaxRadiusFactor);

    // Nace justo por debajo del borde inferior.
    const glm::vec2 position{
        randomFloat(m_screenWidth * kSpawnXMinFactor, m_screenWidth * kSpawnXMaxFactor),
        -radius,
    };

    const float apex = randomFloat(m_screenHeight * kMinApexFactor,
                                   m_screenHeight * kMaxApexFactor);
    const glm::vec2 velocity{
        randomFloat(-m_screenWidth * kMaxHorizontalSpeedFactor,
                     m_screenWidth * kMaxHorizontalSpeedFactor),
        launchSpeedForApex(apex),
    };

    const float angularVelocity = randomFloat(kMinAngularSpeed, kMaxAngularSpeed);
    return std::make_unique<Fruit>(position, velocity, radius, angularVelocity, randomColor());
}

float Spawner::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(m_rng);
}

glm::vec3 Spawner::randomColor() {
    std::uniform_int_distribution<int> distribution(0, kFruitColorCount - 1);
    return kFruitPalette[distribution(m_rng)];
}
