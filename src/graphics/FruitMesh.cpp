#include "FruitMesh.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <utility>

/**
 * @file FruitMesh.cpp
 * @brief Generación procedural de las mallas low-poly de las frutas.
 *
 * Todas las mallas usan triángulos independientes con la normal de la cara
 * (flat shading) y un color por triángulo. La orientación de cada cara se
 * corrige comparándola con un vector "hacia fuera", de modo que el orden de
 * los vértices no depende de la parametrización de cada primitiva.
 */

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530718f;

/// Función que decide el color de un punto de la esfera.
using ColorFn = std::function<glm::vec3(float latitude, float longitude)>;

/**
 * @brief Mezcla lineal entre dos colores.
 * @param a Color inicial.
 * @param b Color final.
 * @param t Factor en [0, 1].
 * @return Color interpolado.
 */
glm::vec3 mixColor(const glm::vec3& a, const glm::vec3& b, float t) {
    return a + (b - a) * glm::clamp(t, 0.0f, 1.0f);
}

/**
 * @brief Añade un triángulo con normal plana y corrige su orientación.
 * @param mesh        Malla destino.
 * @param a,b,c       Vértices del triángulo.
 * @param color       Color de la cara.
 * @param outwardHint Vector que apunta hacia fuera de la superficie.
 *
 * Si la normal calculada apunta en contra de `outwardHint`, se intercambian
 * dos vértices. Así las primitivas no dependen de un orden de anillo concreto.
 */
void addTriangle(MeshData& mesh, glm::vec3 a, glm::vec3 b, glm::vec3 c,
                 const glm::vec3& color, const glm::vec3& outwardHint) {
    glm::vec3 cross = glm::cross(b - a, c - a);
    if (glm::dot(cross, outwardHint) < 0.0f) {
        std::swap(b, c);
        cross = -cross;
    }

    const float lengthSquared = glm::dot(cross, cross);
    if (lengthSquared < 1e-10f) {
        return;  // triángulo degenerado (polos o anillos que colapsan)
    }

    const glm::vec3 normal = cross / std::sqrt(lengthSquared);
    const auto base = static_cast<std::uint32_t>(mesh.vertices.size());
    mesh.vertices.push_back({a, normal, color});
    mesh.vertices.push_back({b, normal, color});
    mesh.vertices.push_back({c, normal, color});
    mesh.indices.push_back(base);
    mesh.indices.push_back(base + 1);
    mesh.indices.push_back(base + 2);
}

/**
 * @brief Añade un quad como dos triángulos con la misma normal por cara.
 * @param mesh        Malla destino.
 * @param a,b,c,d     Vértices en orden alrededor del quad.
 * @param color       Color de la cara.
 * @param outwardHint Vector hacia fuera.
 */
void addQuad(MeshData& mesh, const glm::vec3& a, const glm::vec3& b,
             const glm::vec3& c, const glm::vec3& d, const glm::vec3& color,
             const glm::vec3& outwardHint) {
    addTriangle(mesh, a, b, c, color, outwardHint);
    addTriangle(mesh, a, c, d, color, outwardHint);
}

/**
 * @brief Añade los vértices e índices de otra malla al final de `dst`.
 * @param dst Malla destino.
 * @param src Malla origen.
 */
void appendMesh(MeshData& dst, const MeshData& src) {
    const auto offset = static_cast<std::uint32_t>(dst.vertices.size());
    dst.vertices.insert(dst.vertices.end(), src.vertices.begin(), src.vertices.end());
    for (const std::uint32_t index : src.indices) {
        dst.indices.push_back(index + offset);
    }
}

/**
 * @brief Genera una esfera low-poly con polos en Y.
 * @param sectors  Divisiones alrededor del eje Y.
 * @param stacks   Divisiones de polo a polo.
 * @param scale    Escala por eje (permite óvalos).
 * @param dimple   Hundimiento de los polos (0 = esfera, 0.3 = manzana).
 * @param colorFn  Color de cada cara según latitud/longitud.
 * @return Malla con normales planas.
 */
