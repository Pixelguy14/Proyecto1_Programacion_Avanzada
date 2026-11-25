#pragma once

#include "graphics/Texture.h"

#include <cstddef>
#include <string>
#include <unordered_map>

class Renderer;

/**
 * @file AssetManager.h
 * @brief Registro de texturas por identificador de texto (Sprint 5).
 *
 * `AssetManager` usa `std::unordered_map`, la tabla hash de la STL, para
 * asociar un nombre legible (`"bg_room"`, `"tex_fruit"`) con la textura ya
 * cargada. Así se evita cargar dos veces el mismo archivo y la liberación de
 * GPU queda centralizada en un solo lugar.
 *
 *   main
 *     ├── assets.loadTexture(renderer, "bg_room", path)
 *     ├── Background → assets.texture("bg_room")
 *     └── assets.cleanup(renderer)
 *
 * La tabla no posee el contexto OpenGL: `loadTexture()` y `cleanup()` reciben
 * el `Renderer` explícitamente, igual que el resto de recursos gráficos.
 */
class AssetManager {
public:
    AssetManager() = default;
    ~AssetManager() = default;

    /// La tabla hash no se copia: el manager es el único dueño de las texturas.
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
     * @brief Comprueba si un identificador está registrado.
     * @param id Identificador a buscar.
     * @return true si existe una textura con ese identificador.
     */
    bool hasTexture(const std::string& id) const;

    /**
     * @brief Libera todas las texturas de GPU y vacía la tabla.
     * @param renderer Renderer que creó las texturas.
     */
    void cleanup(Renderer& renderer);

    /// @return Número de texturas registradas.
    std::size_t textureCount() const { return m_textures.size(); }

private:
    std::unordered_map<std::string, Texture> m_textures;  ///< id → textura.
};
