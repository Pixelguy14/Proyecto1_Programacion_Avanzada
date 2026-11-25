#include "Window.h"
#include <glad/gl.h>
#include <iostream>

/**
 * @file Window.cpp
 * @brief Implementación de la clase Window (GLFW + GLAD).
 *
 * Cada método público hace una sola cosa y delega los detalles en los helpers
 * privados createWindow() y loadOpenGL(), de modo que init() se lea como una
 * secuencia corta de pasos.
 */

namespace {
/// Color de fondo del juego (gris oscuro) usado por clear().
constexpr float kClearR = 0.1f;
constexpr float kClearG = 0.1f;
constexpr float kClearB = 0.1f;
constexpr float kClearA = 1.0f;
}  // namespace

Window::Window(int width, int height, const char* title)
    : m_width(width),
      m_height(height),
      m_title(title != nullptr ? title : ""),
      m_window(nullptr),
      m_lastTime(0.0f) {}

Window::~Window() {
    terminate();
}

bool Window::init() {
    if (!glfwInit()) {
        std::cerr << "[Window] Failed to initialize GLFW" << std::endl;
        return false;
    }
    m_glfwReady = true;

    if (!createWindow()) {
        terminate();
        return false;
    }

    if (!loadOpenGL()) {
        terminate();
        return false;
    }

    m_lastTime = static_cast<float>(glfwGetTime());
    return true;
}

bool Window::createWindow() {
    // OpenGL 3.3 core profile (requerido por los shaders #version 330).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Buffer de profundidad para las mallas 3D de las frutas (Sprint 5).
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (m_window == nullptr) {
        std::cerr << "[Window] Failed to create GLFW window" << std::endl;
        return false;
    }

    glfwMakeContextCurrent(m_window);
    return true;
}

bool Window::loadOpenGL() {
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "[Window] Failed to initialize GLAD" << std::endl;
        return false;
    }

    glViewport(0, 0, m_width, m_height);
    return true;
}

void Window::pollEvents() {
    glfwPollEvents();
}

float Window::updateDeltaTime() {
    const float current = static_cast<float>(glfwGetTime());
    const float dt = current - m_lastTime;
    m_lastTime = current;
    return dt;
}

bool Window::shouldClose() const {
    return m_window != nullptr && glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() {
    if (m_window != nullptr) {
        glfwSwapBuffers(m_window);
    }
}

void Window::clear() {
    glClearColor(kClearR, kClearG, kClearB, kClearA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::requestClose() {
    if (m_window != nullptr) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}

void Window::terminate() {    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_glfwReady) {
        glfwTerminate();
        m_glfwReady = false;
    }
}
