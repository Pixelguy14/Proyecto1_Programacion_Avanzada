#version 330 core
// Copia de referencia del fragment shader embebido en src/graphics/Renderer.cpp.
// Se usará como archivo externo cuando se cree la clase Shader (Sprint 1).
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // rojo
}
