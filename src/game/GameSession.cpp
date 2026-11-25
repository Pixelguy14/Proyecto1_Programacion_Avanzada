#include "GameSession.h"

#include "entities/Fruit.h"
#include "entities/FruitHalf.h"
#include "graphics/Renderer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>

/**
 * @file GameSession.cpp
 * @brief Implementación de las reglas y el bucle interno de una partida.
 */

namespace {

/// Vidas con las que arranca cada partida.
constexpr int kStartingLives = 3;

/// Presupuesto máximo de partículas simultáneas (jugo + chispas).
constexpr std::size_t kMaxParticles = 500;

/// Ráfaga de jugo al cortar una fruta.
constexpr int kJuiceParticles = 14;
constexpr float kJuiceSpeedMin = 90.0f;
constexpr float kJuiceSpeedMax = 360.0f;
constexpr float kJuiceRadiusMinFactor = 0.06f;
constexpr float kJuiceRadiusMaxFactor = 0.18f;
constexpr float kJuiceLifeMin = 0.30f;
constexpr float kJuiceLifeMax = 0.75f;
constexpr float kJuiceGravityScale = 0.85f;
constexpr float kJuiceDrag = 0.8f;

/// Explosión al cortar una bomba.
constexpr int kExplosionParticles = 24;
constexpr float kExplosionSpeedMin = 220.0f;
constexpr float kExplosionSpeedMax = 680.0f;
constexpr float kExplosionRadiusMinFactor = 0.05f;
constexpr float kExplosionRadiusMaxFactor = 0.20f;
constexpr float kExplosionLifeMin = 0.25f;
constexpr float kExplosionLifeMax = 0.60f;
constexpr float kExplosionGravityScale = 0.45f;
constexpr float kExplosionDrag = 1.6f;
const glm::vec3 kSparkColor{1.00f, 0.62f, 0.15f};
const glm::vec3 kSmokeColor{0.28f, 0.27f, 0.30f};

/// Una vuelta completa, para repartir las partículas en todas las direcciones.
constexpr float kTwoPi = 6.28318530718f;

}  // namespace

GameSession::GameSession(const Input& input, float screenWidth, float screenHeight)
    : m_width(screenWidth),
      m_height(screenHeight),
      m_spawner(screenWidth, screenHeight),
      m_blade(input) {
    m_objects.reserve(64);
    m_particles.reserve(256);

    const auto seed = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    m_rng.seed(seed);
}

void GameSession::reset() {
    m_objects.clear();
    m_particles.clear();
    m_score = 0;
    m_lives = kStartingLives;
    m_gameOver = false;
    m_bombExploded = false;
    m_blade.clearTrail();
    m_spawner = Spawner(m_width, m_height);
}

void GameSession::update(float dt) {
    if (m_gameOver) {
        return;
    }

    if (auto fruit = m_spawner.trySpawn(dt)) {
        m_objects.push_back(std::move(fruit));
    }

    for (auto& object : m_objects) {
        object->update(dt);
    }

    m_blade.update(dt);
    updateParticles(dt);
    sliceFruits();

    m_lives -= countMissedFruits();
    m_lives = std::max(m_lives, 0);

    removeDeadObjects();

    if (m_lives == 0) {
        m_gameOver = true;
    }
}

void GameSession::updateEffects(float dt) {
    updateParticles(dt);
}

int GameSession::sliceFruits() {
    GameObjectList halves;
    int sliced = 0;

    for (auto& object : m_objects) {
        auto* fruit = dynamic_cast<Fruit*>(object.get());
        if (fruit == nullptr || !fruit->hasFlag(FruitFlag::kActive)) {
            continue;
        }

        const Blade::CutResult cut = m_blade.cutAt(fruit->position(), fruit->radius());
        if (!cut.hit) {
            continue;
        }

        if (fruit->hasFlag(FruitFlag::kBomb)) {
            // Cortar una bomba termina la partida: no genera mitades ni puntos.
            fruit->markSliced();
            emitExplosion(fruit->position(), fruit->radius());
            m_bombExploded = true;
            m_lives = 0;
            continue;
        }

        auto [first, second] = fruit->slice(cut.direction);
        if (first) {
            halves.push_back(std::move(first));
        }
        if (second) {
            halves.push_back(std::move(second));
        }

        emitJuice(fruit->position(), fruit->color(), fruit->radius());
        ++sliced;
        ++m_score;
    }

    for (auto& half : halves) {
        m_objects.push_back(std::move(half));
    }

    return sliced;
}

