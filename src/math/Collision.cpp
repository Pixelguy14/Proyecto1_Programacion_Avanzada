#include "Collision.h"

#include <algorithm>

#if defined(FRUITNINJA_ENABLE_AVX) \
    && (defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86))
#  include <immintrin.h>
#  define FRUITNINJA_X86_AVX 1
#else
#  define FRUITNINJA_X86_AVX 0
#endif

// GCC/Clang permiten compilar una sola función con AVX aunque el resto del
// binario sea genérico; el despacho se hace en tiempo de ejecución. MSVC solo
// expone AVX si se compiló con /arch:AVX.
#if FRUITNINJA_X86_AVX && (defined(__GNUC__) || defined(__clang__))
#  define FRUITNINJA_AVX_TARGET __attribute__((target("avx")))
#  define FRUITNINJA_AVX_COMPILED 1
#elif FRUITNINJA_X86_AVX && defined(__AVX__)
#  define FRUITNINJA_AVX_TARGET
#  define FRUITNINJA_AVX_COMPILED 1
#else
#  define FRUITNINJA_AVX_COMPILED 0
#endif

/**
 * @file Collision.cpp
 * @brief Implementación de la intersección segmento-círculo y su lote AVX.
 */

namespace {

#if FRUITNINJA_AVX_COMPILED

/**
 * @brief Comprueba si el CPU actual soporta AVX.
 * @return true si la ruta vectorizada puede ejecutarse.
 *
 * La comprobación se cachea en una variable estática local: `__builtin_cpu_supports`
 * es barato, pero no hace falta repetirlo por cada fruta y cada frame.
 */
bool avxAvailable() {
    static const bool available = [] {
#if defined(__GNUC__) || defined(__clang__)
        return __builtin_cpu_supports("avx") != 0;
#else
        return true;  // MSVC solo llega aquí si compiló con /arch:AVX.
#endif
    }();
    return available;
}

/**
 * @brief Evalúa 8 segmentos contra un círculo con AVX.
 * @param segments Arreglo de al menos 8 segmentos.
 * @param center   Centro del círculo.
 * @param radius   Radio del círculo.
 * @return Máscara de 8 bits; el bit i está activo si el segmento i toca.
 */
FRUITNINJA_AVX_TARGET
std::uint32_t hitMask8(const Collision::Segment* segments,
                       const Vector2D<float>& center, float radius) {
    const __m256 vsx = _mm256_setr_ps(
        segments[0].start.x, segments[1].start.x, segments[2].start.x, segments[3].start.x,
        segments[4].start.x, segments[5].start.x, segments[6].start.x, segments[7].start.x);
    const __m256 vsy = _mm256_setr_ps(
        segments[0].start.y, segments[1].start.y, segments[2].start.y, segments[3].start.y,
        segments[4].start.y, segments[5].start.y, segments[6].start.y, segments[7].start.y);
    const __m256 vex = _mm256_setr_ps(
        segments[0].end.x, segments[1].end.x, segments[2].end.x, segments[3].end.x,
        segments[4].end.x, segments[5].end.x, segments[6].end.x, segments[7].end.x);
    const __m256 vey = _mm256_setr_ps(
        segments[0].end.y, segments[1].end.y, segments[2].end.y, segments[3].end.y,
        segments[4].end.y, segments[5].end.y, segments[6].end.y, segments[7].end.y);

    const __m256 vcx = _mm256_set1_ps(center.x);
    const __m256 vcy = _mm256_set1_ps(center.y);
    const __m256 vr2 = _mm256_set1_ps(radius * radius);
    const __m256 vzero = _mm256_setzero_ps();
    const __m256 vone = _mm256_set1_ps(1.0f);

    const __m256 segx = _mm256_sub_ps(vex, vsx);
    const __m256 segy = _mm256_sub_ps(vey, vsy);
    const __m256 tox = _mm256_sub_ps(vcx, vsx);
    const __m256 toy = _mm256_sub_ps(vcy, vsy);

    const __m256 lenSq = _mm256_add_ps(_mm256_mul_ps(segx, segx),
                                       _mm256_mul_ps(segy, segy));
    const __m256 dot = _mm256_add_ps(_mm256_mul_ps(tox, segx),
                                     _mm256_mul_ps(toy, segy));

    // Segmentos degenerados (inicio == fin): se usa t = 0 en vez de dividir por 0.
    const __m256 valid = _mm256_cmp_ps(lenSq, vzero, _CMP_GT_OQ);
    const __m256 safeLen = _mm256_blendv_ps(vone, lenSq, valid);

    __m256 t = _mm256_div_ps(dot, safeLen);
    t = _mm256_max_ps(t, vzero);
    t = _mm256_min_ps(t, vone);

    const __m256 closestX = _mm256_add_ps(vsx, _mm256_mul_ps(segx, t));
    const __m256 closestY = _mm256_add_ps(vsy, _mm256_mul_ps(segy, t));

    const __m256 dx = _mm256_sub_ps(vcx, closestX);
    const __m256 dy = _mm256_sub_ps(vcy, closestY);
    const __m256 distSq = _mm256_add_ps(_mm256_mul_ps(dx, dx),
                                        _mm256_mul_ps(dy, dy));

    const __m256 hit = _mm256_cmp_ps(distSq, vr2, _CMP_LE_OQ);
    return static_cast<std::uint32_t>(_mm256_movemask_ps(hit));
}

#endif  // FRUITNINJA_AVX_COMPILED

}  // namespace

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

Collision::SegmentHit Collision::firstSegmentHit(const Segment* segments, std::size_t count,
                                                 const Vector2D<float>& center, float radius) {
    if (segments == nullptr || count == 0) {
        return {};
    }

#if FRUITNINJA_AVX_COMPILED
    if (avxAvailable()) {
        std::size_t i = 0;
        for (; i + 8 <= count; i += 8) {
            const std::uint32_t mask = hitMask8(segments + i, center, radius);
            if (mask != 0u) {
                // El arreglo va de más reciente a más antiguo: el bit más bajo
                // es el primer segmento que tocó la fruta.
                std::size_t offset = 0;
                while ((mask & (1u << offset)) == 0u) {
                    ++offset;
                }
                return {true, i + offset};
            }
        }
        for (; i < count; ++i) {
            if (segmentIntersectsCircle(segments[i].start, segments[i].end, center, radius)) {
                return {true, i};
            }
        }
        return {};
    }
#endif

    for (std::size_t i = 0; i < count; ++i) {
        if (segmentIntersectsCircle(segments[i].start, segments[i].end, center, radius)) {
            return {true, i};
        }
    }
    return {};
}

bool Collision::usesAvx() {
#if FRUITNINJA_AVX_COMPILED
    return avxAvailable();
#else
    return false;
#endif
}
