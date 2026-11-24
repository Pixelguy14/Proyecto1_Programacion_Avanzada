#pragma once

/**
 * @file Renderer.h
 * @brief Declaración del Renderer básico para el Sprint 0.
 *
 * ## Jerarquía de responsabilidades
 * El Renderer solo sabe dibujar la geometría del Sprint 0 (un rectángulo
 * estático). Los shaders, el VAO/VBO/EBO y la proyección ortográfica viven
 * aquí; la ventana y el bucle principal no conocen detalles de OpenGL.
 *
 *   main()
 *     └── Renderer  (shaders, geometría, proyección, dibujado, limpieza)
 *
 * Se irá ampliando en sprints posteriores (Shader, Texture, primitivas).
 */
class Renderer {
public:
    /**
     * @brief Inicializa el renderer: compila shaders, configura la proyección
     *        ortográfica y sube la geometría a la GPU.
     * @return true si todos los recursos se crearon correctamente.
     */
    bool init();

    /**
     * @brief Dibuja el rectángulo estático en pantalla.
     *
     * Asume que el color de fondo ya se limpió con Window::clear().
     */
    void render();

    /**
     * @brief Libera los recursos de GPU (VAO, VBO, EBO y shader program).
     *
     * Es seguro llamarlo aunque init() haya fallado parcialmente.
     */
    void cleanup();

private:
    /**
     * @brief Compila el vertex/fragment shader y enlaza el shader program.
     * @return true si la compilación y el enlazado tuvieron éxito.
     */
    bool buildShaderProgram();

    /**
     * @brief Define la matriz ortográfica y la envía al shader como uniform.
     *
     * Mapea las coordenadas de pantalla (0..width, 0..height) a NDC.
     */
    void uploadProjection();

    /**
     * @brief Crea el VAO, el VBO y el EBO con los vértices del rectángulo.
     * @return true si los buffers se generaron correctamente.
     */
    bool setupGeometry();

private:
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;
    unsigned int m_shaderProgram = 0;
};
