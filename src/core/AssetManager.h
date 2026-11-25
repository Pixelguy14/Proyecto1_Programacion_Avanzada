#pragma once

#include "graphics/Texture.h"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

class AudioEngine;
class Renderer;
class Sound;

/**
 * @file AssetManager.h
 * @brief Registro de texturas y sonidos por identificador (Sprint 5).
 *
 * `AssetManager` usa `std::unordered_map`, la tabla hash de la STL, para
 * asociar un nombre legible (`"bg_room"`, `"sfx_slice"`) con el recurso ya
 * cargado. Así se evita cargar dos veces el mismo archivo y la liberación queda
 * centralizada.
 *
 *   main
 *     ├── assets.loadTexture(renderer, "bg_room", path)
 *     ├── assets.loadSound(audio, "sfx_slice", path)
 *     ├── Background → assets.texture("bg_room")
 *     ├── GameSession → assets.playSound("sfx_slice")
 *     └── assets.cleanup(renderer)
 *
 * El manager no posee el contexto OpenGL ni el motor de audio: las cargas
 * reciben `Renderer` y `AudioEngine` explícitamente. El motor debe sobrevivir
 * al manager porque los `Sound` liberan sus voces desde su destructor.
 */
class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    /// La tabla hash no se copia: el manager es el único dueño de los recursos.
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    /**
     * @brief Carga una textura y la registra con un identificador.
     * @param renderer Renderer ya inicializado.
     * @param id       Identificador único dentro del manager.
     * @param path     Ruta del archivo de imagen.
     * @return true si la textura quedó disponible (ya existía o se cargó bien).
     *
     * Si el identificador ya existe, no se vuelve a leer el archivo. Si la
     * carga falla, no se guarda una entrada inválida y se devuelve false.
     */
    bool loadTexture(Renderer& renderer, const std::string& id, const std::string& path);

    /**
     * @brief Busca una textura por identificador.
     * @param id Identificador registrado.
     * @return Puntero a la textura, o nullptr si no existe.
     *
     * El puntero es válido mientras el manager no se destruya ni se elimine la
     * entrada; `std::unordered_map` no invalida referencias al hacer rehash.
     */
    const Texture* texture(const std::string& id) const;

    /**
     * @brief Comprueba si un identificador de textura está registrado.
     * @param id Identificador a buscar.
     * @return true si existe una textura con ese identificador.
     */
    bool hasTexture(const std::string& id) const;

    /**
     * @brief Carga un efecto de sonido y lo registra con un identificador.
     * @param audio Motor de audio ya inicializado.
     * @param id    Identificador único dentro del manager.
     * @param path  Ruta del archivo de audio.
     * @return true si el sonido quedó disponible (ya existía o se cargó bien).
     */
    bool loadSound(AudioEngine& audio, const std::string& id, const std::string& path);

    /**
     * @brief Busca un sonido por identificador.
     * @param id Identificador registrado.
     * @return Puntero al sonido, o nullptr si no existe.
     */
    Sound* sound(const std::string& id);

    /**
     * @brief Comprueba si un identificador de sonido está registrado.
     * @param id Identificador a buscar.
     * @return true si existe un sonido con ese identificador.
     */
    bool hasSound(const std::string& id) const;

    /**
     * @brief Reproduce un sonido registrado.
     * @param id     Identificador del efecto.
     * @param volume Volumen en [0, 1].
     * @param pitch  Factor de tono/velocidad (1 = original).
     * @return true si el identificador existía; false si no se encontró.
     */
    bool playSound(const std::string& id, float volume = 1.0f, float pitch = 1.0f);

    /**
     * @brief Libera sonidos y texturas y vacía las tablas.
     * @param renderer Renderer que creó las texturas.
     *
     * Debe llamarse antes de `AudioEngine::shutdown()` y antes de
     * `Renderer::cleanup()`.
     */
    void cleanup(Renderer& renderer);

    /// @return Número de texturas registradas.
    std::size_t textureCount() const { return m_textures.size(); }

    /// @return Número de sonidos registrados.
    std::size_t soundCount() const { return m_sounds.size(); }

private:
    std::unordered_map<std::string, Texture> m_textures;            ///< id → textura.
    std::unordered_map<std::string, std::unique_ptr<Sound>> m_sounds;  ///< id → sonido.
};
