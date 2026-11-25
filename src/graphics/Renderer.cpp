#include "Renderer.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stb_image es header-only; la implementación se compila una sola vez aquí.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cmath>
#include <iostream>
#include <vector>

/**
 * @file Renderer.cpp
 * @brief Implementación de las primitivas de dibujado 2D y de la estela.
 *
 * Las figuras sólidas se dibujan a partir de dos mallas unitarias (círculo y
 * quad) transformadas con una matriz modelo. La estela, en cambio, genera una
 * cinta dinámica cada frame porque su geometría depende del recorrido del mouse.
 *
 * Blending (`GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`) permite que la estela se
 * desvanezca sin afectar a las figuras opacas.
 *
 * Referencia: The Nature of Code – Cap. 1 (sistemas de coordenadas).
 */

namespace {

/// Segmentos del círculo unitario: más segmentos = borde más suave.
constexpr int kCircleSegments = 64;

/// Segmentos del semicírculo unitario (mitad de fruta).
constexpr int kSemicircleSegments = 40;

/// π, usado para generar la malla del semicírculo.
constexpr float kPi = 3.14159265358979323846f;

/// Distancia mínima para considerar válida una dirección en la estela.
constexpr float kMinDirectionLength = 1e-4f;

/// Floats por vértice en la estela: x, y, alpha.
constexpr int kTrailFloatsPerVertex = 3;

/// Código GLSL del vertex shader de figuras sólidas.
const char* kVertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;

uniform mat4 uProjection;  // pantalla (0..width, 0..height) -> NDC
uniform mat4 uModel;       // escala/rotación/traslación de la figura

void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
)";

/// Código GLSL del fragment shader de figuras sólidas.
const char* kFragmentSrc = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;

void main() {
    FragColor = uColor;
}
)";

/// Vertex shader de texturas: quad transformado por modelo y proyección.
const char* kTextureVertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uModel;

out vec2 vTexCoord;

void main() {
    vTexCoord = aTexCoord;
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
)";

/// Fragment shader de texturas: conserva el color y aplica alpha uniforme.
const char* kTextureFragmentSrc = R"(
#version 330 core
in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform float uAlpha;
out vec4 FragColor;

void main() {
    vec4 texel = texture(uTexture, vTexCoord);
    FragColor = vec4(texel.rgb, texel.a * uAlpha);
}
)";

/// Vertex shader de la estela: la posición ya viene en píxeles de pantalla.
const char* kTrailVertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in float aAlpha;

uniform mat4 uProjection;

out float vAlpha;

