#pragma once

#include "Fruit.h"

#include <glm/glm.hpp>

#include <memory>
#include <random>

/**
 * @file Spawner.h
 * @brief Generador de frutas y bombas (Sprints 1–4).
 *
 * El Spawner decide *cuándo* y *cómo* nace un objeto: temporización aleatoria,
 * posición por debajo de la pantalla, velocidad de lanzamiento y color. No
 * almacena objetos; devuelve uno nuevo y la sesión se encarga de él.
 *
 * Las frutas nacen con la bandera `FruitFlag::kActive`. A partir del Sprint 4
 * (parte 2), tras unos lanzamientos de gracia, una fracción de los objetos
 * nace con `FruitFlag::kBomb`; cortar una bomba termina la partida.
 *
 *   GameSession::update()
 *     └── Spawner::trySpawn()
 *           └── std::make_unique<Fruit>()
 *
 * Los tamaños y velocidades se expresan como fracciones de la resolución para
 * que el juego se comporte igual en 1080p y en otras resoluciones.
 */
class Spawner {
public:
    /**
     * @brief Crea un spawner ligado a una resolución lógica.
     * @param screenWidth  Ancho de la pantalla en píxeles.
     * @param screenHeight Alto de la pantalla en píxeles.
     *
     * La semilla del generador aleatorio se toma del reloj del sistema.
     */
    Spawner(float screenWidth, float screenHeight);

    /**
     * @brief Avanza el temporizador y, si toca, crea una fruta o una bomba.
     * @param dt Delta time en segundos.
     * @return El objeto recién creado, o nullptr si todavía no toca lanzar otro.
     *
     * La primera llamada siempre devuelve una fruta, para no empezar con la
     * pantalla vacía. Las bombas solo aparecen después de unos lanzamientos de
     * gracia, para que el jugador entienda primero el corte normal.
     */
    std::unique_ptr<Fruit> trySpawn(float dt);

private:
    /**
     * @brief Construye un objeto con parámetros aleatorios válidos.
     * @return Fruta (o bomba, si el sorteo lo decide) lista para la sesión.
     */
    std::unique_ptr<Fruit> spawnFruit();

    /**
     * @brief Decide si el próximo objeto debe ser una bomba.
     * @return true si toca bomba según la probabilidad y el periodo de gracia.
     */
    bool shouldSpawnBomb();

    /**
     * @brief Nivel de dificultad normalizado.
     * @return 0 al inicio y 1 tras `kDifficultyRampSpawns` lanzamientos.
     *
     * Con la dificultad suben la frecuencia de lanzamiento, la velocidad
     * horizontal y la probabilidad de bomba.
     */
    float difficulty() const;

    /**
     * @brief Genera un número real uniforme en [min, max].
     * @param min Límite inferior.
     * @param max Límite superior.
     * @return Valor aleatorio en el rango indicado.
     */
    float randomFloat(float min, float max);

    /**
     * @brief Elige un color de la paleta de frutas.
     * @return Color RGB en el rango [0, 1].
     */
    glm::vec3 randomColor();

private:
    std::mt19937 m_rng;               ///< Generador Mersenne Twister.
    float m_screenWidth;              ///< Ancho lógico de la pantalla.
    float m_screenHeight;             ///< Alto lógico de la pantalla.
    float m_timeUntilSpawn = 0.0f;    ///< Segundos restantes hasta el próximo lanzamiento.
    int m_spawned = 0;                ///< Objetos lanzados; controla la gracia de bombas.
};
