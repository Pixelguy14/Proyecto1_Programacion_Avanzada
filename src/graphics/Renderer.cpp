#include "Renderer.h"
#include <glad/gl.h>
#include <iostream>

/**
 * @file Renderer.cpp
 * @brief Implementación mínima del Renderer para el Sprint 0.
 *
 * Dibuja un rectángulo rojo centrado usando coordenadas normalizadas de
 * OpenGL (NDC). En el Sprint 1 se añadirá la proyección ortográfica para
 * trabajar directamente en coordenadas de pantalla (0–1920, 0–1080).
 */

// ----- Código fuente de los shaders inline -----

static const char* vertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const char* fragmentSrc = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // rojo
}
)";

// ----- Función auxiliar para compilar un shader -----

/**
 * @brief Compila un shader GLSL y verifica errores.
 * @param source Código fuente GLSL.
 * @param type   GL_VERTEX_SHADER o GL_FRAGMENT_SHADER.
 * @return ID del shader compilado (0 en caso de error).
 */
static unsigned int compileShader(const char* source, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "[Shader error] " << log << std::endl;
        return 0;
    }
    return shader;
}

// ----- Métodos de Renderer -----

void Renderer::init() {
    // Compilar y enlazar shaders
    unsigned int vs = compileShader(vertexSrc,   GL_VERTEX_SHADER);
    unsigned int fs = compileShader(fragmentSrc, GL_FRAGMENT_SHADER);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vs);
    glAttachShader(m_shaderProgram, fs);
    glLinkProgram(m_shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Vértices del rectángulo (NDC: -0.5 a 0.5)
    float vertices[] = {
        -0.5f,  0.5f,   // superior izquierdo
        -0.5f, -0.5f,   // inferior izquierdo
         0.5f, -0.5f,   // inferior derecho
         0.5f,  0.5f    // superior derecho
    };
    unsigned int indices[] = { 0, 1, 2,  0, 2, 3 };

    unsigned int EBO;
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Renderer::render() {
    glUseProgram(m_shaderProgram);
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::cleanup() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteProgram(m_shaderProgram);
}
