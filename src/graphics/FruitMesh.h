#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @file FruitMesh.h
 * @brief Mallas low-poly procedurales de las frutas (Sprint 5, Fase C).
 *
 * La jugabilidad sigue siendo 2D (posición, física y colisión), pero el cuerpo
 * de cada fruta se dibuja como una malla 3D con flat shading. Las mallas se
 * generan una sola vez al arrancar y se reutilizan para todas las instancias.
 *
 * Cada vértice lleva posición, normal de cara y color; el Renderer las sube a
 * un VAO/VBO/EBO y las dibuja con un shader de luz direccional.
 */

/// Tipos de malla disponibles. El valor se usa como índice en el Renderer.
enum class FruitMeshId : std::uint8_t {
    kMelon,   ///< Melón ovalado con franjas verdes.
    kApple,   ///< Manzana con hendidura y tallo.
    kOrange,  ///< Naranja casi esférica.
    kLemon,   ///< Limón alargado con puntas.
    kBanana,  ///< Plátano curvado.
    kBomb,    ///< Bomba oscura con mecha y chispa.
};

/// Número de mallas registradas en el Renderer.
constexpr std::size_t kFruitMeshCount =
    static_cast<std::size_t>(FruitMeshId::kBomb) + 1;

/// Vértice de una malla low-poly.
struct MeshVertex {
    glm::vec3 position;  ///< Posición local (la fruta se escala después).
    glm::vec3 normal;    ///< Normal de la cara (flat shading).
    glm::vec3 color;     ///< Color RGB en [0, 1].
};

/// Datos de malla en CPU, listos para subir a OpenGL.
struct MeshData {
    std::vector<MeshVertex> vertices;   ///< Vértices duplicados por cara.
    std::vector<std::uint32_t> indices; ///< Índices de triángulos.
};

/// Colores de cáscara y pulpa de cada fruta.
struct FruitAppearance {
    glm::vec3 shell;  ///< Color exterior (cáscara).
    glm::vec3 flesh;  ///< Color interior (jugo y mitades).
};

/**
 * @brief Devuelve los colores de cáscara y pulpa de un tipo de fruta.
 * @param id Tipo de malla.
 * @return Colores RGB en [0, 1].
 */
FruitAppearance fruitAppearance(FruitMeshId id);

/**
 * @brief Genera la malla procedural de una fruta.
 * @param id Tipo de malla.
 * @return Vértices e índices con normales planas y color por cara.
 *
 * Las mallas caben aproximadamente en una esfera unitaria; el Renderer las
 * escala con el radio de la fruta. La generación es determinista.
 */
MeshData makeFruitMesh(FruitMeshId id);