MeshData makeSphere(int sectors, int stacks, const glm::vec3& scale, float dimple,
                    const ColorFn& colorFn) {
    MeshData mesh;

    const auto point = [&](float latitude, float longitude) {
        if (latitude <= 1e-5f) {
            return glm::vec3{0.0f, scale.y * (1.0f - dimple), 0.0f};
        }
        if (latitude >= kPi - 1e-5f) {
            return glm::vec3{0.0f, -scale.y * (1.0f - dimple), 0.0f};
        }
        const float s = std::sin(latitude);
        return glm::vec3{
            s * std::cos(longitude) * scale.x,
            std::cos(latitude) * scale.y,
            s * std::sin(longitude) * scale.z,
        };
    };

    for (int i = 0; i < stacks; ++i) {
        const float lat0 = kPi * static_cast<float>(i) / static_cast<float>(stacks);
        const float lat1 = kPi * static_cast<float>(i + 1) / static_cast<float>(stacks);

        for (int j = 0; j < sectors; ++j) {
            const float lon0 = kTwoPi * static_cast<float>(j) / static_cast<float>(sectors);
            const float lon1 = kTwoPi * static_cast<float>(j + 1) / static_cast<float>(sectors);

            const glm::vec3 p00 = point(lat0, lon0);
            const glm::vec3 p01 = point(lat0, lon1);
            const glm::vec3 p10 = point(lat1, lon0);
            const glm::vec3 p11 = point(lat1, lon1);
            const glm::vec3 color = colorFn((lat0 + lat1) * 0.5f, (lon0 + lon1) * 0.5f);

            if (i == 0) {
                addTriangle(mesh, p00, p10, p11, color, (p00 + p10 + p11) / 3.0f);
            } else if (i == stacks - 1) {
                addTriangle(mesh, p00, p10, p01, color, (p00 + p10 + p01) / 3.0f);
            } else {
                addQuad(mesh, p00, p01, p11, p10, color,
                        (p00 + p01 + p11 + p10) / 4.0f);
            }
        }
    }

    return mesh;
}

/**
 * @brief Añade un cilindro (o tubo) redondeado a una malla.
 * @param mesh   Malla destino.
 * @param base   Centro de la base inferior.
 * @param axis   Vector de la base a la tapa.
 * @param radius Radio del cilindro.
 * @param sides  Número de lados.
 * @param color  Color de la superficie.
 * @param cap    true para cerrar la tapa superior.
 */
void addCylinder(MeshData& mesh, const glm::vec3& base, const glm::vec3& axis,
                 float radius, int sides, const glm::vec3& color, bool cap) {
    const glm::vec3 n = glm::normalize(axis);
    const glm::vec3 reference = std::abs(n.y) < 0.9f ? glm::vec3{0.0f, 1.0f, 0.0f}
                                                      : glm::vec3{1.0f, 0.0f, 0.0f};
    const glm::vec3 u = glm::normalize(glm::cross(reference, n));
    const glm::vec3 v = glm::cross(n, u);

    std::vector<glm::vec3> ring0;
    std::vector<glm::vec3> ring1;
    ring0.reserve(static_cast<std::size_t>(sides));
    ring1.reserve(static_cast<std::size_t>(sides));

    for (int i = 0; i < sides; ++i) {
        const float angle = kTwoPi * static_cast<float>(i) / static_cast<float>(sides);
        const glm::vec3 offset = u * (std::cos(angle) * radius)
                               + v * (std::sin(angle) * radius);
        ring0.push_back(base + offset);
        ring1.push_back(base + axis + offset);
    }

    for (int i = 0; i < sides; ++i) {
        const int j = (i + 1) % sides;
        const glm::vec3 faceCenter = (ring0[i] + ring0[j] + ring1[j] + ring1[i]) * 0.25f;
        const glm::vec3 radial = (faceCenter - base) - n * glm::dot(faceCenter - base, n);
        addQuad(mesh, ring0[i], ring0[j], ring1[j], ring1[i], color, radial);
    }

    if (cap) {
        const glm::vec3 tip = base + axis;
        for (int i = 0; i < sides; ++i) {
            const int j = (i + 1) % sides;
            addTriangle(mesh, tip, ring1[i], ring1[j], color, n);
        }
    }
}

