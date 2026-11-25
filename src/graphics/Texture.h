#pragma once

/**
 * @file Texture.h
 * @brief Identificador y dimensiones de una textura 2D de OpenGL.
 *
 * El struct no libera la textura por sí solo: `Renderer::destroyTexture()` es
 * quien conoce el contexto OpenGL. Se mantiene como tipo de valor para poder
 * guardarlo en `std::array` o pasarlo por copia sin ownership implícito.
 */
struct Texture {
    unsigned int id = 0;  ///< Nombre de textura de OpenGL (0 = inválida).
    int width = 0;        ///< Ancho en píxeles.
    int height = 0;       ///< Alto en píxeles.

    /// @return true si la textura tiene un identificador válido.
    bool valid() const { return id != 0; }
};
