# Project Development Process Documentation

## Overview
This document records the step‑by‑step development process for the **Fruit Ninja** C++ OpenGL project described in `documentacion1.md`.  It includes:
- Sprint goals and tasks
- Design decisions and references to *The Nature of Code*
- Build and run instructions: ver `readme.md` (fuente de verdad, con dependencias por sistema operativo).
- Code structure and conventions (including Doxygen‑style docstrings/comments)
- Checklist for each sprint

The file will be updated after each sprint with detailed explanations of the implementation, challenges, and how the code works.

---

## Sprint 0 – Foundations (Hello World)
**Goal:** Set up the development environment, configure CMake, create a window using GLFW, and render a simple rectangle.

### Tasks
1. Install required libraries (GLFW, GLAD, GLM).
2. Write a minimal `CMakeLists.txt`.
3. Create `src/main.cpp` with a basic game loop.
4. Implement a `Window` class (`src/core/Window.h/.cpp`).
5. Write a simple vertex/fragment shader.
6. Render a static rectangle.
7. Verify orthographic projection.

### References
- *The Nature of Code* – Chapter 2.6 (physics basics, but here we focus on rendering).
- OpenGL tutorials for GLFW/GLAD setup.

### Checklist
- [x] CMake builds successfully.
- [x] Window opens with correct size (1920×1080).
- [x] Rectangle appears (rojo, confirmado por el usuario).
- [x] Orthographic projection maps screen coordinates (0–1920, 0–1080).
- [x] Abstract base class `GameObject` defined (`update`, `render` virtuales puros).

### Decisiones de diseño
- **GLAD2** (generador Python) se usa en lugar del GLAD clásico. Requiere `jinja2` (`pip install jinja2 --user`).
- **`GLFW_INCLUDE_NONE`** definido antes de incluir GLFW para evitar conflictos con los headers de OpenGL del sistema.
- **Out-of-source build** forzado por guardia en `CMakeLists.txt`: siempre `cmake -S . -B build`. La guía de instalación completa (dependencias por sistema operativo) vive en `readme.md`.
- `cmake_minimum_required` se elevó a **3.16** porque GLFW 3.5 no configura con versiones anteriores. El resto de versiones mínimas (Python 3.8+, Jinja2 2.7–3.x, OpenGL 3.3) están en `readme.md`.
- `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` evita reconfigurar a mano al añadir `.cpp` nuevos en cada sprint.
- La proyección ortográfica se aplica como uniform `uProjection` en el vertex shader, usando `glm::ortho(0, 1920, 0, 1080)`. Las coordenadas de los vértices se expresan en píxeles de pantalla.
- `GameObject` es una clase base abstracta con `virtual update(float dt) = 0` y `virtual render() = 0`. Incluye `isAlive()` y destructor virtual.
- `Window` guarda el título en `std::string` (no en `const char*`) para evitar punteros colgantes. Es **no copiable** y su destructor llama a `terminate()`, que es idempotente.
- `Window::pollEvents()` se separó de `Window::updateDeltaTime()`: el bucle principal primero procesa eventos y luego calcula `dt` (una responsabilidad por método).
- `Renderer::init()` devuelve `bool` y delega en tres pasos con nombres explícitos:
  `buildShaderProgram()` → `uploadProjection()` → `setupGeometry()`.
  El EBO es un miembro (`m_EBO`) para poder liberarlo en `cleanup()`; antes era local y se fugaba.
- `main()` solo orquesta: crea ventana y renderer, llama a `runGameLoop()` y libera recursos. La jerarquía es `main → runGameLoop → Window/Renderer`.

### Convenciones de código y documentación
- **Doxygen en todo `src/`**: cada archivo usa `@file` y `@brief`; cada función pública lleva `@brief`, `@param` y `@return` cuando aplica. Los helpers privados también se documentan.
- **Funciones pequeñas con una sola responsabilidad**: si un método mezcla inicialización, configuración y dibujado, se parte en helpers privados con nombre descriptivo.
- **Sin `new`/`delete` manuales todavía**: los recursos de OpenGL se liberan con `cleanup()` (VAO, VBO, EBO, programa). A partir del Sprint 1 las entidades se gestionarán con `std::vector` y punteros inteligentes.
- **Constantes con nombre** en lugar de números mágicos (resolución, color de fondo, resolución lógica).
- Compila sin advertencias con `-Wall -Wextra -Wpedantic`.
- `src/shaders/*.glsl` son copias de referencia de los shaders embebidos en `Renderer.cpp`; se cargarán desde archivo al crear la clase `Shader` (Sprint 1).


---

## Sprint 1 – Physics & Movement
> (To be filled after completion)

## Sprint 2 – Blade Trail
> (To be filled after completion)

## Sprint 3 – Collision & Cutting
> (To be filled after completion)

## Sprint 4 – Game Loop, UI & Particles
> (To be filled after completion)

## Sprint 5 – Polish & Juice
> (To be filled after completion)

---

## Bibliography
- *The Nature of Code* – Daniel Shiffman.
- OpenGL 4.6 Documentation.
- GLFW, GLAD, GLM official docs.

---

*This file will be expanded incrementally as we implement each sprint.*
