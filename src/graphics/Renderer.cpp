#include "Renderer.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>
#include <vector>

/**
 * @file Renderer.cpp
 * @brief Implementación de las primitivas de dibujado 2D.
 *
 * Todas las figuras se dibujan a partir de dos mallas unitarias (un círculo de
 * radio 1 y un quad de -1 a 1) transformadas con una matriz modelo. El vertex
 * shader aplica `uProjection * uModel`; el fragment shader usa `uColor`.
 *
 * Referencia: The Nature of Code – Cap. 1 (sistemas de coordenadas).
 */

namespace {

/// Segmentos del círculo unitario: más segmentos = borde más suave.
constexpr int kCircleSegments = 64;

/// Código GLSL del vertex shader: proyección ortográfica + transformación modelo.
const char* kVertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;

uniform mat4 uProjection;  // pantalla (0..width, 0..height) -> NDC
uniform mat4 uModel;       // escala/rotación/traslación de la figura

void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
)";

/// Código GLSL del fragment shader: color plano por uniform.
const char* kFragmentSrc = R"(
#version 330 core
uniform vec3 uColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

/**
 * @brief Compila un shader GLSL y verifica errores.
 * @param source Código fuente GLSL (terminado en '\0').
 * @param type   GL_VERTEX_SHADER o GL_FRAGMENT_SHADER.
 * @return ID del shader compilado, o 0 si hubo un error de compilación.
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
        std::cerr << "[Renderer] Shader compile error: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

/**
 * @brief Enlaza un vertex y un fragment shader ya compilados.
 * @param vertexShader   ID del vertex shader.
 * @param fragmentShader ID del fragment shader.
 * @return ID del shader program, o 0 si el enlazado falló.
 */
unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    const unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "[Renderer] Program link error: " << log << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

/**
 * @brief Sube una malla 2D (posiciones vec2) a un VAO/VBO nuevos.
 * @param vertices Vértices intercalados (x, y, x, y, ...).
 * @param vao      VAO de salida.
 * @param vbo      VBO de salida.
 */
void uploadMesh(const std::vector<float>& vertices, unsigned int& vao, unsigned int& vbo) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

/**
 * @brief Genera los vértices de un círculo unitario como abanico de triángulos.
 *
 * El primer vértice es el centro (0,0) y los siguientes recorren la
 * circunferencia; se repite el primero al final para cerrar el abanico.
 * @return Vector intercalado (x, y) listo para subir a la GPU.
 */
std::vector<float> makeUnitCircleVertices() {
    std::vector<float> vertices;
    vertices.reserve(static_cast<std::size_t>(kCircleSegments + 2) * 2);

    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    for (int i = 0; i <= kCircleSegments; ++i) {
        const float angle = 2.0f * 3.14159265358979323846f * static_cast<float>(i)
                            / static_cast<float>(kCircleSegments);
        vertices.push_back(std::cos(angle));
        vertices.push_back(std::sin(angle));
    }
    return vertices;
}

}  // namespace

bool Renderer::init(int screenWidth, int screenHeight) {
    if (!buildShaderProgram()) {
        return false;
    }
    uploadProjection(screenWidth, screenHeight);
    return setupGeometry();
}

bool Renderer::buildShaderProgram() {
    const unsigned int vertexShader = compileShader(kVertexSrc, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        return false;
    }

    const unsigned int fragmentShader = compileShader(kFragmentSrc, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    m_shaderProgram = linkProgram(vertexShader, fragmentShader);

    // Los shaders ya están enlazados: se pueden liberar en cualquier caso.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (m_shaderProgram == 0) {
        return false;
    }

    m_projectionLocation = glGetUniformLocation(m_shaderProgram, "uProjection");
    m_modelLocation = glGetUniformLocation(m_shaderProgram, "uModel");
    m_colorLocation = glGetUniformLocation(m_shaderProgram, "uColor");
    return true;
}

void Renderer::uploadProjection(int screenWidth, int screenHeight) {
    // Mapea el espacio de pantalla (0,width) x (0,height) a NDC, con Y hacia arriba.
    const glm::mat4 projection = glm::ortho(
        0.0f, static_cast<float>(screenWidth),
        0.0f, static_cast<float>(screenHeight),
        -1.0f, 1.0f);

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
}

bool Renderer::setupGeometry() {
    const std::vector<float> circleVertices = makeUnitCircleVertices();
    uploadMesh(circleVertices, m_circleVAO, m_circleVBO);
    m_circleVertexCount = static_cast<unsigned int>(circleVertices.size() / 2);

    // Quad unitario (-1..1) en orden de triangle strip: BL, BR, TL, TR.
    const std::vector<float> quadVertices = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };
    uploadMesh(quadVertices, m_rectVAO, m_rectVBO);

    return m_circleVAO != 0 && m_circleVBO != 0 && m_rectVAO != 0 && m_rectVBO != 0;
}

void Renderer::drawCircle(const glm::vec2& center, float radius, const glm::vec3& color) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(radius, radius, 1.0f));
    drawPrimitive(m_circleVAO, m_circleVertexCount, GL_TRIANGLE_FAN, model, color);
}

void Renderer::drawRect(const glm::vec2& center, const glm::vec2& size,
                        float angleRadians, const glm::vec3& color) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::rotate(model, angleRadians, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(size.x * 0.5f, size.y * 0.5f, 1.0f));
    drawPrimitive(m_rectVAO, 4, GL_TRIANGLE_STRIP, model, color);
}

void Renderer::drawPrimitive(unsigned int vao, unsigned int vertexCount, unsigned int mode,
                             const glm::mat4& model, const glm::vec3& color) {
    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_modelLocation, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(m_colorLocation, 1, glm::value_ptr(color));

    glBindVertexArray(vao);
    glDrawArrays(mode, 0, static_cast<int>(vertexCount));
    glBindVertexArray(0);
}

void Renderer::cleanup() {
    if (m_circleVBO != 0) {
        glDeleteBuffers(1, &m_circleVBO);
        m_circleVBO = 0;
    }
    if (m_circleVAO != 0) {
        glDeleteVertexArrays(1, &m_circleVAO);
        m_circleVAO = 0;
    }
    if (m_rectVBO != 0) {
        glDeleteBuffers(1, &m_rectVBO);
        m_rectVBO = 0;
    }
    if (m_rectVAO != 0) {
        glDeleteVertexArrays(1, &m_rectVAO);
        m_rectVAO = 0;
    }
    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
}
