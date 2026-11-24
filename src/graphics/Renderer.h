#pragma once

#include <glm/glm.hpp>

/**
 * @file Renderer.h
 * @brief Primitivas de dibujado 2D para el juego (Sprint 1).
 *
 * ## Jerarquía de responsabilidades
 * El Renderer conoce OpenGL (shaders, VAO/VBO, uniforms) y expone una API
 * sencilla en coordenadas de pantalla. Las entidades solo piden "dibuja un
 * círculo aquí" o "dibuja un rectángulo rotado"; no tocan OpenGL.
 *
 *   Fruit::render()
 *     └── Renderer::drawCircle() / drawRect()
 *           └── shader + VAO + uniforms
 *
 * La proyección ortográfica mapea (0..width, 0..height) a NDC, con Y hacia
 * arriba, de modo que las posiciones del juego se expresan en píxeles.
 */
class Renderer {
public:
    /**
     * @brief Inicializa shaders, proyección y geometría base.
     * @param screenWidth  Ancho lógico de la ventana en píxeles.
     * @param screenHeight Alto lógico de la ventana en píxeles.
     * @return true si todos los recursos se crearon correctamente.
     */
    bool init(int screenWidth, int screenHeight);

    /**
     * @brief Libera los recursos de GPU (VAOs, VBOs y shader program).
     *
     * Es seguro llamarlo aunque init() haya fallado parcialmente.
     */
    void cleanup();

    /**
     * @brief Dibuja un círculo relleno.
     * @param center Centro en píxeles de pantalla.
     * @param radius Radio en píxeles.
     * @param color  Color RGB en el rango [0, 1].
     */
    void drawCircle(const glm::vec2& center, float radius, const glm::vec3& color);

    /**
     * @brief Dibuja un rectángulo relleno, opcionalmente rotado.
     * @param center        Centro del rectángulo en píxeles.
     * @param size          Tamaño completo (ancho, alto) en píxeles.
     * @param angleRadians  Rotación en radianes (positiva = antihorario).
     * @param color         Color RGB en el rango [0, 1].
     */
    void drawRect(const glm::vec2& center, const glm::vec2& size,
                  float angleRadians, const glm::vec3& color);

private:
    /**
     * @brief Compila el vertex/fragment shader y enlaza el shader program.
     * @return true si la compilación y el enlazado tuvieron éxito.
     */
    bool buildShaderProgram();

    /**
     * @brief Configura la matriz ortográfica y la envía al shader como uniform.
     * @param screenWidth  Ancho lógico de la ventana.
     * @param screenHeight Alto lógico de la ventana.
     */
    void uploadProjection(int screenWidth, int screenHeight);

    /**
     * @brief Crea los VAO/VBO del círculo unitario y del quad unitario.
     * @return true si los buffers se generaron correctamente.
     */
    bool setupGeometry();

    /**
     * @brief Envía la transformación y el color, y dibuja la malla indicada.
     * @param vao         VAO a dibujar.
     * @param vertexCount Número de vértices.
     * @param mode        GL_TRIANGLE_FAN o GL_TRIANGLE_STRIP.
     * @param model       Matriz modelo (escala, rotación, traslación).
     * @param color       Color RGB.
     */
    void drawPrimitive(unsigned int vao, unsigned int vertexCount, unsigned int mode,
                       const glm::mat4& model, const glm::vec3& color);

private:
    unsigned int m_circleVAO = 0;  ///< VAO del círculo unitario (radio 1).
    unsigned int m_circleVBO = 0;  ///< VBO del círculo unitario.
    unsigned int m_rectVAO = 0;    ///< VAO del quad unitario (-1..1).
    unsigned int m_rectVBO = 0;    ///< VBO del quad unitario.
    unsigned int m_circleVertexCount = 0;  ///< Vértices del abanico del círculo.

    unsigned int m_shaderProgram = 0;
    int m_projectionLocation = -1;
    int m_modelLocation = -1;
    int m_colorLocation = -1;
};
