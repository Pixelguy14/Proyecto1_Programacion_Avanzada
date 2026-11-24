#pragma once

#include "Fruit.h"

#include <glm/glm.hpp>

#include <memory>
#include <random>

/**
 * @file Spawner.h
 * @brief Generador de frutas para el Sprint 1.
 *
 * El Spawner decide *cuándo* y *cómo* nace una fruta: temporización aleatoria,
 * posición por debajo de la pantalla, velocidad de lanzamiento y color. No
 * almacena frutas; devuelve una nueva y el bucle principal se encarga de ella.
 *
 *   runGameLoop()
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
     * @brief Avanza el temporizador y, si toca, crea una fruta nueva.
     * @param dt Delta time en segundos.
     * @return La fruta recién creada, o nullptr si todavía no toca lanzar otra.
     *
     * La primera llamada siempre devuelve una fruta, para no empezar con la
     * pantalla vacía.
     */
    std::unique_ptr<Fruit> trySpawn(float dt);

private:
    /**
     * @brief Construye una fruta con parámetros aleatorios válidos.
     * @return Fruta lista para entrar en el vector de objetos del juego.
     */
    std::unique_ptr<Fruit> spawnFruit();

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
};
