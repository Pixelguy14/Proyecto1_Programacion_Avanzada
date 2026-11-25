#include "AssetManager.h"

#include "graphics/Renderer.h"

#include <iostream>

/**
 * @file AssetManager.cpp
 * @brief Implementación de la tabla hash de texturas.
 */

bool AssetManager::loadTexture(Renderer& renderer, const std::string& id,
                               const std::string& path) {
    const auto existing = m_textures.find(id);
    if (existing != m_textures.end()) {
        return existing->second.valid();
    }

    Texture texture = renderer.loadTexture(path);
    if (!texture.valid()) {
        std::cerr << "[AssetManager] No se pudo registrar '" << id << "': "
                  << path << std::endl;
        return false;
    }

    m_textures.emplace(id, texture);
    return true;
}

const Texture* AssetManager::texture(const std::string& id) const {
    const auto it = m_textures.find(id);
    return it != m_textures.end() ? &it->second : nullptr;
}

bool AssetManager::hasTexture(const std::string& id) const {
    return m_textures.find(id) != m_textures.end();
}

void AssetManager::cleanup(Renderer& renderer) {
    for (auto& [id, texture] : m_textures) {
        (void)id;
        renderer.destroyTexture(texture);
    }
    m_textures.clear();
}
