#include "Collision.h"

#include <algorithm>

/**
 * @file Collision.cpp
 * @brief Implementación de la intersección segmento-círculo.
 */

bool Collision::segmentIntersectsCircle(const Vector2D<float>& start,
                                        const Vector2D<float>& end,
                                        const Vector2D<float>& center,
                                        float radius) {
    // Vector del segmento y del inicio al centro del círculo.
    const Vector2D<float> segment = end - start;
    const Vector2D<float> toCenter = center - start;

    const float lengthSquared = segment.lengthSquared();

    // Proyección del centro sobre la recta del segmento, recortada al segmento.
    // Si el segmento es degenerado (inicio == fin), se usa el propio inicio.
    float t = 0.0f;
    if (lengthSquared > 0.0f) {
        t = std::clamp(toCenter.dot(segment) / lengthSquared, 0.0f, 1.0f);
    }

    const Vector2D<float> closest = start + segment * t;

    // Comparación al cuadrado: evita la raíz y es exacta para el borde.
    return (center - closest).lengthSquared() <= radius * radius;
}
