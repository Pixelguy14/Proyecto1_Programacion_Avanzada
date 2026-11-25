#pragma once

/**
 * @file Input.h
 * @brief Captura de mouse y teclado mediante callbacks de GLFW (Sprints 2–4).
 *
 * Input es la única pieza que conoce GLFW para eventos de entrada. Traduce las
 * coordenadas de ventana de GLFW (origen arriba-izquierda, Y hacia abajo) al
 * espacio del juego (origen abajo-izquierda, Y hacia arriba) que usa la
 * proyección ortográfica.
 *
 *   Blade::update()
 *     └── Input::mousePosition()
 *           └── GLFW cursor-position callback
 *
 * ## Nota de propiedad
 * Input registra su propio puntero en `glfwSetWindowUserPointer`. Si en el
 * futuro otro sistema necesita callbacks, habrá que centralizar el user pointer
 * (por ejemplo, en Window) antes de añadir más manejadores.
 */

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "math/Vector2D.h"

#include <array>

class Input {
public:
    Input() = default;

    /// Input no se copia: sus callbacks apuntan a esta instancia concreta.
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    /**
     * @brief Registra los callbacks de cursor y botones en una ventana GLFW.
     * @param window       Ventana ya creada.
     * @param windowHeight Alto lógico de la ventana (para invertir el eje Y).
     * @return true si la ventana es válida y los callbacks quedaron registrados.
     *
     * También oculta el cursor del sistema: la estela de la espada hace de
     * puntero visible.
     */
    bool attach(GLFWwindow* window, int windowHeight);

    /**
     * @brief Posición actual del mouse en coordenadas del juego.
     * @return Posición en píxeles con origen en la esquina inferior izquierda.
     */
    Vector2D<float> mousePosition() const { return m_mousePosition; }

    /// @return true mientras el botón izquierdo esté presionado.
    bool isMouseDown() const { return m_mouseDown; }

    /**
     * @brief Limpia las teclas pulsadas en el frame anterior.
     *
     * Debe llamarse antes de `Window::pollEvents()`: los callbacks de teclado
     * del frame actual vuelven a llenar el registro de pulsaciones.
     */
    void beginFrame() { m_keysPressed.fill(false); }

    /**
     * @brief Comprueba si una tecla se pulsó en el frame actual.
     * @param key Código de tecla de GLFW (por ejemplo `GLFW_KEY_ENTER`).
     * @return true solo en el frame en que la tecla pasó de suelta a pulsada.
     */
    bool wasKeyPressed(int key) const {
        return key >= 0 && key <= GLFW_KEY_LAST && m_keysPressed[static_cast<std::size_t>(key)];
    }

private:
    /**
     * @brief Callback de GLFW: actualiza la posición al mover el cursor.
     * @param window Ventana que emitió el evento.
     * @param xPos   Coordenada X en píxeles de ventana.
     * @param yPos   Coordenada Y en píxeles de ventana (origen arriba).
     */
    static void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos);

    /**
     * @brief Callback de GLFW: actualiza el estado del botón izquierdo.
     * @param window Ventana que emitió el evento.
     * @param button Botón que cambió.
     * @param action GLFW_PRESS o GLFW_RELEASE.
     * @param mods   Modificadores de teclado (no usados).
     */
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    /**
     * @brief Callback de GLFW: registra las teclas pulsadas en este frame.
     * @param window Ventana que emitió el evento.
     * @param key    Código de tecla de GLFW.
     * @param action GLFW_PRESS, GLFW_RELEASE o GLFW_REPEAT.
     *
     * Solo se registra `GLFW_PRESS`; las repeticiones del sistema no cuentan
     * como una pulsación nueva.
     */
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    /**
     * @brief Guarda la posición convirtiendo el eje Y al espacio del juego.
     * @param x Coordenada X en píxeles de ventana.
     * @param y Coordenada Y en píxeles de ventana (origen arriba).
     */
    void setMousePosition(double x, double y);

private:
    Vector2D<float> m_mousePosition{};  ///< Posición en coordenadas del juego.
    int m_windowHeight = 0;             ///< Altura lógica para invertir el eje Y.
    bool m_mouseDown = false;           ///< Estado del botón izquierdo.
    std::array<bool, GLFW_KEY_LAST + 1> m_keysPressed{};  ///< Pulsaciones del frame.
};
