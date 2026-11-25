#pragma once

#include <glm/glm.hpp>

#include <array>
#include <string>

// FreeType se declara por adelantado para no filtrar sus cabeceras a todo el
// proyecto; la implementación incluye ft2build.h en el .cpp.
struct FT_FaceRec_;
struct FT_LibraryRec_;

/**
 * @file TextRenderer.h
 * @brief Texto 2D con FreeType y un atlas de glifos (Sprint 4).
 *
 * ## Jerarquía
 *   GameUi
 *     └── TextRenderer
 *           ├── FreeType  → rasteriza los glifos ASCII a un atlas
 *           └── OpenGL    → shader propio y un quad por glifo
 *
 * El atlas se genera una sola vez en `init()`: se rasterizan los caracteres
 * imprimibles ASCII (32–126) con el tamaño de píxel pedido y se empaquetan en
 * una cuadrícula de celdas fijas. Cada glifo guarda sus coordenadas UV y sus
 * métricas (avance, rodamiento y tamaño), de modo que dibujar texto solo
 * consiste en emitir quads con la UV correspondiente.
 *
 * El texto se dibuja en píxeles de pantalla, con el mismo sistema de
 * coordenadas que el resto del juego (origen abajo-izquierda, Y hacia arriba).
 */
class TextRenderer {
public:
    TextRenderer() = default;

    /// El atlas y el shader se liberan con `cleanup()`, no en el destructor:
    /// el contexto OpenGL puede haberse destruido antes que este objeto.
    ~TextRenderer() = default;

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    /**
     * @brief Carga la fuente, rasteriza el atlas y crea los recursos OpenGL.
     * @param fontPath     Ruta al archivo TTF/OTF.
     * @param screenWidth  Ancho lógico de la pantalla para la proyección.
     * @param screenHeight Alto lógico de la pantalla para la proyección.
     * @param pixelSize    Altura nominal de la fuente en píxeles.
     * @return true si la fuente y el atlas quedaron listos.
     */
    bool init(const std::string& fontPath, int screenWidth, int screenHeight,
              int pixelSize = 64);

    /**
     * @brief Libera la textura, los buffers, el shader y la cara de FreeType.
     *
     * Es idempotente y seguro aunque `init()` haya fallado a medias.
     */
    void cleanup();

    /// @return true si el atlas está cargado y se puede dibujar texto.
    bool ready() const { return m_atlasTexture != 0; }

    /**
     * @brief Altura de una línea de texto.
     * @param scale Factor de escala respecto al tamaño rasterizado.
     * @return Altura en píxeles, incluidos ascendentes y descendentes.
     */
    float lineHeight(float scale = 1.0f) const { return m_lineHeight * scale; }

    /**
     * @brief Distancia desde la parte superior de la línea hasta la línea base.
     * @param scale Factor de escala respecto al tamaño rasterizado.
     * @return Ascendente en píxeles.
     */
    float ascent(float scale = 1.0f) const { return m_ascent * scale; }

    /**
     * @brief Mide el ancho de la línea más larga de un texto.
     * @param text  Texto a medir; puede contener saltos de línea `\n`.
     * @param scale Factor de escala.
     * @return Ancho en píxeles.
     */
    float textWidth(const std::string& text, float scale = 1.0f) const;

    /**
     * @brief Dibuja texto alineado a la izquierda.
     * @param text    Texto a dibujar; admite saltos de línea `\n`.
     * @param topLeft Esquina superior izquierda de la primera línea, en píxeles.
     * @param scale   Factor de escala respecto al tamaño rasterizado.
     * @param color   Color RGBA en [0, 1]; el alpha multiplica la cobertura.
     *
     * Las líneas siguientes bajan `lineHeight(scale)` píxeles. Los caracteres
     * fuera del rango ASCII imprimible se omiten.
     */
    void drawText(const std::string& text, const glm::vec2& topLeft, float scale,
                  const glm::vec4& color) const;

private:
    /**
     * @brief Métricas y UV de un glifo dentro del atlas.
     */
    struct Glyph {
        float u0 = 0.0f;         ///< UV izquierda.
        float v0 = 0.0f;         ///< UV superior.
        float u1 = 0.0f;         ///< UV derecha.
        float v1 = 0.0f;         ///< UV inferior.
        float width = 0.0f;      ///< Ancho del bitmap en píxeles.
        float height = 0.0f;     ///< Alto del bitmap en píxeles.
        float bearingX = 0.0f;   ///< Desplazamiento horizontal desde el origen.
        float bearingY = 0.0f;   ///< Desplazamiento vertical desde la línea base.
        float advance = 0.0f;    ///< Avance horizontal del cursor.
    };

    /**
     * @brief Crea el shader de texto, su VAO/VBO dinámico y la proyección.
     * @param screenWidth  Ancho lógico de la pantalla.
     * @param screenHeight Alto lógico de la pantalla.
     * @return true si el programa y los buffers se crearon correctamente.
     */
    bool buildResources(int screenWidth, int screenHeight);

    /**
     * @brief Rasteriza los glifos ASCII y sube el atlas a una textura GL_RED.
     * @param pixelSize Altura nominal de la fuente.
     * @return true si el atlas se generó completo.
     */
    bool buildAtlas(int pixelSize);

private:
    std::array<Glyph, 128> m_glyphs{};  ///< Glifos indexados por código ASCII.
    unsigned int m_atlasTexture = 0;    ///< Textura GL_RED del atlas.
    int m_atlasWidth = 0;               ///< Ancho del atlas en píxeles.
    int m_atlasHeight = 0;              ///< Alto del atlas en píxeles.
    float m_lineHeight = 0.0f;          ///< Altura de línea a tamaño nativo.
    float m_ascent = 0.0f;              ///< Ascendente a tamaño nativo.

    unsigned int m_program = 0;         ///< Shader program de texto.
    int m_projectionLocation = -1;      ///< Uniform de la proyección.
    int m_colorLocation = -1;           ///< Uniform del color RGBA.
    int m_samplerLocation = -1;         ///< Uniform de la textura del atlas.
    unsigned int m_vao = 0;             ///< VAO del buffer dinámico de quads.
    unsigned int m_vbo = 0;             ///< VBO reescrito en cada drawText().

    FT_LibraryRec_* m_library = nullptr;  ///< Biblioteca FreeType.
    FT_FaceRec_* m_face = nullptr;        ///< Cara de la fuente cargada.
};