/**
 * @brief Añade un cono a una malla.
 * @param mesh       Malla destino.
 * @param baseCenter Centro del anillo de la base.
 * @param axis       Vector de la base a la punta.
 * @param radius     Radio de la base.
 * @param sides      Número de lados.
 * @param color      Color de la superficie.
 *
 * No se genera la tapa de la base porque queda oculta dentro de la fruta.
 */
void addCone(MeshData& mesh, const glm::vec3& baseCenter, const glm::vec3& axis,
             float radius, int sides, const glm::vec3& color) {
    const glm::vec3 n = glm::normalize(axis);
    const glm::vec3 reference = std::abs(n.y) < 0.9f ? glm::vec3{0.0f, 1.0f, 0.0f}
                                                      : glm::vec3{1.0f, 0.0f, 0.0f};
    const glm::vec3 u = glm::normalize(glm::cross(reference, n));
    const glm::vec3 v = glm::cross(n, u);
    const glm::vec3 tip = baseCenter + axis;

    std::vector<glm::vec3> ring;
    ring.reserve(static_cast<std::size_t>(sides));
    for (int i = 0; i < sides; ++i) {
        const float angle = kTwoPi * static_cast<float>(i) / static_cast<float>(sides);
        ring.push_back(baseCenter + u * (std::cos(angle) * radius)
                                 + v * (std::sin(angle) * radius));
    }

    for (int i = 0; i < sides; ++i) {
        const int j = (i + 1) % sides;
        const glm::vec3 faceCenter = (tip + ring[i] + ring[j]) / 3.0f;
        const glm::vec3 radial = (faceCenter - baseCenter)
            - n * glm::dot(faceCenter - baseCenter, n);
        addTriangle(mesh, tip, ring[i], ring[j], color, radial);
    }
}

/**
 * @brief Añade un tallo corto (cilindro marrón).
 * @param mesh   Malla destino.
 * @param base   Centro de la base.
 * @param height Altura del tallo.
 * @param radius Radio.
 * @param color  Color del tallo.
 */
void addStem(MeshData& mesh, const glm::vec3& base, float height, float radius,
             const glm::vec3& color) {
    addCylinder(mesh, base, glm::vec3{0.0f, height, 0.0f}, radius, 5, color, true);
}

/**
 * @brief Genera el plátano curvado con anillos de sección poligonal.
 * @return Malla del plátano.
 */
