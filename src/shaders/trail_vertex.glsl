#version 330 core
// Copia de referencia del vertex shader de la estela (Renderer.cpp).
// La posición ya viene en píxeles de pantalla; alpha es por vértice.
layout(location = 0) in vec2 aPos;
layout(location = 1) in float aAlpha;

uniform mat4 uProjection;

out float vAlpha;

void main() {
    vAlpha = aAlpha;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