void main() {
    vAlpha = aAlpha;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

/// Fragment shader de la estela: multiplica el alpha base por el del vértice.
const char* kTrailFragmentSrc = R"(
#version 330 core
uniform vec4 uColor;

in float vAlpha;
out vec4 FragColor;

void main() {
    FragColor = vec4(uColor.rgb, uColor.a * vAlpha);
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
 * @brief Compila y enlaza un par de shaders, liberando los shaders intermedios.
 * @param vertexSrc   Código GLSL del vertex shader.
 * @param fragmentSrc Código GLSL del fragment shader.
 * @return ID del shader program, o 0 si algo falló.
 */
unsigned int createProgram(const char* vertexSrc, const char* fragmentSrc) {
    const unsigned int vertexShader = compileShader(vertexSrc, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        return 0;
    }

    const unsigned int fragmentShader = compileShader(fragmentSrc, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }

    const unsigned int program = linkProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

/**
 * @brief Sube una malla 2D estática (posiciones vec2) a un VAO/VBO nuevos.
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
 * @brief Crea el quad unitario con posición y coordenadas de textura.
 * @param vao VAO de salida.
 * @param vbo VBO de salida.
 *
 * Vértices en orden triangle strip: BL, BR, TL, TR. Las UV van de (0,0) en la
 * esquina inferior izquierda a (1,1) en la superior derecha.
 */
void uploadTextureQuad(unsigned int& vao, unsigned int& vbo) {
    const std::vector<float> vertices = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);

    constexpr int kFloatsPerVertex = 4;
    const auto stride = static_cast<GLsizei>(kFloatsPerVertex * sizeof(float));
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
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
        const float angle = 2.0f * kPi * static_cast<float>(i)
                            / static_cast<float>(kCircleSegments);
        vertices.push_back(std::cos(angle));
        vertices.push_back(std::sin(angle));
    }
    return vertices;
}

/**
 * @brief Genera los vértices de un semicírculo unitario (de 0 a π).
 *
 * El diámetro queda sobre el eje X y el cuerpo en el semieje positivo de Y.
 * El primer vértice es el centro (0,0) y los siguientes recorren el arco.
 * @return Vector intercalado (x, y) listo para subir a la GPU.
 */
std::vector<float> makeUnitSemicircleVertices() {
    std::vector<float> vertices;
    vertices.reserve(static_cast<std::size_t>(kSemicircleSegments + 2) * 2);

    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    for (int i = 0; i <= kSemicircleSegments; ++i) {
        const float angle = kPi * static_cast<float>(i)
                            / static_cast<float>(kSemicircleSegments);
        vertices.push_back(std::cos(angle));
        vertices.push_back(std::sin(angle));
    }
    return vertices;
}

/**
 * @brief Convierte una lista de puntos de estela en una cinta (triangle strip).
 * @param points Puntos ordenados del más antiguo al más reciente.
 * @return Vértices intercalados (x, y, alpha), dos por punto.
 *
 * Para cada punto se calcula la dirección de avance (diferencia central, salvo
 * en los extremos), su normal y los dos bordes de la cinta. Los puntos
 * degenerados (dirección casi nula) se omiten.
 */
std::vector<float> buildTrailVertices(const std::vector<Renderer::TrailPoint>& points) {
    std::vector<float> vertices;
    if (points.size() < 2) {
        return vertices;
    }

    vertices.reserve(points.size() * 2 * kTrailFloatsPerVertex);

    for (std::size_t i = 0; i < points.size(); ++i) {
        glm::vec2 direction{0.0f};
        if (i == 0) {
            direction = points[1].position - points[0].position;
        } else if (i + 1 == points.size()) {
            direction = points[i].position - points[i - 1].position;
        } else {
            direction = points[i + 1].position - points[i - 1].position;
        }

        const float directionLength = glm::length(direction);
        if (directionLength < kMinDirectionLength) {
            continue;
        }

        const glm::vec2 normal = glm::vec2(-direction.y, direction.x) / directionLength;
        const float halfWidth = points[i].width * 0.5f;
        const glm::vec2 left = points[i].position + normal * halfWidth;
        const glm::vec2 right = points[i].position - normal * halfWidth;

        vertices.push_back(left.x);
        vertices.push_back(left.y);
        vertices.push_back(points[i].alpha);

        vertices.push_back(right.x);
        vertices.push_back(right.y);
        vertices.push_back(points[i].alpha);
    }

    return vertices;
}

}  // namespace

bool Renderer::init(int screenWidth, int screenHeight) {
    if (!buildShaderProgram() || !buildTrailProgram() || !buildTextureProgram()) {
        return false;
    }

    uploadProjection(screenWidth, screenHeight);

    // La estela necesita blending para desvanecerse; las figuras opacas no cambian.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return setupGeometry();
}

bool Renderer::buildShaderProgram() {
    m_shaderProgram = createProgram(kVertexSrc, kFragmentSrc);
    if (m_shaderProgram == 0) {
        return false;
    }

    m_projectionLocation = glGetUniformLocation(m_shaderProgram, "uProjection");
    m_modelLocation = glGetUniformLocation(m_shaderProgram, "uModel");
    m_colorLocation = glGetUniformLocation(m_shaderProgram, "uColor");
    return true;
}

bool Renderer::buildTrailProgram() {
    m_trailProgram = createProgram(kTrailVertexSrc, kTrailFragmentSrc);
    if (m_trailProgram == 0) {
        return false;
    }

    m_trailProjectionLocation = glGetUniformLocation(m_trailProgram, "uProjection");
    m_trailColorLocation = glGetUniformLocation(m_trailProgram, "uColor");
    return true;
}

