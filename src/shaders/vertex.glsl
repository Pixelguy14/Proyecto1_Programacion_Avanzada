#version 330 core
// Copia de referencia del vertex shader embebido en src/graphics/Renderer.cpp.
// Se usará como archivo externo cuando se cree la clase Shader (Sprint 1).
layout(location = 0) in vec2 aPos;

// Matriz de proyección ortográfica: convierte coordenadas de pantalla a NDC.
uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
