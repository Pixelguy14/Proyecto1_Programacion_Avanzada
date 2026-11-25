#pragma once

#include "math/Vector2D.h"

#include <cstddef>
#include <cstdint>

/**
 * @file Collision.h
 * @brief Pruebas de intersección 2D para el corte de frutas (Sprints 3–4).
 *
 * La espada se modela como una polilínea: cada par de posiciones consecutivas
 * del historial de `Blade` forma un segmento. Una fruta es un círculo, así que
 * la detección se reduce a preguntar si algún segmento del tajo toca el disco.
 *
 *   Blade::cutAt()
 *     └── Collision::firstSegmentHit()
 *           ├── ruta AVX: 8 segmentos por iteración (si el CPU la soporta)
 *           └── ruta escalar: respaldo portable
 *
 * No hay dependencia de GLM ni de OpenGL: solo `Vector2D<float>`, lo que
 * permite probar la matemática de forma aislada.
 */
namespace Collision {

/**
 * @brief Segmento recto entre dos posiciones del recorrido de la espada.
 */
struct Segment {
    Vector2D<float> start;  ///< Extremo inicial.
    Vector2D<float> end;    ///< Extremo final.
};

/**
 * @brief Resultado de buscar el primer segmento que toca un círculo.
 */
struct SegmentHit {
    bool hit = false;        ///< true si algún segmento interseca el círculo.
    std::size_t index = 0;   ///< Índice del primer segmento que tocó.
};

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

/**
 * @brief Busca el primer segmento del arreglo que toca un círculo.
 * @param segments Arreglo contiguo de segmentos, ordenado del más reciente al
 *                 más antiguo.
 * @param count    Número de segmentos en el arreglo.
 * @param center   Centro del círculo (la fruta).
 * @param radius   Radio del círculo en píxeles.
 * @return `hit` y el índice del primer segmento que interseca.
 *
 * El índice devuelto siempre es el menor posible, es decir, el segmento más
 * reciente que tocó la fruta, igual que recorrer el arreglo de principio a fin.
 *
 * Cuando el CPU y la compilación lo permiten, evalúa 8 segmentos a la vez con
 * instrucciones AVX; en caso contrario usa la ruta escalar. El resultado es
 * idéntico en ambas rutas.
 */
SegmentHit firstSegmentHit(const Segment* segments, std::size_t count,
                           const Vector2D<float>& center, float radius);

/**
 * @brief Indica si la ruta AVX está activa en este proceso.
 * @return true si el binario incluye AVX y el CPU actual lo soporta.
 *
 * Se usa solo para diagnóstico; la detección es transparente para el juego.
 */
bool usesAvx();

}  // namespace Collision
