#include "core/GameObject.h"
#include "core/Input.h"
#include "core/Window.h"
#include "entities/Blade.h"
#include "entities/Spawner.h"
#include "graphics/Renderer.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

/**
 * @file main.cpp
 * @brief Punto de entrada del juego Fruit Ninja.
 *
 * ## Jerarquía
 *   main()
 *     ├── Window    → ventana, contexto OpenGL, tiempo y eventos
 *     ├── Input     → posición del mouse mediante callbacks GLFW
 *     ├── Renderer  → primitivas de dibujado (círculo, rectángulo, estela)
 *     ├── Spawner   → crea frutas cada cierto tiempo
 *     ├── Blade     → historial del mouse y estela de la espada
 *     └── runGameLoop()
 *           ├── trySpawn()          (añade frutas)
 *           ├── updateObjects()     (física de cada entidad)
 *           ├── removeDeadObjects() (libera las que ya cayeron)
 *           ├── blade.update()      (registra el recorrido del mouse)
 *           └── renderObjects() + blade.render()
 */

namespace {

/// Resolución lógica del juego (coincide con la proyección ortográfica).
constexpr int kWindowWidth = 1920;
constexpr int kWindowHeight = 1080;
constexpr const char* kWindowTitle = "Fruit Ninja";

/// Tope de dt para evitar saltos físicos tras pausas o arrastres de ventana.
constexpr float kMaxFrameTime = 0.05f;

/// Contenedor polimórfico de entidades; posee los objetos con unique_ptr.
using GameObjectList = std::vector<std::unique_ptr<GameObject>>;

/**
 * @brief Actualiza la física/lógica de todas las entidades.
 * @param objects Lista de entidades.
 * @param dt      Delta time en segundos.
 */
void updateObjects(GameObjectList& objects, float dt) {
    for (auto& object : objects) {
        object->update(dt);
    }
}

/**
 * @brief Elimina de la lista las entidades marcadas como no vivas.
 * @param objects Lista de entidades (se modifica in-place).
 *
 * unique_ptr libera la memoria automáticamente al borrar el elemento.
 */
void removeDeadObjects(GameObjectList& objects) {
    const auto isDead = [](const std::unique_ptr<GameObject>& object) {
        return !object->isAlive();
    };
    objects.erase(std::remove_if(objects.begin(), objects.end(), isDead), objects.end());
}

/**
 * @brief Dibuja todas las entidades.
 * @param objects  Lista de entidades.
 * @param renderer Renderer compartido.
 */
void renderObjects(const GameObjectList& objects, Renderer& renderer) {
    for (const auto& object : objects) {
        object->render(renderer);
    }
}

/**
 * @brief Ejecuta el game loop hasta que el usuario cierre la ventana.
 * @param window   Ventana ya inicializada.
 * @param renderer Renderer ya inicializado.
 * @param spawner  Spawner ligado a la resolución de la ventana.
 * @param input    Entrada ya registrada sobre la ventana.
 *
 * La espada se dibuja al final para que su estela quede por encima de las frutas.
 */
void runGameLoop(Window& window, Renderer& renderer, Spawner& spawner, const Input& input) {
    Blade blade(input);

    GameObjectList objects;
    objects.reserve(64);  // capacidad inicial para evitar reallocaciones frecuentes

    while (!window.shouldClose()) {
        window.pollEvents();

        const float dt = std::min(window.updateDeltaTime(), kMaxFrameTime);

        if (auto fruit = spawner.trySpawn(dt)) {
            objects.push_back(std::move(fruit));
        }

        updateObjects(objects, dt);
        removeDeadObjects(objects);
        blade.update(dt);

        window.clear();
        renderObjects(objects, renderer);
        blade.render(renderer);
        window.swapBuffers();
    }
}

}  // namespace

/**
 * @brief Crea la ventana, renderer, entrada y spawner; ejecuta y libera recursos.
 * @return 0 si el juego terminó correctamente; 1 si falló la inicialización.
 */
int main() {
    Window window(kWindowWidth, kWindowHeight, kWindowTitle);
    if (!window.init()) {
        std::cerr << "[main] No se pudo inicializar la ventana." << std::endl;
        return 1;
    }

    Renderer renderer;
    if (!renderer.init(window.width(), window.height())) {
        std::cerr << "[main] No se pudo inicializar el renderer." << std::endl;
        window.terminate();
        return 1;
    }

    Input input;
    if (!input.attach(window.handle(), window.height())) {
        std::cerr << "[main] No se pudo registrar la entrada." << std::endl;
        renderer.cleanup();
        window.terminate();
        return 1;
    }

    Spawner spawner(static_cast<float>(window.width()),
                    static_cast<float>(window.height()));

    runGameLoop(window, renderer, spawner, input);

    renderer.cleanup();
    window.terminate();
    return 0;
}
