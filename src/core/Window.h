#pragma once

/**
 * @file Window.h
 * @brief Declaración de la clase Window que encapsula GLFW y el contexto OpenGL.
 *
 * ## Jerarquía de responsabilidades
 * Window es la capa más baja del juego: solo conoce GLFW, el contexto OpenGL,
 * el viewport y el reloj. No sabe nada de frutas, shaders ni entidades.
 *
 *   main()
 *     └── Window   (ciclo de vida, ventana, tiempo, eventos, limpieza)
 *
 * IMPORTANTE: Siempre se debe incluir glad/gl.h ANTES que GLFW.
 * GLFW_INCLUDE_NONE evita que GLFW incluya las cabeceras de OpenGL del sistema,
 * lo que causaría conflictos con las definiciones de GLAD.
 */

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>

class Window {
public:
    /**
     * @brief Constructor de la ventana.
     * @param width  Ancho en píxeles.
     * @param height Alto en píxeles.
     * @param title  Texto de la barra de título (se copia, no se guarda el puntero).
     */
    Window(int width, int height, const char* title);

    /**
     * @brief Libera la ventana y termina GLFW si todavía estaba activo.
     *
     * Es idempotente: llamar a terminate() y después destruir el objeto es seguro.
     */
    ~Window();

    /// Una ventana no se puede copiar: posee recursos únicos de GLFW.
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /**
     * @brief Inicializa GLFW, crea el contexto OpenGL 3.3 core y carga GLAD.
     * @return true si la inicialización tuvo éxito; false en caso de error.
     */
    bool init();

    /**
     * @brief Procesa los eventos pendientes de la ventana (teclado, mouse, cierre).
     *
     * Debe llamarse una vez por frame, antes de actualizar el estado del juego.
     */
    void pollEvents();

    /**
     * @brief Calcula el tiempo transcurrido desde la llamada anterior.
     * @return Delta time en segundos.
     */
    float updateDeltaTime();

    /**
     * @brief Indica si la ventana debe cerrarse (por ejemplo, el usuario pulsa X).
     * @return true si GLFW marcó la ventana para cerrarse.
     */
    bool shouldClose() const;

    /**
     * @brief Intercambia los buffers front/back para mostrar el frame dibujado.
     */
    void swapBuffers();

    /**
     * @brief Limpia el buffer de color con el color de fondo del juego.
     */
    void clear();

    /**
     * @brief Libera recursos y finaliza GLFW.
     */
    void terminate();

    /**
     * @brief Solicita el cierre de la ventana como si el usuario pulsara la X.
     *
     * El bucle principal termina en la siguiente comprobación de `shouldClose()`.
     */
    void requestClose();

    /// @return Ancho lógico solicitado al crear la ventana, en píxeles.
    int width() const { return m_width; }

    /// @return Alto lógico solicitado al crear la ventana, en píxeles.
    int height() const { return m_height; }

    /// @return Handle nativo de GLFW para registrar callbacks de entrada.
    GLFWwindow* handle() const { return m_window; }

private:
    /**
     * @brief Crea la ventana GLFW con el perfil de OpenGL 3.3 core.
     * @return true si la ventana se creó correctamente.
     */
    bool createWindow();

    /**
     * @brief Carga las funciones de OpenGL mediante GLAD y configura el viewport.
     * @return true si GLAD pudo inicializarse.
     */
    bool loadOpenGL();

private:
    int m_width;
    int m_height;
    std::string m_title;
    GLFWwindow* m_window;
    float m_lastTime;
    /// Indica si glfwInit() ya se ejecutó correctamente (para terminate() idempotente).
    bool m_glfwReady = false;
};
