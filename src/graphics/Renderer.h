#pragma once

#include "graphics/FruitMesh.h"
#include "graphics/Mesh.h"
#include "graphics/Texture.h"

#include <glm/glm.hpp>

#include <array>
#include <string>
#include <vector>

/**
 * @file Renderer.h
 * @brief Primitivas 2D, texturas y mallas 3D para el juego (Sprints 1–5).
 *
 * ## Jerarquía de responsabilidades
 * El Renderer conoce OpenGL (shaders, VAO/VBO, uniforms) y expone una API
 * sencilla en coordenadas de pantalla. Las entidades solo piden "dibuja un
 * círculo aquí", "dibuja esta estela", "dibuja este fondo" o "dibuja esta
 * fruta 3D"; no tocan OpenGL.
 *
 *   Fruit::render()      → drawMesh() (malla low-poly)
 *   FruitHalf::render()  → drawPolygon()
 *   Blade::render()      → drawTrail()
 *   Background::render() → drawTexture()
 *   GameUi::render()     → TextRenderer + drawRect() con alpha
 *
 * La proyección ortográfica mapea (0..width, 0..height) a NDC, con Y hacia
 * arriba. El rango de Z es amplio para que las mallas 3D tengan profundidad;
 * los elementos 2D se dibujan en z = 0 con el depth test desactivado.
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
     * @brief Libera los recursos de GPU (VAOs, VBOs, texturas y shader programs).
     *
     * Es seguro llamarlo aunque init() haya fallado parcialmente. Las texturas
     * cargadas con loadTexture() deben liberarse antes con destroyTexture().
     */
    void cleanup();

    /**
     * @brief Carga un PNG/JPG desde disco como textura de OpenGL.
     * @param path Ruta del archivo de imagen.
     * @return Textura cargada, o `{}` si no se pudo leer.
     *
     * La imagen se voltea verticalmente al cargarla para que la fila 0 del
     * archivo quede en la parte inferior del espacio del juego (Y hacia arriba).
     */
    Texture loadTexture(const std::string& path);

    /**
     * @brief Libera una textura creada por loadTexture().
     * @param texture Textura a destruir; queda en estado inválido.
     */
    void destroyTexture(Texture& texture);

    /**
     * @brief Dibuja una textura como un quad centrado.
     * @param texture Textura a dibujar.
     * @param center  Centro del quad en píxeles.
     * @param size    Tamaño completo (ancho, alto) en píxeles.
     * @param alpha   Opacidad [0, 1]; útil para crossfades.
     */
    void drawTexture(const Texture& texture, const glm::vec2& center,
                     const glm::vec2& size, float alpha = 1.0f);

    /**
     * @brief Dibuja un círculo relleno.
     * @param center Centro en píxeles de pantalla.
     * @param radius Radio en píxeles.
     * @param color  Color RGB en el rango [0, 1].
     * @param alpha  Opacidad [0, 1].
     */
    void drawCircle(const glm::vec2& center, float radius, const glm::vec3& color,
                    float alpha = 1.0f);

    /**
     * @brief Dibuja un rectángulo relleno, opcionalmente rotado.
     * @param center        Centro del rectángulo en píxeles.
     * @param size          Tamaño completo (ancho, alto) en píxeles.
     * @param angleRadians  Rotación en radianes (positiva = antihorario).
     * @param color         Color RGB en el rango [0, 1].
     * @param alpha         Opacidad [0, 1]; útil para overlays.
     */
    void drawRect(const glm::vec2& center, const glm::vec2& size,
                  float angleRadians, const glm::vec3& color, float alpha = 1.0f);

    /**
     * @brief Dibuja una estela como cinta (triangle strip) con grosor y alpha.
     * @param points Puntos ordenados del más antiguo al más reciente.
     * @param color  Color RGBA base en el rango [0, 1].
     *
     * Cada punto aporta su propio grosor y alpha; el color se interpola con
     * blending sobre lo que ya está dibujado. Se necesitan al menos 2 puntos.
     */
    void drawTrail(const std::vector<TrailPoint>& points, const glm::vec4& color);

    /**
     * @brief Dibuja un polígono relleno definido en coordenadas locales.
     * @param vertices Vértices del contorno en orden (el último se une al primero).
     * @param center   Centro del polígono en píxeles de pantalla.
     * @param angleRadians Rotación en radianes.
     * @param scale    Escala uniforme (normalmente el radio de la fruta).
     * @param color    Color RGB en [0, 1].
     * @param alpha    Opacidad [0, 1].
     *
     * El polígono debe ser convexo o en forma de estrella respecto a su centro;
     * se triangula con un abanico desde el centroide. Se usa para las mitades
     * 2D de las frutas, que tienen siluetas distintas (plátano, limón, etc.).
     */
    void drawPolygon(const std::vector<glm::vec2>& vertices, const glm::vec2& center,
                     float angleRadians, float scale, const glm::vec3& color,
                     float alpha = 1.0f);

    /**
     * @brief Activa el depth test para dibujar mallas 3D.
     *
     * Debe llamarse antes de las mallas y cerrarse con `end3D()` antes de
     * volver a dibujar primitivas 2D, que no usan el buffer de profundidad.
     */
    void begin3D();

    /// @brief Desactiva el depth test y vuelve al modo 2D.
    void end3D();

    /**
     * @brief Dibuja una malla low-poly de fruta con luz direccional.
     * @param id    Tipo de malla registrada en el Renderer.
     * @param model Matriz modelo (traslación, rotación y escala).
     *
     * Solo debe llamarse entre `begin3D()` y `end3D()`. La normal se
     * transforma con la inversa traspuesta del modelo y se normaliza en el
     * shader; el color viene por vértice.
     */
    void drawMesh(FruitMeshId id, const glm::mat4& model);

