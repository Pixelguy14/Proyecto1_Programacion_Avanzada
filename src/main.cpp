#include "core/Input.h"
#include "core/Window.h"
#include "game/GameSession.h"
#include "game/GameState.h"
#include "game/GameUi.h"
#include "graphics/Background.h"
#include "graphics/Renderer.h"
#include "math/Collision.h"

#include <algorithm>
#include <iostream>

/**
 * @file main.cpp
 * @brief Punto de entrada y máquina de estados del juego (Sprints 1–4).
 *
 * ## Jerarquía
 *   main()
 *     ├── Window        ventana, contexto OpenGL, tiempo y eventos
 *     ├── Input         mouse y teclado mediante callbacks GLFW
 *     ├── Renderer      primitivas 2D y texturas
 *     ├── Background    cuatro escenarios con crossfade
 *     ├── GameSession   spawner, entidades, espada, puntaje y vidas
 *     ├── GameUi        tipografía, HUD y overlays
 *     └── runGameLoop() máquina de estados
 *
 * ## Máquina de estados
 *   Menu ──Enter/Espacio──▶ Playing ──Esc/P──▶ Paused
 *     ▲                        │                  │
 *     │                        │ 0 vidas          │ Esc/P
 *     │                        ▼                  │
 *     └──────Esc──────────── GameOver ◀──────────┘
 *                              │
 *                              └──Enter/Espacio──▶ Playing
 *
 * Cada estado tiene su propio fondo (outside, room, inside, nursery) y su
 * propio conjunto de textos. Solo `Playing` avanza la simulación.
 */

