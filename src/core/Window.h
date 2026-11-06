#pragma once

/**
 * @file Window.h
 * @brief Declaración de la clase Window que encapsula GLFW y manejo básico de OpenGL.
 *
 * IMPORTANTE: Siempre se debe incluir glad/gl.h ANTES que GLFW.
 * GLFW_INCLUDE_NONE evita que GLFW incluya las cabeceras de OpenGL del sistema,
 * lo que causaría conflictos con las definiciones de GLAD.
 */

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Window {
public:
    /**
     * @brief Constructor de la ventana.
     * @param width  Ancho en píxeles.
     * @param height Alto en píxeles.
     * @param title  Texto de la barra de título.
     */
    Window(int width, int height, const char* title);

    /**
     * @brief Inicializa GLFW, crea el contexto OpenGL y configura la proyección.
     * @return true si la inicialización tuvo éxito.
     */
    bool init();

    /**
     * @brief Calcula el tiempo transcurrido entre el frame actual y el anterior.
     * @return Delta time en segundos.
     */
    float updateDeltaTime();

    /**
     * @brief Indica si la ventana debe cerrarse (por ejemplo, el usuario pulsa X).
     */
    bool shouldClose() const;

    /**
     * @brief Intercambia los buffers front/back.
     */
    void swapBuffers();

    /**
     * @brief Limpia el buffer de color con un color de fondo por defecto.
     */
    void clear();

    /**
     * @brief Libera recursos y finaliza GLFW.
     */
    void terminate();

private:
    int m_width;
    int m_height;
    const char* m_title;
    GLFWwindow* m_window;
    float m_lastTime;
};