MeshData makeBanana() {
    constexpr int kSegments = 10;
    constexpr int kSides = 8;
    constexpr float kLength = 2.15f;
    constexpr float kArch = 0.62f;
    const glm::vec3 baseColor{0.98f, 0.82f, 0.24f};
    const glm::vec3 lightColor{1.00f, 0.92f, 0.48f};
    const glm::vec3 tipColor{0.52f, 0.62f, 0.18f};
    const glm::vec3 stemColor{0.36f, 0.46f, 0.16f};

    MeshData mesh;

    struct Ring {
        glm::vec3 center;
        std::vector<glm::vec3> vertices;
    };

    std::vector<Ring> rings;
    rings.reserve(kSegments + 1);

    for (int i = 0; i <= kSegments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kSegments);
        const glm::vec3 center{
            (t - 0.5f) * kLength,
            kArch * (1.0f - (2.0f * t - 1.0f) * (2.0f * t - 1.0f)) - kArch * 0.5f,
            0.0f,
        };

        const float dy = -4.0f * kArch * (2.0f * t - 1.0f);
        const glm::vec3 tangent = glm::normalize(glm::vec3{kLength, dy, 0.0f});
        const glm::vec3 side = glm::normalize(glm::vec3{-tangent.y, tangent.x, 0.0f});
        const glm::vec3 up{0.0f, 0.0f, 1.0f};

        // Más grueso en el centro y ligeramente más estrecho hacia la punta.
        const float asymmetry = 0.90f + 0.10f * (1.0f - t);
        const float radius = 0.38f * std::pow(std::sin(kPi * t), 0.52f) * asymmetry;

        Ring ring;
        ring.center = center;
        ring.vertices.reserve(static_cast<std::size_t>(kSides));
        for (int j = 0; j < kSides; ++j) {
            const float angle = kTwoPi * static_cast<float>(j) / static_cast<float>(kSides);
            ring.vertices.push_back(center + side * (std::cos(angle) * radius)
                                            + up * (std::sin(angle) * radius * 0.92f));
        }
        rings.push_back(std::move(ring));
    }

    for (int i = 0; i < kSegments; ++i) {
        const float t = (static_cast<float>(i) + 0.5f) / static_cast<float>(kSegments);
        const float light = 0.5f + 0.5f * std::sin(kPi * t);
        const float tip = std::pow(std::abs(2.0f * t - 1.0f), 3.0f);
        const glm::vec3 color = mixColor(mixColor(baseColor, lightColor, light),
                                         tipColor, tip * 0.55f);
        const glm::vec3 ringCenter = (rings[i].center + rings[i + 1].center) * 0.5f;

        for (int j = 0; j < kSides; ++j) {
            const int k = (j + 1) % kSides;
            const glm::vec3 a = rings[i].vertices[j];
            const glm::vec3 b = rings[i].vertices[k];
            const glm::vec3 c = rings[i + 1].vertices[k];
            const glm::vec3 d = rings[i + 1].vertices[j];
            const glm::vec3 faceCenter = (a + b + c + d) * 0.25f;
            // Las caras alternas marcan las aristas longitudinales del plátano.
            const glm::vec3 sideColor = (j % 2 == 0) ? color : color * 0.86f;
            addQuad(mesh, a, b, c, d, sideColor, faceCenter - ringCenter);
        }
    }

    // Tallo en un extremo y punta oscura en el otro. Ambos se hunden un poco
    // en el cuerpo para que no quede una separación visible.
    const glm::vec3 startCenter = rings.front().center;
    const glm::vec3 startTangent = glm::normalize(
        glm::vec3{kLength, 4.0f * kArch, 0.0f});
    addCylinder(mesh, startCenter + startTangent * 0.10f,
                -startTangent * 0.34f, 0.060f, 5, stemColor, true);

    const glm::vec3 endCenter = rings.back().center;
    const glm::vec3 endTangent = glm::normalize(
        glm::vec3{kLength, -4.0f * kArch, 0.0f});
    addCylinder(mesh, endCenter - endTangent * 0.08f,
                endTangent * 0.18f, 0.040f, 5, tipColor * 0.75f, true);

    return mesh;
}

}  // namespace

FruitAppearance fruitAppearance(FruitMeshId id) {
    switch (id) {
        case FruitMeshId::kMelon:
            return {{0.24f, 0.48f, 0.20f}, {0.90f, 0.22f, 0.21f}};
        case FruitMeshId::kApple:
            return {{0.78f, 0.16f, 0.15f}, {0.96f, 0.90f, 0.72f}};
        case FruitMeshId::kOrange:
            return {{0.95f, 0.52f, 0.12f}, {0.98f, 0.72f, 0.30f}};
        case FruitMeshId::kLemon:
            return {{0.97f, 0.85f, 0.18f}, {0.99f, 0.95f, 0.60f}};
        case FruitMeshId::kBanana:
            return {{0.96f, 0.80f, 0.22f}, {0.99f, 0.94f, 0.72f}};
        case FruitMeshId::kBomb:
            return {{0.08f, 0.08f, 0.10f}, {0.08f, 0.08f, 0.10f}};
    }
    return {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}};
}

