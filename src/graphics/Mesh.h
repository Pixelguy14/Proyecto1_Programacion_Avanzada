#pragma once

#include <cstddef>

/**
 * @file Mesh.h
 * @brief Identificadores de GPU de una malla estática (Sprint 5, Fase C).
 *
 * `Mesh` solo guarda los nombres de OpenGL y el número de índices. El Renderer
 * es quien crea, dibuja y libera estos objetos.
 */
struct Mesh {
    unsigned int vao = 0;        ///< Vertex Array Object.
    unsigned int vbo = 0;        ///< Vertex Buffer Object.
    unsigned int ebo = 0;        ///< Element Buffer Object (índices).
    std::size_t indexCount = 0;  ///< Número de índices a dibujar.

    /// @return true si la malla tiene todos sus recursos de GPU.
    bool valid() const { return vao != 0 && vbo != 0 && ebo != 0 && indexCount > 0; }
};
