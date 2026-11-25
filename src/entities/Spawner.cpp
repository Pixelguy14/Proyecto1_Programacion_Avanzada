#include "Spawner.h"

#include <algorithm>
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

/// Bombas: lanzamientos de gracia antes de la primera y probabilidad después.
constexpr int kBombGraceSpawns = 4;
constexpr float kBombChance = 0.16f;
constexpr float kBombChanceLate = 0.28f;

/// Dificultad progresiva (Sprint 5): el juego se endurece con los lanzamientos.
constexpr int kDifficultyRampSpawns = 30;
constexpr float kMinSpawnIntervalLate = 0.32f;
constexpr float kMaxSpawnIntervalLate = 0.80f;
constexpr float kMaxHorizontalSpeedFactorLate = 0.14f;

/// Las bombas son un poco más grandes para que se distingan a simple vista.
constexpr float kBombRadiusBonus = 1.08f;

/// La fruta nace en la franja central de la pantalla (20 %–80 % del ancho).
constexpr float kSpawnXMinFactor = 0.20f;
constexpr float kSpawnXMaxFactor = 0.80f;

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

    // La dificultad recorta el intervalo y aumenta la velocidad horizontal.
    const float level = difficulty();
    const float minInterval = kMinSpawnInterval
        + (kMinSpawnIntervalLate - kMinSpawnInterval) * level;
    const float maxInterval = kMaxSpawnInterval
        + (kMaxSpawnIntervalLate - kMaxSpawnInterval) * level;
    m_timeUntilSpawn = randomFloat(minInterval, maxInterval);
    return spawnFruit();
}

std::unique_ptr<Fruit> Spawner::spawnFruit() {
    const bool bomb = shouldSpawnBomb();
    ++m_spawned;

    float radius = randomFloat(m_screenHeight * kMinRadiusFactor,
                               m_screenHeight * kMaxRadiusFactor);
    if (bomb) {
        radius *= kBombRadiusBonus;
    }

    // Nace justo por debajo del borde inferior.
    const Vector2D<float> position{
        randomFloat(m_screenWidth * kSpawnXMinFactor, m_screenWidth * kSpawnXMaxFactor),
        -radius,
    };

    const float apex = randomFloat(m_screenHeight * kMinApexFactor,
                                   m_screenHeight * kMaxApexFactor);
    const float maxHorizontalSpeed = kMaxHorizontalSpeedFactor
        + (kMaxHorizontalSpeedFactorLate - kMaxHorizontalSpeedFactor) * difficulty();
    const Vector2D<float> velocity{
        randomFloat(-m_screenWidth * maxHorizontalSpeed,
                     m_screenWidth * maxHorizontalSpeed),
        launchSpeedForApex(apex),
    };

    const float angularVelocity = randomFloat(kMinAngularSpeed, kMaxAngularSpeed);
    const FruitMeshId meshId = bomb ? FruitMeshId::kBomb : randomFruitKind();

    return std::make_unique<Fruit>(position, velocity, radius, angularVelocity, meshId);
}

bool Spawner::shouldSpawnBomb() {
    if (m_spawned < kBombGraceSpawns) {
        return false;
    }
    const float chance = kBombChance + (kBombChanceLate - kBombChance) * difficulty();
    return randomFloat(0.0f, 1.0f) < chance;
}

float Spawner::difficulty() const {
    const float progress = static_cast<float>(m_spawned)
        / static_cast<float>(kDifficultyRampSpawns);
    return std::min(1.0f, progress);
}

float Spawner::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(m_rng);
}

FruitMeshId Spawner::randomFruitKind() {
    // Lista explícita de frutas: no depende del orden interno del enum.
    static constexpr FruitMeshId kKinds[] = {
        FruitMeshId::kMelon,
        FruitMeshId::kApple,
        FruitMeshId::kOrange,
        FruitMeshId::kLemon,
        FruitMeshId::kBanana,
    };
    constexpr int kCount = static_cast<int>(sizeof(kKinds) / sizeof(kKinds[0]));
    std::uniform_int_distribution<int> distribution(0, kCount - 1);
    return kKinds[distribution(m_rng)];
}