namespace {

/// Resolución lógica del juego (coincide con la proyección ortográfica).
constexpr int kWindowWidth = 1920;
constexpr int kWindowHeight = 1080;
constexpr const char* kWindowTitle = "Fruit Ninja";

/// Tope de dt para evitar saltos físicos tras pausas o arrastres de ventana.
constexpr float kMaxFrameTime = 0.05f;

/**
 * @brief Resultado de procesar la entrada de un estado.
 */
struct StateTransition {
    GameState state;     ///< Estado al que se quiere pasar.
    bool resetSession;   ///< true si hay que reiniciar la partida al entrar.
};

/**
 * @brief Asocia cada estado con su escenario de fondo.
 * @param state Estado actual de la máquina.
 * @return Fondo que debe mostrarse.
 *
 * El fondo es cosmético: no altera la dificultad ni la generación de frutas.
 */
BackgroundScene backgroundFor(GameState state) {
    switch (state) {
        case GameState::kMenu:
            return BackgroundScene::kOutside;
        case GameState::kPlaying:
            return BackgroundScene::kRoom;
        case GameState::kPaused:
            return BackgroundScene::kInside;
        case GameState::kGameOver:
            return BackgroundScene::kNursery;
    }
    return BackgroundScene::kOutside;
}

/**
 * @brief Traduce las teclas del frame a una transición de estado.
 * @param current Estado actual.
 * @param input   Entrada del frame.
 * @param window  Ventana, para poder solicitar el cierre desde el menú.
 * @return Estado siguiente y si hay que reiniciar la sesión.
 *
 * Las teclas de movimiento (Enter/Espacio, Esc/P, R, M) son las mismas en
 * todos los estados; solo cambia su significado según el estado actual.
 */
StateTransition transitionFor(GameState current, const Input& input, Window& window) {
    const bool enter = input.wasKeyPressed(GLFW_KEY_ENTER)
                    || input.wasKeyPressed(GLFW_KEY_KP_ENTER)
                    || input.wasKeyPressed(GLFW_KEY_SPACE);
    const bool pauseKey = input.wasKeyPressed(GLFW_KEY_ESCAPE)
                       || input.wasKeyPressed(GLFW_KEY_P);
    const bool restartKey = input.wasKeyPressed(GLFW_KEY_R);
    const bool menuKey = input.wasKeyPressed(GLFW_KEY_M);

    switch (current) {
        case GameState::kMenu:
            if (enter) {
                return {GameState::kPlaying, true};
            }
            if (input.wasKeyPressed(GLFW_KEY_ESCAPE)) {
                window.requestClose();
            }
            break;

        case GameState::kPlaying:
            if (pauseKey) {
                return {GameState::kPaused, false};
            }
            break;

        case GameState::kPaused:
            if (pauseKey) {
                return {GameState::kPlaying, false};
            }
            if (restartKey) {
                return {GameState::kPlaying, true};
            }
            if (menuKey) {
                return {GameState::kMenu, false};
            }
            break;

        case GameState::kGameOver:
            if (enter || restartKey) {
                return {GameState::kPlaying, true};
            }
            if (pauseKey) {
                return {GameState::kMenu, false};
            }
            break;
    }

    return {current, false};
}

/**
 * @brief Ejecuta el bucle principal hasta que el usuario cierre la ventana.
 * @param window     Ventana ya inicializada.
 * @param renderer   Renderer ya inicializado.
 * @param input      Entrada registrada sobre la ventana.
 * @param background Fondos ya cargados.
 * @param session    Sesión de juego reutilizable entre partidas.
 * @param ui         Textos y overlays ya inicializados.
 */
void runGameLoop(Window& window, Renderer& renderer, Input& input, Background& background,
                 GameSession& session, GameUi& ui) {
    GameState state = GameState::kMenu;
    background.setScene(backgroundFor(state), true);

    while (!window.shouldClose()) {
        input.beginFrame();
        window.pollEvents();

        const float dt = std::min(window.updateDeltaTime(), kMaxFrameTime);

        const StateTransition transition = transitionFor(state, input, window);
        if (transition.state != state) {
            if (transition.resetSession) {
                session.reset();
            }
            state = transition.state;
        }

        if (state == GameState::kPlaying) {
            session.update(dt);
            if (session.isGameOver()) {
                state = GameState::kGameOver;
            }
        }

        // La explosión de la bomba sigue animándose aunque la partida terminó.
        if (state == GameState::kGameOver) {
            session.updateEffects(dt);
        }

        background.setScene(backgroundFor(state));
        background.update(dt);

        window.clear();
        background.render(renderer);

        // En pausa y game over se congela la escena, pero se sigue dibujando.
        if (state != GameState::kMenu) {
            session.render(renderer);
        }

        switch (state) {
            case GameState::kMenu:
                ui.renderMenu(renderer);
                break;
            case GameState::kPlaying:
                ui.renderHud(renderer, session.score(), session.lives());
                break;
            case GameState::kPaused:
                ui.renderHud(renderer, session.score(), session.lives());
                ui.renderPaused(renderer);
                break;
            case GameState::kGameOver:
                ui.renderGameOver(renderer, session.score(), session.endedByBomb());
                break;
        }

        window.swapBuffers();
    }
}

}  // namespace

/**
 * @brief Crea ventana, renderer, entrada, fondos y UI; ejecuta y libera todo.
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

    Background background;
    if (!background.init(renderer, static_cast<float>(window.width()),
                         static_cast<float>(window.height()))) {
        std::cerr << "[main] Algunos fondos no se pudieron cargar." << std::endl;
    }

    GameSession session(input, static_cast<float>(window.width()),
                        static_cast<float>(window.height()));

    std::cout << "[main] Colisiones: "
              << (Collision::usesAvx() ? "AVX" : "escalar") << std::endl;

    GameUi ui;
    if (!ui.init(window.width(), window.height())) {
        std::cerr << "[main] La interfaz de texto no se pudo inicializar." << std::endl;
    }

    runGameLoop(window, renderer, input, background, session, ui);

    ui.cleanup();
    background.cleanup(renderer);
    renderer.cleanup();
    window.terminate();
    return 0;
}
