#pragma once

/**
 * @file Renderer.h
 * @brief Declaración del Renderer básico para el Sprint 0.
 *
 * Por ahora solo dibuja un rectángulo rojo centrado usando un VAO/VBO
 * y una proyección ortográfica. Se irá ampliando en sprints posteriores.
 */
class Renderer {
public:
    /**
     * @brief Inicializa el VAO, VBO y compila los shaders básicos.
     */
    void init();

    /**
     * @brief Dibuja el rectángulo en pantalla.
     */
    void render();

    /**
     * @brief Libera los recursos de GPU (VAO, VBO, shader program).
     */
    void cleanup();

private:
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_shaderProgram = 0;
};
