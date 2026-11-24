#version 330 core
// Copia de referencia del vertex shader embebido en src/graphics/Renderer.cpp.
// Se usará como archivo externo cuando se cree la clase Shader (Sprint 2/3).
layout(location = 0) in vec2 aPos;

// uProjection: pantalla (0..width, 0..height) -> NDC.
// uModel: escala, rotación y traslación de la figura.
uniform mat4 uProjection;
uniform mat4 uModel;

void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
