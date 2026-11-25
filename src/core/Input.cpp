#include "Input.h"

/**
 * @file Input.cpp
 * @brief Implementación de los callbacks de mouse y teclado de GLFW.
 */

bool Input::attach(GLFWwindow* window, int windowHeight) {
    if (window == nullptr) {
        return false;
    }

    m_windowHeight = windowHeight;

    // El user pointer permite que los callbacks estáticos lleguen a esta instancia.
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, &Input::cursorPositionCallback);
    glfwSetMouseButtonCallback(window, &Input::mouseButtonCallback);
    glfwSetKeyCallback(window, &Input::keyCallback);
    glfwSetCharCallback(window, &Input::charCallback);

    // La estela de la espada sustituye al cursor del sistema.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Posición inicial, antes del primer movimiento del mouse.
    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(window, &x, &y);
    setMousePosition(x, y);
    return true;
}

void Input::cursorPositionCallback(GLFWwindow* window, double xPos, double yPos) {
    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input != nullptr) {
        input->setMousePosition(xPos, yPos);
    }
}

void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) {
        return;
    }

    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input != nullptr) {
        input->m_mouseDown = (action == GLFW_PRESS);
    }
}

void Input::keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action,
                        int /*mods*/) {
    if (action != GLFW_PRESS || key < 0 || key > GLFW_KEY_LAST) {
        return;
    }

    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input != nullptr) {
        input->m_keysPressed[static_cast<std::size_t>(key)] = true;
    }
}

void Input::charCallback(GLFWwindow* window, unsigned int codepoint) {
    if (codepoint < 32 || codepoint > 126) {
        return;  // el atlas de texto solo cubre ASCII imprimible
    }

    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input != nullptr) {
        input->m_textInput.push_back(static_cast<char>(codepoint));
    }
}

void Input::setMousePosition(double x, double y) {
    // GLFW usa origen arriba-izquierda; el juego usa origen abajo-izquierda.
    m_mousePosition = Vector2D<float>{
        static_cast<float>(x),
        static_cast<float>(m_windowHeight) - static_cast<float>(y),
    };
}