int GameSession::countMissedFruits() {
    int missed = 0;

    for (const auto& object : m_objects) {
        const auto* fruit = dynamic_cast<const Fruit*>(object.get());
        if (fruit == nullptr
            || fruit->isAlive()
            || !fruit->hasFlag(FruitFlag::kActive)
            || fruit->hasFlag(FruitFlag::kBomb)) {
            continue;
        }
        ++missed;
    }

    return missed;
}

void GameSession::removeDeadObjects() {
    const auto isDead = [](const std::unique_ptr<GameObject>& object) {
        return !object->isAlive();
    };
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(), isDead),
                    m_objects.end());
}

void GameSession::updateParticles(float dt) {
    for (auto& particle : m_particles) {
        particle->update(dt);
    }
    removeDeadParticles();
}

void GameSession::removeDeadParticles() {
    const auto isDead = [](const std::unique_ptr<Particle>& particle) {
        return !particle->isAlive();
    };
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(), isDead),
                      m_particles.end());
}

void GameSession::emitJuice(const Vector2D<float>& origin, const glm::vec3& color,
                            float radius) {
    for (int i = 0; i < kJuiceParticles; ++i) {
        if (m_particles.size() >= kMaxParticles) {
            break;
        }

        const float angle = randomFloat(0.0f, kTwoPi);
        const float speed = randomFloat(kJuiceSpeedMin, kJuiceSpeedMax);
        const Vector2D<float> velocity{
            std::cos(angle) * speed,
            std::sin(angle) * speed,
        };

        const float particleRadius =
            radius * randomFloat(kJuiceRadiusMinFactor, kJuiceRadiusMaxFactor);
        const float lifetime = randomFloat(kJuiceLifeMin, kJuiceLifeMax);

        // Variación ligera del color para que la ráfaga no sea uniforme.
        const float tint = randomFloat(0.80f, 1.12f);
        const glm::vec3 particleColor{
            std::min(color.r * tint, 1.0f),
            std::min(color.g * tint, 1.0f),
            std::min(color.b * tint, 1.0f),
        };

        m_particles.push_back(std::make_unique<Particle>(
            origin, velocity, particleRadius, lifetime, particleColor,
            kJuiceGravityScale, kJuiceDrag));
    }
}

void GameSession::emitExplosion(const Vector2D<float>& origin, float radius) {
    for (int i = 0; i < kExplosionParticles; ++i) {
        if (m_particles.size() >= kMaxParticles) {
            break;
        }

        const float angle = randomFloat(0.0f, kTwoPi);
        const float speed = randomFloat(kExplosionSpeedMin, kExplosionSpeedMax);
        const Vector2D<float> velocity{
            std::cos(angle) * speed,
            std::sin(angle) * speed,
        };

        const float particleRadius =
            radius * randomFloat(kExplosionRadiusMinFactor, kExplosionRadiusMaxFactor);
        const float lifetime = randomFloat(kExplosionLifeMin, kExplosionLifeMax);

        // Alterna chispas brillantes y humo oscuro.
        const glm::vec3& particleColor = (i % 2 == 0) ? kSparkColor : kSmokeColor;

        m_particles.push_back(std::make_unique<Particle>(
            origin, velocity, particleRadius, lifetime, particleColor,
            kExplosionGravityScale, kExplosionDrag));
    }
}

void GameSession::renderObjects(Renderer& renderer) {
    for (const auto& object : m_objects) {
        object->render(renderer);
    }
}

void GameSession::renderParticles(Renderer& renderer) {
    for (const auto& particle : m_particles) {
        particle->render(renderer);
    }
}

void GameSession::render(Renderer& renderer) {
    renderObjects(renderer);
    renderParticles(renderer);
    m_blade.render(renderer);
}

float GameSession::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(m_rng);
}
