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
- `GameObject` es una clase base abstracta con `virtual update(float dt) = 0` y `virtual render(Renderer&) = 0`. Incluye `isAlive()` y destructor virtual. El renderer se pasa como argumento (Sprint 1) para evitar un singleton global.
- `Window` guarda el título en `std::string` (no en `const char*`) para evitar punteros colgantes. Es **no copiable** y su destructor llama a `terminate()`, que es idempotente.
- `Window::pollEvents()` se separó de `Window::updateDeltaTime()`: el bucle principal primero procesa eventos y luego calcula `dt` (una responsabilidad por método).
- `Renderer::init()` devuelve `bool` y delega en tres pasos con nombres explícitos:
  `buildShaderProgram()` → `uploadProjection()` → `setupGeometry()`.
  El EBO es un miembro (`m_EBO`) para poder liberarlo en `cleanup()`; antes era local y se fugaba.
- `main()` solo orquesta: crea ventana y renderer, llama a `runGameLoop()` y libera recursos. La jerarquía es `main → runGameLoop → Window/Renderer`.

### Convenciones de código y documentación
- **Doxygen en todo `src/`**: cada archivo usa `@file` y `@brief`; cada función pública lleva `@brief`, `@param` y `@return` cuando aplica. Los helpers privados también se documentan.
- **Funciones pequeñas con una sola responsabilidad**: si un método mezcla inicialización, configuración y dibujado, se parte en helpers privados con nombre descriptivo.
- **Sin `new`/`delete` manuales**: los recursos de OpenGL se liberan con `cleanup()` y las entidades se gestionan con `std::vector<std::unique_ptr<GameObject>>` (Sprint 1).
- **Constantes con nombre** en lugar de números mágicos (resolución, color de fondo, resolución lógica).
- Compila sin advertencias con `-Wall -Wextra -Wpedantic`.
- `src/shaders/*.glsl` son copias de referencia de los shaders embebidos en `Renderer.cpp`; se cargarán desde archivo al crear la clase `Shader` (Sprint 2/3).


---

## Sprint 1 – Physics & Movement
**Goal:** Give the fruits life: parabolic movement under gravity, random launches and rotation, with dynamic memory management via the STL.

### Tasks
1. Create the `Fruit` entity (position, velocity, acceleration, radius) inheriting from `GameObject`.
2. Implement semi‑implicit Euler integration in a physics manager: `velocity += gravity * dt; position += velocity * dt`.
3. Implement a `Spawner` that launches fruits from below the screen with a random velocity.
4. Add visual rotation (angular velocity).
5. Manage the population with `std::vector<std::unique_ptr<GameObject>>` (STL + RAII).

### References
- *The Nature of Code* – Chapter 2.6 (gravity and force modeling).

### Implementación
- `src/physics/Physics2D.h/.cpp`
  - `PhysicsBody` agrupa posición, velocidad y aceleración (sin dependencias de OpenGL).
  - `Physics2D::integrate()` aplica la aceleración y avanza un paso semi-implícito.
  - `Physics2D::integrateRotation()` avanza el ángulo con `fmod` para mantenerlo acotado.
  - `Physics2D::kGravity = (0, -980)` px/s².
- `src/entities/Fruit.h/.cpp`
  - Hereda de `GameObject`; implementa `update(float dt)` y `render(Renderer&)`.
  - `update()` integra la física y la rotación; si cae bajo la pantalla y va descendiendo, pone `m_alive = false`.
  - `render()` dibuja un círculo del color de la fruta y un tallo rotado (el tallo hace visible la rotación).
  - Expone `position()` y `radius()` para la colisión del Sprint 3.
- `src/entities/Spawner.h/.cpp`
  - Temporizador aleatorio de 0.55–1.15 s; la primera llamada lanza una fruta de inmediato.
  - Nace en la franja central (20 %–80 % del ancho), justo bajo el borde inferior.
  - La velocidad vertical se calcula para alcanzar un ápice aleatorio entre 45 % y 80 % de la altura: `v = sqrt(2·g·h)`.
  - Tamaño de fruta, velocidad horizontal, giro y color son aleatorios (paleta de 5 colores).
- `src/graphics/Renderer.h/.cpp`
  - Sustituye el rectángulo fijo por primitivas: `drawCircle()` y `drawRect()` con `uModel` (traslación/rotación/escala) y `uColor`.
  - Dos mallas unitarias reutilizables: círculo (GL_TRIANGLE_FAN, 64 segmentos) y quad (GL_TRIANGLE_STRIP).
- `src/main.cpp`
  - `GameObjectList = std::vector<std::unique_ptr<GameObject>>` y helpers `updateObjects`, `removeDeadObjects`, `renderObjects`.
  - `dt` se recorta a 50 ms para evitar saltos físicos tras pausas o al arrastrar la ventana.

### Checklist
- [x] `Fruit` hereda de `GameObject` y sobreescribe `update`/`render`.
- [x] Integración de Euler semi‑implícita con gravedad.
- [x] `Spawner` lanza frutas desde abajo con velocidad aleatoria.
- [x] Rotación visual (tallo girando con la fruta).
- [x] Población dinámica con `std::unique_ptr` (sin `new`/`delete` manuales).
- [x] Las frutas fuera de pantalla se eliminan sin fugas.
- [x] Compila sin advertencias (`-Wall -Wextra -Wpedantic`).
- [x] Verificado visualmente: 3 frutas en vuelo, colores y tallos distintos.

### Decisiones de diseño
- **`glm::vec2` en lugar de `Vector2D`**: la clase de vectores con plantillas y sobrecarga de operadores corresponde al Sprint 2. Cambiar ahora duplicaría trabajo, así que la física usa GLM y se migrará cuando toque.
- **`render(Renderer&)` en lugar de `render()`**: mantiene la firma virtual pura del enunciado sin recurrir a un Renderer global. Las entidades no incluyen `glad` ni tocan OpenGL.
- **Círculo + tallo**: un círculo girando es indistinguible de uno quieto; el tallo rotado demuestra la velocidad angular y prepara el dibujado de frutas con orientación.
- **Parámetros relativos a la resolución**: radios, velocidades y ápice se expresan como fracciones de ancho/alto, de modo que el juego se vea igual en 1080p u otra resolución.
- **Ápice → velocidad**: en lugar de números mágicos para `vy`, se despeja de `v² = 2·g·h`. Cambiar la gravedad o la altura objetivo sigue dando un vuelo coherente.
- **Muerte por caída**: solo se elimina cuando `velocity.y < 0` y salió por abajo; así no se mata a la fruta recién lanzada desde `y = -radius`.
- **`dt` acotado**: evita que un frame largo (ventana arrastrada, pestaña en segundo plano) teletransporte las frutas.
- **Eliminación con `erase(remove_if(...))`**: patrón idiomático de STL; `unique_ptr` libera la memoria al borrar el elemento.
- **Tope de complejidad**: el bucle dibuja fruta por fruta; el batcheo/SIMD de partículas llega en el Sprint 4, cuando haya cientos de objetos.

### Verificación
- Compilación limpia y ejecución sin errores.
- Captura de la ventana (`build/sprint1.png`, no versionada) con tres frutas en distintas fases del vuelo y tallos con ángulos distintos.

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
