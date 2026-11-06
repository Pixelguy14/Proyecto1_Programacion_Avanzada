#include "core/Window.h"
#include "graphics/Renderer.h"

/**
 * @file main.cpp
 * @brief Punto de entrada del juego Fruit Ninja.
 *
 * Crea la ventana, inicializa el renderer y ejecuta el bucle principal
 * calculando el delta‑time entre frames.
 */
int main() {
    // Crear ventana de 1920x1080
    Window window(1920, 1080, "Fruit Ninja");
    if (!window.init()) {
        return -1;
    }

    // Inicializar renderer (carga shaders, VAO, etc.)
    Renderer renderer;
    renderer.init();

    // Bucle principal
    while (!window.shouldClose()) {
        // Calcular delta time
        float dt = window.updateDeltaTime();

        // Renderizado
        window.clear();
        renderer.render();
        window.swapBuffers();
    }

    // Limpieza
    renderer.cleanup();
    window.terminate();
    return 0;
}
