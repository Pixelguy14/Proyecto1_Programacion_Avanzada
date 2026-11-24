#version 330 core
// Copia de referencia del fragment shader de la estela (Renderer.cpp).
// El alpha final combina el color base con el alpha interpolado del vértice.
uniform vec4 uColor;

in float vAlpha;
out vec4 FragColor;

void main() {
    FragColor = vec4(uColor.rgb, uColor.a * vAlpha);
}
