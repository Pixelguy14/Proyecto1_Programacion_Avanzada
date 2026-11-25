#include "Background.h"

#include "core/AssetManager.h"
#include "core/AssetPaths.h"
#include "graphics/Renderer.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>

/**
 * @file Background.cpp
 * @brief Carga de fondos mediante AssetManager y fundido cruzado.
 */

namespace {

/// Duración del crossfade, en segundos.
constexpr float kFadeDuration = 0.8f;

/**
 * @brief Clave e archivo de cada escenario.
 *
 * El orden coincide con `BackgroundScene`, pero los nombres de archivo no
 * siguen el nombre del enum: `kRoom` (partida) usa `dojo_inside.png`, que
 * muestra el salón del dojo, y `kInside` (pausa) usa `dojo_room.png`, que
 * muestra la recámara/biblioteca. Es intencional: `dojo_inside` es el dojo
 * y `dojo_room` es una recámara. No invertir sin cambiar también los estados.
 */
struct SceneAsset {
    const char* key;   ///< Identificador en el AssetManager.
    const char* file;  ///< Archivo dentro de `src/graphics/assets/`.
};

constexpr SceneAsset kSceneAssets[] = {
    {"bg_outside", "dojo_outside.png"},  // kOutside: exterior del dojo
    {"bg_inside", "dojo_inside.png"},    // kRoom: salón del dojo (partida)
    {"bg_room", "dojo_room.png"},        // kInside: recámara (pausa)
    {"bg_nursery", "dojo_nursery.png"},  // kNursery: guardería
};

/**
 * @brief Convierte un escenario en su índice de arreglo.
 * @param scene Escenario consultado.
 * @return Índice en [0, 3].
 */
std::size_t sceneIndex(BackgroundScene scene) {
    return static_cast<std::size_t>(scene);
}

}  // namespace

bool Background::init(AssetManager& assets, Renderer& renderer, float width, float height) {
    m_assets = &assets;
    m_width = width;
    m_height = height;

    bool allLoaded = true;
    for (std::size_t i = 0; i < m_keys.size(); ++i) {
        m_keys[i] = kSceneAssets[i].key;
        const std::string path = AssetPaths::resolve(kSceneAssets[i].file);
        if (!assets.loadTexture(renderer, m_keys[i], path)) {
            std::cerr << "[Background] Falto cargar el fondo: " << path << std::endl;
            allLoaded = false;
        }
    }

    return allLoaded;
}

void Background::setScene(BackgroundScene scene, bool immediate) {
    if (immediate) {
        m_current = scene;
        m_previous = scene;
        m_hasPrevious = false;
        m_fade = 1.0f;
        return;
    }

    if (scene == m_current) {
        return;
    }

    m_previous = m_current;
    m_current = scene;
    m_hasPrevious = true;
    m_fade = 0.0f;
}

void Background::update(float dt) {
    if (!m_hasPrevious) {
        return;
    }

    m_fade = std::min(1.0f, m_fade + dt / kFadeDuration);
    if (m_fade >= 1.0f) {
        m_hasPrevious = false;
    }
}

void Background::render(Renderer& renderer) {
    if (m_assets == nullptr) {
        return;
    }

    const glm::vec2 center{m_width * 0.5f, m_height * 0.5f};
    const glm::vec2 size{m_width, m_height};

    if (m_hasPrevious) {
        const Texture* previous = m_assets->texture(m_keys[sceneIndex(m_previous)]);
        if (previous != nullptr) {
            renderer.drawTexture(*previous, center, size, 1.0f);
        }
    }

    const Texture* current = m_assets->texture(m_keys[sceneIndex(m_current)]);
    if (current != nullptr) {
        renderer.drawTexture(*current, center, size, m_hasPrevious ? m_fade : 1.0f);
    }
}
