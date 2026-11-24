#pragma once

#include <glm/glm.hpp>

#include <vector>

/**
 * @file Renderer.h
 * @brief Primitivas de dibujado 2D para el juego (Sprints 1–3).
 *
 * ## Jerarquía de responsabilidades
 * El Renderer conoce OpenGL (shaders, VAO/VBO, uniforms) y expone una API
 * sencilla en coordenadas de pantalla. Las entidades solo piden "dibuja un
 * círculo aquí", "dibuja esta estela"; no tocan OpenGL.
 *
 *   Fruit::render()      → drawCircle() / drawRect()
 *   FruitHalf::render()  → drawSemicircle()
 *   Blade::render()      → drawTrail()
 *                            └── shader + VAO dinámico + blending
 *
 * La proyección ortográfica mapea (0..width, 0..height) a NDC, con Y hacia
 * arriba, de modo que las posiciones del juego se expresan en píxeles.
 */
class Renderer {
public:
    /**
     * @brief Punto de una estela: la espada decide posición, grosor y alpha.
     */
    struct TrailPoint {
        glm::vec2 position;  ///< Centro del segmento en píxeles.
        float width;         ///< Grosor del segmento en píxeles.
        float alpha;         ///< Opacidad [0, 1]; 1 = totalmente opaco.
    };

    /**
     * @brief Inicializa shaders, proyección y geometría base.
     * @param screenWidth  Ancho lógico de la ventana en píxeles.
     * @param screenHeight Alto lógico de la ventana en píxeles.
     * @return true si todos los recursos se crearon correctamente.
     */
    bool init(int screenWidth, int screenHeight);

    /**
     * @brief Libera los recursos de GPU (VAOs, VBOs y shader programs).
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

    /**
     * @brief Dibuja un semicírculo relleno (mitad de fruta).
     * @param center       Centro del diámetro en píxeles.
     * @param radius       Radio en píxeles.
     * @param angleRadians Orientación del diámetro (positiva = antihorario).
     * @param color        Color RGB en el rango [0, 1].
     *
     * La malla unitaria ocupa la mitad superior del disco (de 0 a π) con el
     * diámetro sobre el eje X; `angleRadians` lo alinea con la dirección del
     * corte. Para la mitad opuesta basta sumar π al ángulo.
     */
    void drawSemicircle(const glm::vec2& center, float radius, float angleRadians,
                        const glm::vec3& color);

    /**
     * @brief Dibuja una estela como cinta (triangle strip) con grosor y alpha.
     * @param points Puntos ordenados del más antiguo al más reciente.
     * @param color  Color RGBA base en el rango [0, 1].
     *
     * Cada punto aporta su propio grosor y alpha; el color se interpola con
     * blending sobre lo que ya está dibujado. Se necesitan al menos 2 puntos.
     */
    void drawTrail(const std::vector<TrailPoint>& points, const glm::vec4& color);

private:
    /**
     * @brief Compila el shader de primitivas sólidas (círculo/rectángulo).
     * @return true si el shader program quedó listo.
     */
    bool buildShaderProgram();

    /**
     * @brief Compila el shader de la estela (posición + alpha por vértice).
     * @return true si el shader program quedó listo.
     */
    bool buildTrailProgram();

    /**
     * @brief Configura la matriz ortográfica y la envía a ambos shaders.
     * @param screenWidth  Ancho lógico de la ventana.
     * @param screenHeight Alto lógico de la ventana.
     */
    void uploadProjection(int screenWidth, int screenHeight);

    /**
     * @brief Crea las mallas: círculo, semicírculo, quad y VBO dinámico de la estela.
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
    unsigned int m_semiVAO = 0;    ///< VAO del semicírculo unitario.
    unsigned int m_semiVBO = 0;    ///< VBO del semicírculo unitario.
    unsigned int m_circleVertexCount = 0;  ///< Vértices del abanico del círculo.
    unsigned int m_semiVertexCount = 0;    ///< Vértices del abanico del semicírculo.

    unsigned int m_trailVAO = 0;   ///< VAO de la estela dinámica.
    unsigned int m_trailVBO = 0;   ///< VBO de la estela (se reescribe cada frame).

    unsigned int m_shaderProgram = 0;
    int m_projectionLocation = -1;
    int m_modelLocation = -1;
    int m_colorLocation = -1;

    unsigned int m_trailProgram = 0;
    int m_trailProjectionLocation = -1;
    int m_trailColorLocation = -1;
};
