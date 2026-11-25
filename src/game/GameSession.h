#pragma once

#include "core/GameObject.h"
#include "entities/Blade.h"
#include "entities/Particle.h"
#include "entities/Spawner.h"
#include "graphics/ScreenShake.h"

#include <memory>
#include <random>
#include <vector>

class Input;
class Renderer;

/**
 * @file GameSession.h
 * @brief Estado y reglas de una partida (Sprint 4).
 *
 * GameSession agrupa todo lo que existe solo mientras se juega: el spawner,
 * las entidades, la espada, las partículas, el puntaje y las vidas. El bucle
 * principal decide *cuándo* actualizarla (solo en `Playing`) y la UI consulta
 * sus contadores.
 *
 *   main (máquina de estados)
 *     └── GameSession
 *           ├── Spawner            crea frutas y bombas
 *           ├── GameObjectList     frutas y mitades
 *           ├── ParticleList       jugo y chispas (solo visuales)
 *           ├── Blade              estela y cortes
 *           └── score / lives      reglas de fin de partida
 *
 * ## Reglas actuales
 * - Cortar una fruta suma 1 punto y emite una ráfaga de jugo.
 * - Dejar caer una fruta activa (sin cortarla) resta 1 vida.
 * - Cortar una bomba termina la partida de inmediato (vidas a cero) y emite
 *   una explosión de chispas.
 * - Con 0 vidas, `isGameOver()` devuelve true y el bucle cambia a `GameOver`.
 *
 * Las partículas no participan en las reglas: no suman puntos, no restan vidas
 * y se eliminan solas al agotar su vida.
 *
 * ## Screen shake (Sprint 5)
 * La sesión también acumula pequeños impulsos de cámara al cortar frutas y uno
 * grande al explotar una bomba. `screenShakeOffset()` devuelve el
 * desplazamiento que el bucle aplica al Renderer mientras dibuja la escena.
 */
class GameSession {
public:
    /**
     * @brief Crea una sesión ligada a la entrada y a la resolución lógica.
     * @param input        Entrada de la que la espada lee el mouse.
     * @param screenWidth  Ancho de la pantalla en píxeles.
     * @param screenHeight Alto de la pantalla en píxeles.
     *
     * La referencia a `input` debe sobrevivir a la sesión.
     */
    GameSession(const Input& input, float screenWidth, float screenHeight);

    /**
     * @brief Reinicia la partida: vacía entidades y restaura puntaje y vidas.
     *
     * También limpia la estela y las partículas para que no arrastren puntos
     * de la pantalla anterior y recrea el spawner con la resolución actual.
     */
    void reset();

    /**
     * @brief Avanza un frame de simulación.
     * @param dt Delta time en segundos.
     *
     * Orden: spawn → física → espada → partículas → cortes → frutas perdidas
     * → limpieza. Las partículas se actualizan antes de los cortes para que las
     * ráfagas recién emitidas aparezcan en su origen y no un paso adelantadas.
     * Si las vidas llegan a cero, la sesión queda marcada como terminada.
     */
    void update(float dt);

    /**
     * @brief Avanza solo los efectos visuales, sin tocar las reglas.
     * @param dt Delta time en segundos.
     *
     * Se usa en `GameOver` para que la explosión de una bomba termine de
     * animarse aunque la simulación principal ya esté detenida.
     */
    void updateEffects(float dt);

    /**
     * @brief Dibuja entidades, partículas y, encima, la estela de la espada.
     * @param renderer Renderer compartido.
     */
    void render(Renderer& renderer);

    /// @return Puntaje acumulado en la partida.
    int score() const { return m_score; }

    /// @return Vidas restantes (nunca menor que cero).
    int lives() const { return m_lives; }

    /// @return true si la partida terminó por quedarse sin vidas.
    bool isGameOver() const { return m_gameOver; }

    /// @return true si la partida terminó porque el jugador cortó una bomba.
    bool endedByBomb() const { return m_bombExploded; }

    /// @return Desplazamiento actual del screen shake en píxeles.
    const glm::vec2& screenShakeOffset() const { return m_shake.offset(); }

private:
    /// Contenedor polimórfico de entidades; posee los objetos con unique_ptr.
    using GameObjectList = std::vector<std::unique_ptr<GameObject>>;

    /// Contenedor de partículas visuales.
    using ParticleList = std::vector<std::unique_ptr<Particle>>;

    /**
     * @brief Corta las frutas activas que toque el recorrido reciente.
     * @return Número de frutas cortadas en este frame.
     *
     * Las mitades se recogen en un vector temporal para no invalidar el
     * iterador mientras se recorre la lista. Si el tajo toca una bomba, la
     * marca como cortada, emite la explosión y deja las vidas en cero.
     */
    int sliceFruits();

    /**
     * @brief Cuenta las frutas activas que cayeron sin ser cortadas.
     * @return Número de frutas perdidas en este frame.
     *
     * Las bombas se excluyen: dejarlas caer no penaliza.
     */
    int countMissedFruits();

    /**
     * @brief Elimina de la lista las entidades marcadas como no vivas.
     */
    void removeDeadObjects();

    /**
     * @brief Avanza y elimina las partículas caducadas.
     * @param dt Delta time en segundos.
     */
    void updateParticles(float dt);

    /**
     * @brief Elimina las partículas marcadas como no vivas.
     */
    void removeDeadParticles();

    /**
     * @brief Emite una ráfaga de gotas de jugo en el punto de corte.
     * @param origin Centro de la fruta cortada.
     * @param color  Color de la fruta.
     * @param radius Radio de la fruta, para escalar el tamaño de las gotas.
     *
     * Respeta el presupuesto máximo de partículas para que una racha de cortes
     * no haga crecer la lista sin control.
     */
    void emitJuice(const Vector2D<float>& origin, const glm::vec3& color, float radius);

    /**
     * @brief Emite chispas y humo al cortar una bomba.
     * @param origin Centro de la bomba.
     * @param radius Radio de la bomba.
     */
    void emitExplosion(const Vector2D<float>& origin, float radius);

    /**
     * @brief Dibuja todas las entidades de la lista.
     * @param renderer Renderer compartido.
     */
    void renderObjects(Renderer& renderer);

    /**
     * @brief Dibuja las partículas activas.
     * @param renderer Renderer compartido.
     */
    void renderParticles(Renderer& renderer);

    /**
     * @brief Genera un número real uniforme en [min, max].
     * @param min Límite inferior.
     * @param max Límite superior.
     * @return Valor aleatorio en el rango indicado.
     */
    float randomFloat(float min, float max);

private:
    float m_width;               ///< Ancho lógico de la pantalla.
    float m_height;              ///< Alto lógico de la pantalla.
    Spawner m_spawner;           ///< Generador de frutas y bombas.
    Blade m_blade;               ///< Espada ligada a la entrada.
    ScreenShake m_shake;         ///< Impulsos de cámara (Sprint 5).
    GameObjectList m_objects;    ///< Frutas y mitades activas.
    ParticleList m_particles;    ///< Jugo y chispas (solo visuales).
    std::mt19937 m_rng;          ///< Generador para la variación de partículas.
    int m_score = 0;             ///< Puntaje actual.
    int m_lives = 3;             ///< Vidas restantes.
    bool m_gameOver = false;     ///< true cuando ya no quedan vidas.
    bool m_bombExploded = false; ///< true si el fin de partida fue por una bomba.
};
