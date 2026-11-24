#version 330 core
// Copia de referencia del fragment shader embebido en src/graphics/Renderer.cpp.
// Se usará como archivo externo cuando se cree la clase Shader (Sprint 2/3).
uniform vec3 uColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(uColor, 1.0);
}
