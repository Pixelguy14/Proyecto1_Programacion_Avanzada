#pragma once

#include "graphics/Texture.h"

#include <array>

class Renderer;

/**
 * @file Background.h
 * @brief Fondos de los escenarios y crossfade entre estados (Sprint 4).
 *
 * Los cuatro escenarios son cosméticos: no cambian la dificultad ni la
 * generación de frutas. La máquina de estados decide cuál se muestra:
 *
 *   Menu      → outside
 *   Playing   → room
 *   Paused    → inside
 *   GameOver  → nursery
 *
 * ## Crossfade
 * Al cambiar de escenario se guarda el fondo anterior y se dibuja el nuevo
 * encima con alpha creciente durante `kFadeDuration` segundos. El fondo viejo
 * se dibuja opaco debajo, de modo que nunca se ve el color de limpieza.
 */
enum class BackgroundScene {
    kOutside,  ///< Dojo al aire libre (menú).
    kRoom,     ///< Habitación del dojo (partida).
    kInside,   ///< Interior del dojo (pausa).
    kNursery,  ///< Guardería / jardín (game over).
};

class Background {
public:
    Background() = default;

    /**
     * @brief Carga las cuatro texturas de fondo.
     * @param renderer Renderer ya inicializado.
     * @param width    Ancho lógico de la pantalla.
     * @param height   Alto lógico de la pantalla.
     * @return true si todas las texturas se cargaron correctamente.
     */
    bool init(Renderer& renderer, float width, float height);

    /**
     * @brief Libera las texturas de GPU.
     * @param renderer Renderer que creó las texturas.
     */
    void cleanup(Renderer& renderer);

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
    /// Texturas en el mismo orden que `BackgroundScene`.
    std::array<Texture, 4> m_textures{};
    BackgroundScene m_current = BackgroundScene::kOutside;  ///< Fondo de destino.
    BackgroundScene m_previous = BackgroundScene::kOutside; ///< Fondo que se desvanece.
    bool m_hasPrevious = false;  ///< true mientras hay un crossfade en curso.
    float m_fade = 1.0f;         ///< Alpha del fondo actual [0, 1].
    float m_width = 0.0f;        ///< Ancho lógico de la pantalla.
    float m_height = 0.0f;       ///< Alto lógico de la pantalla.
};
