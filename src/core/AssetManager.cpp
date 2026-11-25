#include "AssetManager.h"

#include "audio/AudioEngine.h"
#include "audio/Sound.h"
#include "graphics/Renderer.h"

#include <iostream>

/**
 * @file AssetManager.cpp
 * @brief Implementación de las tablas hash de texturas y sonidos.
 */

AssetManager::AssetManager() = default;

AssetManager::~AssetManager() = default;

bool AssetManager::loadTexture(Renderer& renderer, const std::string& id,
                               const std::string& path) {
    const auto existing = m_textures.find(id);
    if (existing != m_textures.end()) {
        return existing->second.valid();
    }

    Texture texture = renderer.loadTexture(path);
    if (!texture.valid()) {
        std::cerr << "[AssetManager] No se pudo registrar la textura '" << id << "': "
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

bool AssetManager::loadSound(AudioEngine& audio, const std::string& id,
                             const std::string& path) {
    const auto existing = m_sounds.find(id);
    if (existing != m_sounds.end()) {
        return existing->second != nullptr && existing->second->valid();
    }

    auto sound = std::make_unique<Sound>();
    if (!sound->load(audio, path)) {
        std::cerr << "[AssetManager] No se pudo registrar el sonido '" << id << "': "
                  << path << std::endl;
        return false;
    }

    m_sounds.emplace(id, std::move(sound));
    return true;
}

Sound* AssetManager::sound(const std::string& id) {
    const auto it = m_sounds.find(id);
    return it != m_sounds.end() ? it->second.get() : nullptr;
}

bool AssetManager::hasSound(const std::string& id) const {
    return m_sounds.find(id) != m_sounds.end();
}

bool AssetManager::playSound(const std::string& id, float volume, float pitch) {
    Sound* effect = sound(id);
    if (effect == nullptr) {
        return false;
    }
    effect->play(volume, pitch);
    return true;
}

void AssetManager::cleanup(Renderer& renderer) {
    // Los Sound deben liberarse antes de que el AudioEngine se apague.
    m_sounds.clear();

    for (auto& [id, texture] : m_textures) {
        (void)id;
        renderer.destroyTexture(texture);
    }
    m_textures.clear();
}