MeshData makeFruitMesh(FruitMeshId id) {
    switch (id) {
        case FruitMeshId::kMelon: {
            constexpr int kSectors = 12;
            MeshData mesh = makeSphere(kSectors, 8, {0.95f, 1.15f, 0.95f}, 0.04f,
                [kSectors](float latitude, float longitude) {
                    const int stripe = static_cast<int>(
                        std::floor(longitude / (kTwoPi / static_cast<float>(kSectors)))) % 2;
                    const glm::vec3 dark{0.18f, 0.40f, 0.16f};
                    const glm::vec3 light{0.34f, 0.58f, 0.24f};
                    const float shade = 0.88f + 0.12f * std::cos(latitude);
                    return (stripe == 0 ? dark : light) * shade;
                });
            addStem(mesh, {0.0f, 1.10f, 0.0f}, 0.16f, 0.035f, {0.35f, 0.24f, 0.10f});
            return mesh;
        }
        case FruitMeshId::kApple: {
            MeshData mesh = makeSphere(10, 7, {0.98f, 0.98f, 0.98f}, 0.28f,
                [](float latitude, float /*longitude*/) {
                    const float top = 0.5f + 0.5f * std::cos(latitude);
                    return mixColor({0.55f, 0.08f, 0.08f}, {0.90f, 0.24f, 0.18f}, top);
                });
            addStem(mesh, {0.0f, 0.70f, 0.0f}, 0.28f, 0.045f, {0.35f, 0.23f, 0.10f});
            return mesh;
        }
        case FruitMeshId::kOrange: {
            return makeSphere(10, 7, {1.0f, 0.95f, 1.0f}, 0.0f,
                [](float latitude, float /*longitude*/) {
                    const float top = 0.5f + 0.5f * std::cos(latitude);
                    return mixColor({0.88f, 0.42f, 0.08f}, {0.99f, 0.64f, 0.16f}, top);
                });
        }
        case FruitMeshId::kLemon: {
            MeshData mesh = makeSphere(10, 7, {1.05f, 0.85f, 0.85f}, 0.0f,
                [](float latitude, float /*longitude*/) {
                    const float top = 0.5f + 0.5f * std::cos(latitude);
                    return mixColor({0.92f, 0.78f, 0.12f}, {0.99f, 0.92f, 0.32f}, top);
                });
            addCone(mesh, {0.95f, 0.0f, 0.0f}, {0.42f, 0.0f, 0.0f}, 0.24f, 8,
                    {0.98f, 0.90f, 0.28f});
            addCone(mesh, {-0.95f, 0.0f, 0.0f}, {-0.42f, 0.0f, 0.0f}, 0.24f, 8,
                    {0.98f, 0.90f, 0.28f});
            return mesh;
        }
        case FruitMeshId::kBanana:
            return makeBanana();
        case FruitMeshId::kBomb: {
            MeshData mesh = makeSphere(10, 7, {0.96f, 0.96f, 0.96f}, 0.0f,
                [](float latitude, float /*longitude*/) {
                    const float belt = std::exp(-std::pow((latitude - kPi * 0.5f) / 0.38f, 2.0f));
                    return mixColor({0.06f, 0.06f, 0.08f}, {0.55f, 0.12f, 0.10f}, belt);
                });
            addCylinder(mesh, {0.0f, 0.86f, 0.0f}, {0.0f, 0.44f, 0.0f}, 0.055f, 5,
                        {0.38f, 0.25f, 0.12f}, true);
            MeshData spark = makeSphere(6, 4, {0.13f, 0.13f, 0.13f}, 0.0f,
                [](float /*latitude*/, float /*longitude*/) {
                    return glm::vec3{1.0f, 0.68f, 0.16f};
                });
            // La chispa se coloca en la punta de la mecha.
            for (MeshVertex& vertex : spark.vertices) {
                vertex.position.y += 1.36f;
            }
            appendMesh(mesh, spark);
            return mesh;
        }
    }
    return {};
}
