#include "Renderer.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

/**
 * @file Renderer.cpp
 * @brief Implementación del Renderer para el Sprint 0.
 *
 * Dibuja un rectángulo rojo usando coordenadas de pantalla (0–1920, 0–1080)
 * gracias a la proyección ortográfica definida por glm::ortho().
 *
 * ## ¿Qué es la proyección ortográfica?
 * En OpenGL, el espacio NDC (Normalized Device Coordinates) va de -1 a 1 en
 * cada eje. Con una proyección ortográfica mapeamos un rango arbitrario de
 * coordenadas (0 a 1920 en X, 0 a 1080 en Y) a ese espacio, de modo que
 * podemos usar coordenadas de píxeles directamente en nuestro código de juego.
 *
 * La matriz resultante se pasa al vertex shader como el uniform "uProjection".
 *
 * Referencia: The Nature of Code – Cap. 1 (Vectores y sistemas de coordenadas).
 */

namespace {

/// Resolución lógica usada por la proyección ortográfica.
constexpr float kScreenWidth = 1920.0f;
constexpr float kScreenHeight = 1080.0f;

/// Código GLSL del vertex shader: aplica la proyección a cada vértice.
const char* kVertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;

// Matriz de proyección ortográfica: convierte coordenadas de pantalla a NDC.
uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

/// Código GLSL del fragment shader: color rojo plano.
const char* kFragmentSrc = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // rojo
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

}  // namespace

bool Renderer::init() {
    if (!buildShaderProgram()) {
        return false;
    }
    uploadProjection();
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
    return m_shaderProgram != 0;
}

void Renderer::uploadProjection() {
    // Mapea el espacio de pantalla (0,1920) x (0,1080) a NDC.
    const glm::mat4 projection = glm::ortho(
        0.0f, kScreenWidth,   // left, right
        0.0f, kScreenHeight,  // bottom, top
        -1.0f, 1.0f           // near, far
    );

    glUseProgram(m_shaderProgram);
    const int location = glGetUniformLocation(m_shaderProgram, "uProjection");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));
}

bool Renderer::setupGeometry() {
    // Rectángulo centrado en pantalla, en píxeles: (760,440)–(1160,640).
    const float vertices[] = {
         760.0f,  640.0f,  // superior izquierdo
         760.0f,  440.0f,  // inferior izquierdo
        1160.0f,  440.0f,  // inferior derecho
        1160.0f,  640.0f   // superior derecho
    };
    const unsigned int indices[] = {0, 1, 2, 0, 2, 3};

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Atributo 0: posición (vec2) con stride de 2 floats.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return m_VAO != 0 && m_VBO != 0 && m_EBO != 0;
}

void Renderer::render() {
    glUseProgram(m_shaderProgram);
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Renderer::cleanup() {
    if (m_EBO != 0) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
}
