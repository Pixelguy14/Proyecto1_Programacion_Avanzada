#include "Background.h"

#include "core/AssetPaths.h"
#include "graphics/Renderer.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>

/**
 * @file Background.cpp
 * @brief Carga de fondos y fundido cruzado entre escenarios.
 */

namespace {

/// Duración del crossfade, en segundos.
constexpr float kFadeDuration = 0.8f;

/// Nombres de archivo en el mismo orden que `BackgroundScene`.
constexpr const char* kSceneFiles[] = {
    "dojo_outside.png",
    "dojo_inside.png",
    "dojo_room.png",
    "dojo_nursery.png",
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

bool Background::init(Renderer& renderer, float width, float height) {
    m_width = width;
    m_height = height;

    bool allLoaded = true;
    for (std::size_t i = 0; i < m_textures.size(); ++i) {
        const std::string path = AssetPaths::resolve(kSceneFiles[i]);
        m_textures[i] = renderer.loadTexture(path);
        if (!m_textures[i].valid()) {
            std::cerr << "[Background] Falto cargar el fondo: " << path << std::endl;
            allLoaded = false;
        }
    }

    return allLoaded;
}

void Background::cleanup(Renderer& renderer) {
    for (Texture& texture : m_textures) {
        renderer.destroyTexture(texture);
    }
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
    const glm::vec2 center{m_width * 0.5f, m_height * 0.5f};
    const glm::vec2 size{m_width, m_height};

    if (m_hasPrevious) {
        const Texture& previous = m_textures[sceneIndex(m_previous)];
        renderer.drawTexture(previous, center, size, 1.0f);
    }

    const Texture& current = m_textures[sceneIndex(m_current)];
    renderer.drawTexture(current, center, size, m_hasPrevious ? m_fade : 1.0f);
}
