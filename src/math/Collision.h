#pragma once

#include "math/Vector2D.h"

/**
 * @file Collision.h
 * @brief Pruebas de intersección 2D para el corte de frutas (Sprint 3).
 *
 * La espada se modela como una polilínea: cada par de posiciones consecutivas
 * del historial de `Blade` forma un segmento. Una fruta es un círculo, así que
 * la detección se reduce a preguntar si algún segmento del tajo toca el disco.
 *
 *   Blade::cutAt()
 *     └── Collision::segmentIntersectsCircle()
 *           ├── proyecto el centro sobre el segmento
 *           └── comparo distancias al cuadrado (sin raíces)
 *
 * No hay dependencia de GLM ni de OpenGL: solo `Vector2D<float>`, lo que
 * permite probar la matemática de forma aislada.
 */
namespace Collision {

/**
 * @brief Comprueba si un segmento interseca (o toca) un círculo.
 * @param start  Extremo inicial del segmento.
 * @param end    Extremo final del segmento.
 * @param center Centro del círculo.
 * @param radius Radio del círculo (debe ser no negativo).
 * @return true si la distancia mínima del centro al segmento es menor o igual
 *         que el radio.
 *
 * El punto del segmento más cercano al centro se calcula proyectando el vector
 * `center - start` sobre la dirección del segmento y recortando el parámetro al
 * intervalo [0, 1]. Así el algoritmo cubre los tres casos: el centro cae
 * enfrente del segmento, sobre él, o más allá de un extremo.
 *
 * @code
 *   const bool hit = Collision::segmentIntersectsCircle(
 *       {100.0f, 100.0f}, {300.0f, 100.0f}, {200.0f, 140.0f}, 50.0f);
 * @endcode
 */
bool segmentIntersectsCircle(const Vector2D<float>& start,
                             const Vector2D<float>& end,
                             const Vector2D<float>& center,
                             float radius);

}  // namespace Collision