private:
    /**
     * @brief Compila el shader de mallas 3D (luz direccional + color).
     * @return true si el shader program quedó listo.
     */
    bool buildMeshProgram();

    /**
     * @brief Sube una malla de CPU a VAO/VBO/EBO.
     * @param data Datos de vértices e índices.
     * @param mesh Malla de GPU de salida.
     * @return true si los buffers se crearon correctamente.
     */
    bool uploadMeshData(const MeshData& data, Mesh& mesh);
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
     * @brief Compila el shader de texturas (fondo con alpha uniforme).
     * @return true si el shader program quedó listo.
     */
    bool buildTextureProgram();

    /**
     * @brief Configura la matriz ortográfica y la envía a todos los shaders.
     * @param screenWidth  Ancho lógico de la ventana.
     * @param screenHeight Alto lógico de la ventana.
     */
    void uploadProjection(int screenWidth, int screenHeight);

    /**
     * @brief Crea las mallas: círculo, quad, textura, estela y polígono.
     * @return true si los buffers se generaron correctamente.
     */
    bool setupGeometry();

    /**
     * @brief Envía la transformación y el color, y dibuja la malla indicada.
     * @param vao         VAO a dibujar.
     * @param vertexCount Número de vértices.
     * @param mode        GL_TRIANGLE_FAN o GL_TRIANGLE_STRIP.
     * @param model       Matriz modelo (escala, rotación, traslación).
     * @param color       Color RGBA.
     */
    void drawPrimitive(unsigned int vao, unsigned int vertexCount, unsigned int mode,
                       const glm::mat4& model, const glm::vec4& color);

private:
    unsigned int m_circleVAO = 0;  ///< VAO del círculo unitario (radio 1).
    unsigned int m_circleVBO = 0;  ///< VBO del círculo unitario.
    unsigned int m_rectVAO = 0;    ///< VAO del quad unitario (-1..1).
    unsigned int m_rectVBO = 0;    ///< VBO del quad unitario.
    unsigned int m_textureVAO = 0; ///< VAO del quad texturizado (-1..1).
    unsigned int m_textureVBO = 0; ///< VBO del quad texturizado.
    unsigned int m_circleVertexCount = 0;  ///< Vértices del abanico del círculo.

    unsigned int m_trailVAO = 0;   ///< VAO de la estela dinámica.
    unsigned int m_trailVBO = 0;   ///< VBO de la estela (se reescribe cada frame).

    unsigned int m_polygonVAO = 0; ///< VAO del polígono dinámico 2D.
    unsigned int m_polygonVBO = 0; ///< VBO del polígono dinámico 2D.

    unsigned int m_shaderProgram = 0;
    int m_projectionLocation = -1;
    int m_modelLocation = -1;
    int m_colorLocation = -1;

    unsigned int m_trailProgram = 0;
    int m_trailProjectionLocation = -1;
    int m_trailColorLocation = -1;

    unsigned int m_textureProgram = 0;
    int m_textureProjectionLocation = -1;
    int m_textureModelLocation = -1;
    int m_textureAlphaLocation = -1;
    int m_textureSamplerLocation = -1;

    unsigned int m_meshProgram = 0;
    int m_meshProjectionLocation = -1;
    int m_meshModelLocation = -1;
    int m_meshNormalLocation = -1;
    std::array<Mesh, kFruitMeshCount> m_meshes{};  ///< Mallas low-poly de frutas.
};
