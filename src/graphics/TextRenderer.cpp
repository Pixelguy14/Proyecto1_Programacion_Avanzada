#include "TextRenderer.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

/**
 * @file TextRenderer.cpp
 * @brief Atlas FreeType y dibujado de texto con quads texturizados.
 *
 * El atlas es una cuadrícula de celdas fijas: 95 caracteres imprimibles ASCII
 * en 10 columnas. Cada celda mide 1.5 veces el tamaño de píxel de la fuente,
 * espacio suficiente para ascendentes, descendentes y acentos del latin básico.
 */

namespace {

/// Primer y último carácter incluidos en el atlas.
constexpr int kFirstChar = 32;
constexpr int kLastChar = 126;

/// Distribución del atlas: 10 columnas × 10 filas para 95 glifos.
constexpr int kAtlasColumns = 10;
constexpr int kAtlasRows = 10;

/// Margen interior de cada celda, en píxeles.
constexpr int kCellPadding = 4;

/// Floats por vértice del texto: x, y, u, v.
constexpr int kTextFloatsPerVertex = 4;

/// Vértices por glifo (dos triángulos).
constexpr int kVerticesPerGlyph = 6;

/// Vertex shader del texto: posiciones ya expresadas en píxeles de pantalla.
const char* kVertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uProjection;

out vec2 vTexCoord;

void main() {
    vTexCoord = aTexCoord;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

/// Fragment shader del texto: la cobertura del atlas modula el alpha del color.
const char* kFragmentSrc = R"(
#version 330 core
in vec2 vTexCoord;

uniform sampler2D uAtlas;
uniform vec4 uColor;

out vec4 FragColor;

void main() {
    float coverage = texture(uAtlas, vTexCoord).r;
    FragColor = vec4(uColor.rgb, uColor.a * coverage);
}
)";

/**
 * @brief Compila un shader GLSL y muestra el log si falla.
 * @param source Código fuente GLSL.
 * @param type   GL_VERTEX_SHADER o GL_FRAGMENT_SHADER.
 * @return ID del shader, o 0 si hubo un error.
 */
unsigned int compileShader(const char* source, unsigned int type) {
    const unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "[TextRenderer] Shader compile error: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

/**
 * @brief Compila y enlaza el par de shaders del texto.
 * @return ID del programa, o 0 si algo falló.
 */
unsigned int createTextProgram() {
    const unsigned int vertexShader = compileShader(kVertexSrc, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        return 0;
    }

    const unsigned int fragmentShader = compileShader(kFragmentSrc, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }

    const unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "[TextRenderer] Program link error: " << log << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

/**
 * @brief Añade los seis vértices de un quad texturizado.
 * @param vertices Buffer de salida (x, y, u, v intercalados).
 * @param x0,y0    Esquina inferior izquierda en píxeles.
 * @param x1,y1    Esquina superior derecha en píxeles.
 * @param u0,v0    UV de la esquina superior izquierda del glifo.
 * @param u1,v1    UV de la esquina inferior derecha del glifo.
 */
void appendGlyphQuad(std::vector<float>& vertices, float x0, float y0, float x1, float y1,
                     float u0, float v0, float u1, float v1) {
    // Triángulo 1: inferior izquierda, inferior derecha, superior derecha.
    vertices.insert(vertices.end(), {
        x0, y0, u0, v1,
        x1, y0, u1, v1,
        x1, y1, u1, v0,
        // Triángulo 2: inferior izquierda, superior derecha, superior izquierda.
        x0, y0, u0, v1,
        x1, y1, u1, v0,
        x0, y1, u0, v0,
    });
}

}  // namespace

bool TextRenderer::init(const std::string& fontPath, int screenWidth, int screenHeight,
                        int pixelSize) {
    if (FT_Init_FreeType(&m_library) != 0) {
        std::cerr << "[TextRenderer] No se pudo inicializar FreeType." << std::endl;
        return false;
    }

    if (FT_New_Face(m_library, fontPath.c_str(), 0, &m_face) != 0) {
        std::cerr << "[TextRenderer] No se pudo cargar la fuente: " << fontPath << std::endl;
        cleanup();
        return false;
    }

    if (!buildResources(screenWidth, screenHeight)) {
        cleanup();
        return false;
    }

    if (!buildAtlas(pixelSize)) {
        cleanup();
        return false;
    }

    return true;
}

bool TextRenderer::buildResources(int screenWidth, int screenHeight) {
    m_program = createTextProgram();
    if (m_program == 0) {
        return false;
    }

    m_projectionLocation = glGetUniformLocation(m_program, "uProjection");
    m_colorLocation = glGetUniformLocation(m_program, "uColor");
    m_samplerLocation = glGetUniformLocation(m_program, "uAtlas");

    glUseProgram(m_program);
    const glm::mat4 projection = glm::ortho(
        0.0f, static_cast<float>(screenWidth),
        0.0f, static_cast<float>(screenHeight),
        -1.0f, 1.0f);
    glUniformMatrix4fv(m_projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(m_samplerLocation, 0);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    const auto stride = static_cast<GLsizei>(kTextFloatsPerVertex * sizeof(float));
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return m_vao != 0 && m_vbo != 0;
}

bool TextRenderer::buildAtlas(int pixelSize) {
    if (FT_Set_Pixel_Sizes(m_face, 0, static_cast<FT_UInt>(pixelSize)) != 0) {
        std::cerr << "[TextRenderer] Tamano de fuente invalido: " << pixelSize << std::endl;
        return false;
    }

    const int cellSize = (pixelSize * 3) / 2;
    m_atlasWidth = kAtlasColumns * cellSize;
    m_atlasHeight = kAtlasRows * cellSize;

    std::vector<unsigned char> atlas(
        static_cast<std::size_t>(m_atlasWidth) * static_cast<std::size_t>(m_atlasHeight), 0);

    for (int code = kFirstChar; code <= kLastChar; ++code) {
        if (FT_Load_Char(m_face, static_cast<FT_ULong>(code), FT_LOAD_RENDER) != 0) {
            std::cerr << "[TextRenderer] No se pudo rasterizar el caracter " << code << std::endl;
            return false;
        }

        const FT_GlyphSlot slot = m_face->glyph;
        const FT_Bitmap& bitmap = slot->bitmap;
        if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
            std::cerr << "[TextRenderer] Formato de bitmap no soportado." << std::endl;
            return false;
        }

        const int width = static_cast<int>(bitmap.width);
        const int height = static_cast<int>(bitmap.rows);
        if (width + 2 * kCellPadding > cellSize || height + 2 * kCellPadding > cellSize) {
            std::cerr << "[TextRenderer] Glifo demasiado grande para la celda del atlas."
                      << std::endl;
            return false;
        }

        const int index = code - kFirstChar;
        const int cellX = (index % kAtlasColumns) * cellSize + kCellPadding;
        const int cellY = (index / kAtlasColumns) * cellSize + kCellPadding;

        // Copia fila a fila: bitmap.pitch puede incluir padding al final de cada fila.
        for (int row = 0; row < height; ++row) {
            const unsigned char* sourceRow = bitmap.buffer + row * bitmap.pitch;
            const std::size_t destination =
                static_cast<std::size_t>(cellY + row) * static_cast<std::size_t>(m_atlasWidth)
                + static_cast<std::size_t>(cellX);
            std::copy(sourceRow, sourceRow + width, atlas.begin() + destination);
        }

        Glyph& glyph = m_glyphs[static_cast<std::size_t>(code)];
        glyph.width = static_cast<float>(width);
        glyph.height = static_cast<float>(height);
        glyph.bearingX = static_cast<float>(slot->bitmap_left);
        glyph.bearingY = static_cast<float>(slot->bitmap_top);
        glyph.advance = static_cast<float>(slot->advance.x) / 64.0f;
        glyph.u0 = static_cast<float>(cellX) / static_cast<float>(m_atlasWidth);
        glyph.v0 = static_cast<float>(cellY) / static_cast<float>(m_atlasHeight);
        glyph.u1 = static_cast<float>(cellX + width) / static_cast<float>(m_atlasWidth);
        glyph.v1 = static_cast<float>(cellY + height) / static_cast<float>(m_atlasHeight);
    }

    m_lineHeight = static_cast<float>(m_face->size->metrics.height) / 64.0f;
    m_ascent = std::fabs(static_cast<float>(m_face->size->metrics.ascender) / 64.0f);

    glGenTextures(1, &m_atlasTexture);
    glBindTexture(GL_TEXTURE_2D, m_atlasTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_atlasWidth, m_atlasHeight, 0, GL_RED,
                 GL_UNSIGNED_BYTE, atlas.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return m_atlasTexture != 0;
}

void TextRenderer::cleanup() {
    if (m_atlasTexture != 0) {
        glDeleteTextures(1, &m_atlasTexture);
        m_atlasTexture = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    if (m_face != nullptr) {
        FT_Done_Face(m_face);
        m_face = nullptr;
    }
    if (m_library != nullptr) {
        FT_Done_FreeType(m_library);
        m_library = nullptr;
    }
}

float TextRenderer::textWidth(const std::string& text, float scale) const {
    float widestLine = 0.0f;
    float lineWidth = 0.0f;

    for (char character : text) {
        if (character == '\n') {
            widestLine = std::max(widestLine, lineWidth);
            lineWidth = 0.0f;
            continue;
        }

        const auto code = static_cast<unsigned char>(character);
        if (code < kFirstChar || code > kLastChar) {
            continue;
        }
        lineWidth += m_glyphs[code].advance * scale;
    }

    return std::max(widestLine, lineWidth);
}

void TextRenderer::drawText(const std::string& text, const glm::vec2& topLeft, float scale,
                            const glm::vec4& color) const {
    if (!ready() || scale <= 0.0f || text.empty()) {
        return;
    }

    std::vector<float> vertices;
    vertices.reserve(text.size() * kVerticesPerGlyph * kTextFloatsPerVertex);

    float penX = topLeft.x;
    float baseline = topLeft.y - m_ascent * scale;

    for (char character : text) {
        if (character == '\n') {
            penX = topLeft.x;
            baseline -= m_lineHeight * scale;
            continue;
        }

        const auto code = static_cast<unsigned char>(character);
        if (code < kFirstChar || code > kLastChar) {
            continue;
        }

        const Glyph& glyph = m_glyphs[code];
        if (glyph.width > 0.0f && glyph.height > 0.0f) {
            const float x0 = penX + glyph.bearingX * scale;
            const float y1 = baseline + glyph.bearingY * scale;
            const float x1 = x0 + glyph.width * scale;
            const float y0 = y1 - glyph.height * scale;
            appendGlyphQuad(vertices, x0, y0, x1, y1,
                            glyph.u0, glyph.v0, glyph.u1, glyph.v1);
        }
        penX += glyph.advance * scale;
    }

    if (vertices.empty()) {
        return;
    }

    glUseProgram(m_program);
    glUniform4fv(m_colorLocation, 1, glm::value_ptr(color));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_atlasTexture);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size() / kTextFloatsPerVertex));
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}