bool Renderer::buildTextureProgram() {
    m_textureProgram = createProgram(kTextureVertexSrc, kTextureFragmentSrc);
    if (m_textureProgram == 0) {
        return false;
    }

    m_textureProjectionLocation = glGetUniformLocation(m_textureProgram, "uProjection");
    m_textureModelLocation = glGetUniformLocation(m_textureProgram, "uModel");
    m_textureAlphaLocation = glGetUniformLocation(m_textureProgram, "uAlpha");
    m_textureSamplerLocation = glGetUniformLocation(m_textureProgram, "uTexture");
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

    glUseProgram(m_trailProgram);
    glUniformMatrix4fv(m_trailProjectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(m_textureProgram);
    glUniformMatrix4fv(m_textureProjectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
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

    const std::vector<float> semicircleVertices = makeUnitSemicircleVertices();
    uploadMesh(semicircleVertices, m_semiVAO, m_semiVBO);
    m_semiVertexCount = static_cast<unsigned int>(semicircleVertices.size() / 2);

    uploadTextureQuad(m_textureVAO, m_textureVBO);

    // Malla dinámica de la estela: posición (vec2) + alpha (float) por vértice.
    glGenVertexArrays(1, &m_trailVAO);
    glGenBuffers(1, &m_trailVBO);

    glBindVertexArray(m_trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_trailVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    const auto stride = static_cast<GLsizei>(kTrailFloatsPerVertex * sizeof(float));
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return m_circleVAO != 0 && m_circleVBO != 0
        && m_rectVAO != 0 && m_rectVBO != 0
        && m_semiVAO != 0 && m_semiVBO != 0
        && m_textureVAO != 0 && m_textureVBO != 0
        && m_trailVAO != 0 && m_trailVBO != 0;
}

Texture Renderer::loadTexture(const std::string& path) {
    stbi_set_flip_vertically_on_load(1);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr) {
        std::cerr << "[Renderer] No se pudo cargar la textura: " << path << std::endl;
        return {};
    }

    Texture texture;
    texture.width = width;
    texture.height = height;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);

    return texture;
}

void Renderer::destroyTexture(Texture& texture) {
    if (texture.id != 0) {
        glDeleteTextures(1, &texture.id);
    }
    texture = Texture{};
}

void Renderer::drawTexture(const Texture& texture, const glm::vec2& center,
                           const glm::vec2& size, float alpha) {
    if (!texture.valid() || alpha <= 0.0f) {
        return;
    }

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(size.x * 0.5f, size.y * 0.5f, 1.0f));

    glUseProgram(m_textureProgram);
    glUniformMatrix4fv(m_textureModelLocation, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1f(m_textureAlphaLocation, alpha);
    glUniform1i(m_textureSamplerLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glBindVertexArray(m_textureVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::drawCircle(const glm::vec2& center, float radius, const glm::vec3& color,
                          float alpha) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(radius, radius, 1.0f));
    drawPrimitive(m_circleVAO, m_circleVertexCount, GL_TRIANGLE_FAN, model,
                  glm::vec4(color, alpha));
}

void Renderer::drawRect(const glm::vec2& center, const glm::vec2& size,
                        float angleRadians, const glm::vec3& color, float alpha) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::rotate(model, angleRadians, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(size.x * 0.5f, size.y * 0.5f, 1.0f));
    drawPrimitive(m_rectVAO, 4, GL_TRIANGLE_STRIP, model, glm::vec4(color, alpha));
}

void Renderer::drawSemicircle(const glm::vec2& center, float radius, float angleRadians,
                              const glm::vec3& color, float alpha) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::rotate(model, angleRadians, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(radius, radius, 1.0f));
    drawPrimitive(m_semiVAO, m_semiVertexCount, GL_TRIANGLE_FAN, model,
                  glm::vec4(color, alpha));
}

void Renderer::drawTrail(const std::vector<TrailPoint>& points, const glm::vec4& color) {
    const std::vector<float> vertices = buildTrailVertices(points);
    const auto vertexCount = static_cast<int>(vertices.size() / kTrailFloatsPerVertex);
    if (vertexCount < 4) {  // hacen falta al menos dos puntos para una cinta
        return;
    }

    glUseProgram(m_trailProgram);
    glUniform4fv(m_trailColorLocation, 1, glm::value_ptr(color));

    glBindVertexArray(m_trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_trailVBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
    glBindVertexArray(0);
}

void Renderer::drawPrimitive(unsigned int vao, unsigned int vertexCount, unsigned int mode,
                             const glm::mat4& model, const glm::vec4& color) {
    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_modelLocation, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4fv(m_colorLocation, 1, glm::value_ptr(color));

    glBindVertexArray(vao);
    glDrawArrays(mode, 0, static_cast<int>(vertexCount));
    glBindVertexArray(0);
}

void Renderer::cleanup() {
    if (m_trailVBO != 0) {
        glDeleteBuffers(1, &m_trailVBO);
        m_trailVBO = 0;
    }
    if (m_trailVAO != 0) {
        glDeleteVertexArrays(1, &m_trailVAO);
        m_trailVAO = 0;
    }
    if (m_textureVBO != 0) {
        glDeleteBuffers(1, &m_textureVBO);
        m_textureVBO = 0;
    }
    if (m_textureVAO != 0) {
        glDeleteVertexArrays(1, &m_textureVAO);
        m_textureVAO = 0;
    }
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
    if (m_semiVBO != 0) {
        glDeleteBuffers(1, &m_semiVBO);
        m_semiVBO = 0;
    }
    if (m_semiVAO != 0) {
        glDeleteVertexArrays(1, &m_semiVAO);
        m_semiVAO = 0;
    }
    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
    if (m_trailProgram != 0) {
        glDeleteProgram(m_trailProgram);
        m_trailProgram = 0;
    }
    if (m_textureProgram != 0) {
        glDeleteProgram(m_textureProgram);
        m_textureProgram = 0;
    }
}
