#pragma once

#include "graphics/Texture.h"

#include <array>
#include <string>

class AssetManager;
class Renderer;

/**
 * @file Background.h
 * @brief Fondos de los escenarios y crossfade entre estados (Sprints 4–5).
 *
 * Los cuatro escenarios son cosméticos: no cambian la dificultad ni la
 * generación de frutas. La máquina de estados decide cuál se muestra:
 *
 *   Menu      → outside  (dojo_outside.png)
 *   Playing   → room     (dojo_inside.png, salón del dojo)
 *   Paused    → inside   (dojo_room.png, recámara/biblioteca)
 *   GameOver  → nursery  (dojo_nursery.png)
 *
 * Ojo: el nombre del enum y el del archivo no siempre coinciden. `kRoom` es el
 * escenario de juego y muestra el salón del dojo (`dojo_inside.png`), mientras
 * que `kInside` es la pausa y muestra la recámara (`dojo_room.png`). Es una
 * decisión de arte, no un cruce accidental.
 *
 * Las texturas ya no las posee `Background`: se cargan en el `AssetManager`
 * con identificadores (`bg_outside`, `bg_room`, …) y aquí solo se guardan las
 * claves. Así el manager centraliza la carga y la liberación de GPU.
 *
 * ## Crossfade
 * Al cambiar de escenario se guarda el fondo anterior y se dibuja el nuevo
 * encima con alpha creciente durante `kFadeDuration` segundos. El fondo viejo
 * se dibuja opaco debajo, de modo que nunca se ve el color de limpieza.
 */
enum class BackgroundScene {
    kOutside,  ///< Exterior del dojo (menú) → dojo_outside.png.
    kRoom,     ///< Salón del dojo (partida) → dojo_inside.png.
    kInside,   ///< Recámara/biblioteca (pausa) → dojo_room.png.
    kNursery,  ///< Guardería / jardín (game over) → dojo_nursery.png.
};

class Background {
public:
    Background() = default;

    /**
     * @brief Carga las cuatro texturas de fondo en el manager de recursos.
     * @param assets   Manager donde se registran las texturas.
     * @param renderer Renderer ya inicializado.
     * @param width    Ancho lógico de la pantalla.
     * @param height   Alto lógico de la pantalla.
     * @return true si todas las texturas se cargaron correctamente.
     *
     * El manager debe sobrevivir a este objeto.
     */
    bool init(AssetManager& assets, Renderer& renderer, float width, float height);

    /**
     * @brief Cambia el escenario visible, con crossfade si no es inmediato.
     * @param scene     Nuevo escenario.
     * @param immediate true para saltar el fundido (arranque del juego).
     *
     * Si el escenario pedido ya está activo, la llamada no hace nada.
     */
    void setScene(BackgroundScene scene, bool immediate = false);

    /**
     * @brief Avanza el temporizador del crossfade.
     * @param dt Delta time en segundos.
     */
    void update(float dt);

    /**
     * @brief Dibuja el fondo anterior y el actual con su alpha de fundido.
     * @param renderer Renderer compartido.
     */
    void render(Renderer& renderer);

private:
    /// Claves de las texturas en el mismo orden que `BackgroundScene`.
    std::array<std::string, 4> m_keys{};
    const AssetManager* m_assets = nullptr;  ///< Manager que posee las texturas.
    BackgroundScene m_current = BackgroundScene::kOutside;  ///< Fondo de destino.
    BackgroundScene m_previous = BackgroundScene::kOutside; ///< Fondo que se desvanece.
    bool m_hasPrevious = false;  ///< true mientras hay un crossfade en curso.
    float m_fade = 1.0f;         ///< Alpha del fondo actual [0, 1].
    float m_width = 0.0f;        ///< Ancho lógico de la pantalla.
    float m_height = 0.0f;       ///< Alto lógico de la pantalla.
};
