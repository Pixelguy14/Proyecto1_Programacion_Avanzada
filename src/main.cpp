#include "core/Window.h"
#include "graphics/Renderer.h"

#include <iostream>

/**
 * @file main.cpp
 * @brief Punto de entrada del juego Fruit Ninja.
 *
 * ## Jerarquía
 *   main()
 *     ├── Window    → ventana, contexto OpenGL, tiempo y eventos
 *     ├── Renderer  → shaders, geometría y dibujado
 *     └── runGameLoop() → ciclo: eventos → update → render → present
 */

namespace {

/// Resolución lógica del juego (coincide con la proyección ortográfica).
constexpr int kWindowWidth = 1920;
constexpr int kWindowHeight = 1080;
constexpr const char* kWindowTitle = "Fruit Ninja";

/**
 * @brief Ejecuta el game loop hasta que el usuario cierre la ventana.
 * @param window   Ventana ya inicializada.
 * @param renderer Renderer ya inicializado.
 *
 * En el Sprint 0 solo se limpia la pantalla y se dibuja el rectángulo.
 * A partir del Sprint 1, aquí se actualizarán las entidades con dt.
 */
void runGameLoop(Window& window, Renderer& renderer) {
    while (!window.shouldClose()) {
        window.pollEvents();

        [[maybe_unused]] const float dt = window.updateDeltaTime();

        window.clear();
        renderer.render();
        window.swapBuffers();
    }
}

}  // namespace

/**
 * @brief Crea la ventana y el renderer, ejecuta el juego y libera los recursos.
 * @return 0 si el juego terminó correctamente; 1 si falló la inicialización.
 */
int main() {
    Window window(kWindowWidth, kWindowHeight, kWindowTitle);
    if (!window.init()) {
        std::cerr << "[main] No se pudo inicializar la ventana." << std::endl;
        return 1;
    }

    Renderer renderer;
    if (!renderer.init()) {
        std::cerr << "[main] No se pudo inicializar el renderer." << std::endl;
        window.terminate();
        return 1;
    }

    runGameLoop(window, renderer);

    renderer.cleanup();
    window.terminate();
    return 0;
}
