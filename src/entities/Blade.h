#pragma once

#include "core/GameObject.h"
#include "math/Collision.h"
#include "math/Vector2D.h"

#include <deque>
#include <vector>

class Input;
class Renderer;

/**
 * @file Blade.h
 * @brief Espada del jugador: historial de posiciones y estela (Sprints 2–3).
 *
 * Blade no dibuja directamente: guarda los últimos puntos del recorrido del
 * mouse y los convierte en una cinta con grosor y alpha decrecientes. La
 * geometría de la cinta vive en el Renderer.
 *
 *   Input (callbacks GLFW)
 *     └── Blade::update()   historial en std::deque y caché de segmentos
 *           ├── Blade::render() → Renderer::drawTrail()
 *           └── Blade::cutAt()  → Collision::firstSegmentHit() (AVX o escalar)
 *
 * Los puntos caducan por tiempo (no solo por cantidad), así que al detener el
 * mouse la estela se desvanece sola. Los segmentos se reconstruyen una vez por
 * frame para que todas las frutas compartan el mismo arreglo contiguo, que es
 * lo que permite evaluarlos en lote con AVX.
 */
class Blade : public GameObject {
public:
    /**
     * @brief Resultado de comprobar el tajo contra un círculo.
     */
    struct CutResult {
        bool hit = false;                ///< true si algún segmento tocó el círculo.
        Vector2D<float> direction{};     ///< Dirección del segmento que cortó.
    };

public:
    /**
     * @brief Crea la espada asociada a una fuente de entrada.
     * @param input Entrada de la que se lee la posición del mouse.
     *
     * La referencia debe sobrevivir a la Blade.
     */
    explicit Blade(const Input& input);

    /**
     * @brief Envejece la estela y añade la posición actual si hubo movimiento.
     * @param dt Delta time en segundos.
     */
    void update(float dt) override;

    /**
     * @brief Dibuja la estela y la punta de la espada.
     * @param renderer Renderer compartido.
     */
    void render(Renderer& renderer) override;

    /**
     * @brief Comprueba si el recorrido reciente de la espada corta un círculo.
     * @param center Centro del círculo (la fruta).
     * @param radius Radio del círculo en píxeles.
     * @return `hit` y la dirección del segmento que produjo el corte.
     *
     * Recorre los segmentos del historial del más reciente al más antiguo: el
     * primero que intersecta es el que define la orientación del tajo. La
     * búsqueda se delega en `Collision::firstSegmentHit()`, que puede evaluar
     * 8 segmentos a la vez con AVX. Si hay menos de dos puntos, no hay segmento
     * y devuelve `hit = false`.
     */
    CutResult cutAt(const Vector2D<float>& center, float radius) const;

    /**
     * @brief Vacía el historial de la estela y la caché de segmentos.
     *
     * Se usa al cambiar de estado (menú → partida, reinicio o game over) para
     * que el tajo no arrastre puntos de la pantalla anterior.
     */
    void clearTrail() {
        m_trail.clear();
        m_segments.clear();
    }

private:
    /// Punto del historial con su antigüedad.
    struct TrailPoint {
        Vector2D<float> position;  ///< Posición en coordenadas del juego.
        float age = 0.0f;          ///< Segundos desde que se registró.
    };

    /**
     * @brief Reconstruye el arreglo contiguo de segmentos desde el deque.
     *
     * El arreglo queda ordenado del segmento más reciente al más antiguo, que
     * es el orden en el que `firstSegmentHit()` debe encontrar el primer tajo.
     */
    void rebuildSegments();

private:
    const Input& m_input;            ///< Fuente de la posición del mouse.
    std::deque<TrailPoint> m_trail;  ///< Del punto más antiguo al más reciente.
    std::vector<Collision::Segment> m_segments;  ///< Caché contigua para AVX.
};
