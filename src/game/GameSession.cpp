#include "GameSession.h"

#include "entities/Fruit.h"
#include "entities/FruitHalf.h"
#include "graphics/Renderer.h"

#include <algorithm>

/**
 * @file GameSession.cpp
 * @brief Implementación de las reglas y el bucle interno de una partida.
 */

namespace {

/// Vidas con las que arranca cada partida.
constexpr int kStartingLives = 3;

}  // namespace

GameSession::GameSession(const Input& input, float screenWidth, float screenHeight)
    : m_width(screenWidth),
      m_height(screenHeight),
      m_spawner(screenWidth, screenHeight),
      m_blade(input) {
    m_objects.reserve(64);
}

void GameSession::reset() {
    m_objects.clear();
    m_score = 0;
    m_lives = kStartingLives;
    m_gameOver = false;
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
    sliceFruits();

    m_lives -= countMissedFruits();
    m_lives = std::max(m_lives, 0);

    removeDeadObjects();

    if (m_lives == 0) {
        m_gameOver = true;
    }
}

int GameSession::sliceFruits() {
    GameObjectList halves;
    int sliced = 0;

    for (auto& object : m_objects) {
        auto* fruit = dynamic_cast<Fruit*>(object.get());
        if (fruit == nullptr
            || !fruit->hasFlag(FruitFlag::kActive)
            || fruit->hasFlag(FruitFlag::kBomb)) {
            continue;
        }

        const Blade::CutResult cut = m_blade.cutAt(fruit->position(), fruit->radius());
        if (!cut.hit) {
            continue;
        }

        auto [first, second] = fruit->slice(cut.direction);
        if (first) {
            halves.push_back(std::move(first));
        }
        if (second) {
            halves.push_back(std::move(second));
        }
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

void GameSession::renderObjects(Renderer& renderer) {
    for (const auto& object : m_objects) {
        object->render(renderer);
    }
}

void GameSession::render(Renderer& renderer) {
    renderObjects(renderer);
    m_blade.render(renderer);
}
