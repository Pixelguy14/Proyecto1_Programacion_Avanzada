#pragma once

#include "core/GameObject.h"
#include "entities/Blade.h"
#include "entities/Spawner.h"

#include <memory>
#include <vector>

class Input;
class Renderer;

/**
 * @file GameSession.h
 * @brief Estado y reglas de una partida (Sprint 4).
 *
 * GameSession agrupa todo lo que existe solo mientras se juega: el spawner,
 * las entidades, la espada, el puntaje y las vidas. El bucle principal decide
 * *cuándo* actualizarla (solo en `Playing`) y la UI consulta sus contadores.
 *
 *   main (máquina de estados)
 *     └── GameSession
 *           ├── Spawner            crea frutas
 *           ├── GameObjectList     frutas y mitades
 *           ├── Blade              estela y cortes
 *           └── score / lives      reglas de fin de partida
 *
 * ## Reglas actuales
 * - Cortar una fruta suma 1 punto.
 * - Dejar caer una fruta activa (sin cortarla) resta 1 vida.
 * - Con 0 vidas, `isGameOver()` devuelve true y el bucle cambia a `GameOver`.
 *
 * Las bombas (`FruitFlag::kBomb`) ya se respetan al cortar y al contar
 * pérdidas, aunque el spawner todavía no las genere.
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
     * También limpia la estela para que no arrastre puntos de la pantalla
     * anterior y recrea el spawner con la resolución actual.
     */
    void reset();

    /**
     * @brief Avanza un frame de simulación.
     * @param dt Delta time en segundos.
     *
     * Orden: spawn → física → espada → cortes → frutas perdidas → limpieza.
     * Si las vidas llegan a cero, la sesión queda marcada como terminada.
     */
    void update(float dt);

    /**
     * @brief Dibuja las entidades y, encima, la estela de la espada.
     * @param renderer Renderer compartido.
     */
    void render(Renderer& renderer);

    /// @return Puntaje acumulado en la partida.
    int score() const { return m_score; }

    /// @return Vidas restantes (nunca menor que cero).
    int lives() const { return m_lives; }

    /// @return true si la partida terminó por quedarse sin vidas.
    bool isGameOver() const { return m_gameOver; }

private:
    /// Contenedor polimórfico de entidades; posee los objetos con unique_ptr.
    using GameObjectList = std::vector<std::unique_ptr<GameObject>>;

    /**
     * @brief Corta las frutas activas que toque el recorrido reciente.
     * @return Número de frutas cortadas en este frame.
     *
     * Las mitades se recogen en un vector temporal para no invalidar el
     * iterador mientras se recorre la lista.
     */
    int sliceFruits();

    /**
     * @brief Cuenta las frutas activas que cayeron sin ser cortadas.
     * @return Número de frutas perdidas en este frame.
     */
    int countMissedFruits();

    /**
     * @brief Elimina de la lista las entidades marcadas como no vivas.
     */
    void removeDeadObjects();

    /**
     * @brief Dibuja todas las entidades de la lista.
     * @param renderer Renderer compartido.
     */
    void renderObjects(Renderer& renderer);

private:
    float m_width;               ///< Ancho lógico de la pantalla.
    float m_height;              ///< Alto lógico de la pantalla.
    Spawner m_spawner;           ///< Generador de frutas.
    Blade m_blade;               ///< Espada ligada a la entrada.
    GameObjectList m_objects;    ///< Frutas y mitades activas.
    int m_score = 0;             ///< Puntaje actual.
    int m_lives = 3;             ///< Vidas restantes.
    bool m_gameOver = false;     ///< true cuando ya no quedan vidas.
};
