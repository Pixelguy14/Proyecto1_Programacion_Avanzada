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
**Goal:** Capture the mouse, keep a short history of its path and render it as a thick, fading trail.

### Tasks
1. Capture the mouse position using GLFW callbacks.
2. Create a `Blade` class that stores the last 10–15 positions (`std::deque`).
3. Render the history as a thick strip (`GL_LINE_STRIP` conceptually) with fading.
4. Add a visual fade: alpha and width decrease toward the oldest points.

### References
- *The Nature of Code* – tracking and vector math.
- GLFW input guide (cursor position and mouse button callbacks).

### Implementación
- `src/math/Vector2D.h`
  - Plantilla `Vector2D<T>` con `+`, `-`, `*`, `/`, `+=`, `-=`, `*=`, `/=`, `==`, negación, `dot`, `length`, `distanceTo`, `normalized` y `perpendicular`.
  - Alias `Vector2f`. GLM se mantiene solo para matrices en el renderer.
- `src/core/Input.h/.cpp`
  - `attach()` registra `glfwSetCursorPosCallback` y `glfwSetMouseButtonCallback`, oculta el cursor y guarda la posición inicial.
  - `glfwSetWindowUserPointer` conecta los callbacks estáticos con la instancia de `Input`.
  - Convierte Y de GLFW (origen arriba) al espacio del juego (origen abajo).
- `src/entities/Blade.h/.cpp`
  - `std::deque<TrailPoint>` con posición y antigüedad; máximo 24 puntos.
  - Un punto se añade solo si el mouse se movió ≥ 2 px (evita duplicados).
  - Los puntos caducan a los 0.18 s, así que la estela se desvanece incluso al detener el mouse.
  - `render()` calcula grosor (2.5–14 px) y alpha (0–1) por punto y dibuja la punta.
- `src/graphics/Renderer.h/.cpp`
  - `drawTrail()` construye una cinta (triangle strip) con dos vértices por punto, normal perpendicular a la dirección de avance.
  - Atributo por vértice: posición + alpha; VBO dinámico reescrito cada frame.
  - Shader propio para la estela y blending `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`.
- `src/main.cpp`
  - `Input` se registra tras inicializar la ventana; `Blade` vive en `runGameLoop`.
  - La espada se actualiza después de las frutas y se dibuja al final (queda por encima).

### Checklist
- [x] Posición del mouse capturada con callbacks de GLFW.
- [x] `Blade` guarda el historial en `std::deque`.
- [x] Estela con grosor real (cinta, no `glLineWidth`).
- [x] Desvanecido por alpha y afinado hacia el extremo antiguo.
- [x] `Vector2D<T>` con operadores `+`, `-`, `=` y compuestos.
- [x] Compila sin advertencias (`-Wall -Wextra -Wpedantic`).
- [x] Verificado visualmente con un arnés temporal (`build/sprint2.png`).

### Decisiones de diseño
- **`Vector2D<T>` propia para la lógica**: cumple el requisito de plantillas y sobrecarga sin abandonar GLM en el renderer, donde `glm::ortho`/`glm::rotate` son más adecuadas.
- **Cinta en vez de `GL_LINE_STRIP`**: el grosor de líneas en core profile depende del driver (a menudo 1 px). Generar una cinta con `GL_TRIANGLE_STRIP` da grosor y alpha reales y es portable.
- **Desvanecido por tiempo, no por índice**: un contador de antigüedad por punto permite que la estela desaparezca sola al detener el mouse; con fade por índice quedaría congelada.
- **Caducidad + distancia mínima + tope de puntos**: controla el coste y evita segmentos degenerados (dos puntos casi iguales rompen la normal).
- **Punta visible**: el cursor del sistema se oculta; un pequeño círculo en la posición actual evita que el jugador "pierda" el puntero cuando está quieto.
- **Shader separado para la estela**: el shader sólido no necesita alpha por vértice; separarlos evita atributos por defecto y mantiene cada programa mínimo.
- **Blade fuera del vector de entidades**: las frutas se añaden/eliminan dinámicamente; la espada es única y debe dibujarse siempre al final para quedar encima.
- **`Input` dueño del user pointer de GLFW**: documentado en `Input.h`; si se añaden más callbacks habrá que centralizarlo en `Window`.

### Verificación
- Prueba de operadores de `Vector2D` (asserts) superada.
- Arnés temporal: recorrido sintético en forma de ocho inyectado en `Input` (Wayland no permite mover el puntero por software), 300 frames a 60 fps; captura `build/sprint2.png` con estela, desvanecido, afinado y punta.
- Juego real ejecutado sin errores con la entrada registrada.

## Sprint 3 – Collision & Cutting
**Goal:** Detect the sword hitting a fruit, change its state and replace it with two
flying halves.

### Tasks
1. Implement segment-circle intersection math (the sword is a series of segments, the
   fruit is a circle).
2. On collision, change the fruit state to "Sliced".
3. Create the `FruitHalf` class; spawn two halves with diverging velocities.
4. Give the halves their own physics (gravity and rotation).

### References
- *The Nature of Code* – Chapter 2.6 (forces and integration).
- Closest-point-on-segment projection (standard 2D collision math).

### Implementación
- `src/math/Collision.h/.cpp`
  - `Collision::segmentIntersectsCircle(start, end, center, radius)`.
  - Proyecta el centro sobre el segmento, recorta el parámetro a `[0, 1]` y compara
    distancias **al cuadrado** (`≤ radius²`), sin raíces.
  - Cubre los tres casos: centro frente al segmento, sobre él o más allá de un extremo;
    también segmentos degenerados (`start == end`).
- `src/entities/Fruit.h/.cpp`
  - Migrada a `Vector2D<float>`: toda la simulación comparte el mismo tipo de vector.
  - Estados en un registro de bits (`unsigned int m_flags`) con el namespace `FruitFlag`:
    `kActive = 1u << 0`, `kSliced = 1u << 1`, `kBomb = 1u << 2`.
  - Operaciones bitwise expuestas: `addFlag` (`|=`), `removeFlag` (`&= ~`),
    `toggleFlag` (`^=`), `hasFlag` (`&`). Una fruta puede ser `kActive | kBomb`.
  - `isAlive()` se sobrescribe para depender de `kActive`: el estado lógico vive en un
    único sitio (las banderas), no en un `bool` paralelo.
  - `slice(cutDirection)` alinea el diámetro con el tajo (`atan2`), separa las mitades
    a lo largo de `direction.perpendicular()`, les da velocidades opuestas y giros
    distintos, y marca la fruta como cortada. Devuelve
    `std::pair<std::unique_ptr<FruitHalf>, std::unique_ptr<FruitHalf>>`.
- `src/entities/FruitHalf.h/.cpp`
  - Semicírculo con `PhysicsBody` propio, gravedad, rotación y culling inferior.
  - `render()` dibuja la cáscara (color original) y una pulpa más clara y concéntrica
    (`drawSemicircle` dos veces).
  - Expone `position()`, `velocity()`, `radius()` y `angle()`.
- `src/graphics/Renderer.h/.cpp`
  - Nueva malla unitaria de semicírculo (0 a π, 40 segmentos) y
    `drawSemicircle(center, radius, angle, color)` con `GL_TRIANGLE_FAN`.
  - Liberación de `m_semiVAO`/`m_semiVBO` en `cleanup()`.
- `src/entities/Blade.h/.cpp`
  - `CutResult { bool hit; Vector2D<float> direction; }`.
  - `cutAt(center, radius)` recorre los segmentos del historial del más reciente al más
    antiguo y devuelve la dirección del primero que intersecta.
- `src/main.cpp`
  - `sliceFruits()` identifica las `Fruit` con `dynamic_cast`, consulta `Blade::cutAt()`
    y recoge las mitades en un vector temporal para no invalidar el iterador.
  - Orden del bucle: `updateObjects` → `blade.update` → `sliceFruits` →
    `removeDeadObjects` → render. La espada se actualiza antes de cortar y las frutas
    cortadas se liberan en el mismo frame.
- `src/physics/Physics2D.h/.cpp` y `src/entities/Spawner.cpp`
  - Migrados a `Vector2D<float>` para no mezclar GLM (render) con la simulación.

### Checklist
- [x] Intersección segmento-círculo implementada y probada.
- [x] Estado de fruta con banderas y operaciones `&`, `|`, `^`, `&= ~`.
- [x] `Fruit::slice()` genera dos `FruitHalf` con velocidades divergentes.
- [x] Mitades con gravedad, rotación y culling.
- [x] `Blade::cutAt()` devuelve la dirección real del segmento que cortó.
- [x] El juego sustituye la fruta cortada por dos mitades.
- [x] Compila sin advertencias (`-Wall -Wextra -Wpedantic`).
- [x] Pruebas de colisión, banderas, corte y física de mitades superadas.
- [x] Verificado visualmente con un arnés temporal (`build/sprint3.png`).

### Decisiones de diseño
- **Vector2D también en la física**: mezclar `glm::vec2` y `Vector2D` en el mismo cálculo
  obligaba a conversiones constantes. GLM queda solo para matrices en el renderer.
- **Estado en bits, no en `bool`s**: el enunciado pide `&`, `|`, `^`; un registro permite
  combinar estados (activa y bomba) y comprobarlos con una sola operación.
- **`slice()` devuelve `unique_ptr`**: la propiedad de las mitades pasa al bucle, que las
  mete en el mismo `vector<unique_ptr<GameObject>>`; no hay `new`/`delete` manuales.
- **`dynamic_cast` solo al cortar**: las frutas necesitan datos que `GameObject` no
  expone (posición, radio, banderas). El coste es bajo con pocas entidades y evita
  añadir más virtuales a la clase base.
- **Mitades separadas en el frame del corte**: se desplazan un poco a lo largo de la
  perpendicular para que el hueco se vea desde el primer frame, además de su velocidad.
- **Giro independiente por mitad**: además de la rotación heredada, cada mitad recibe
  `±kHalfSpinSpeed`; así el corte se lee como dos cuerpos distintos en el aire.
- **Semicírculo con malla propia**: reutilizar el círculo y recortarlo exigiría
  `glScissor` o descartes en el fragment shader; una malla de 40 segmentos es más simple
  y barata.
- **Colisión solo con el historial de la espada**: el tajo es la polilínea reciente, no
  un punto; así un movimiento rápido sigue cortando aunque el mouse salte píxeles.

### Verificación
- Prueba temporal con asserts:
  - Segmento que atraviesa, roza (tangente), se queda corto y segmento degenerado.
  - Banderas: activa, bomba combinada con `|`, alternancia con `^`, baja con `&= ~`.
  - Corte: dos mitades no nulas, fruta marcada como `kSliced`, posiciones y velocidades
    divergentes, segunda mitad girada π; corte repetido y bomba devuelven `nullptr`.
  - Física: la mitad cae y muere fuera de pantalla; el ángulo avanza con el giro.
- Arnés temporal: tajo diagonal inyectado en `Input` (Wayland no permite mover el
  puntero por software); las mitades se congelan tras unos frames para capturar
  `build/sprint3.png`, donde se ven cáscara, pulpa, separación y giro.
- Juego real ejecutado sin errores con el corte integrado en el bucle.

## Sprint 4 – Game Loop, UI & Backgrounds (parte 1)
**Goal:** Dar al juego una máquina de estados visible (menú, partida, pausa y game over),
fondos por escenario con crossfade y texto rasterizado con FreeType.

> **Alcance de esta iteración:** fondos + tipografía + estados + HUD mínimo. Las
> partículas de jugo, las bombas y la optimización con AVX quedan para la parte 2.

### Tasks
1. Máquina de estados `Menu → Playing → Paused → GameOver`.
2. Cargar los cuatro PNG de `src/graphics/assets/` y asociarlos a cada estado.
3. Crossfade al cambiar de escenario, sin depender de la dificultad.
4. Integrar FreeType para dibujar títulos, HUD e instrucciones.
5. HUD de puntos y vidas; perder una vida al dejar caer una fruta.
6. Reinicio de partida y navegación completa por teclado.

### References
- FreeType 2 Documentation (https://freetype.org/).
- LearnOpenGL – Text Rendering (atlas de glifos).
- SIL Open Font License 1.1 (Zen Antique).
- *The Nature of Code* – Cap. 1 (sistemas de coordenadas).

### Implementación
- `src/game/GameState.h`
  - Enum `GameState { kMenu, kPlaying, kPaused, kGameOver }` y diagrama de
    transiciones documentado.
- `src/game/GameSession.h/.cpp`
  - Agrupa `Spawner`, la lista de `GameObject`, `Blade`, `score` y `lives`.
  - `update()` conserva el orden del Sprint 3: spawn → física → espada → corte →
    frutas perdidas → limpieza.
  - `reset()` vacía entidades, restaura 3 vidas, pone el puntaje a cero y limpia la
    estela; se llama al entrar a `Playing` desde el menú, game over o al reiniciar.
  - `countMissedFruits()` detecta frutas activas que murieron por caer (no las
    cortadas, que ya perdieron `kActive`). Las bombas se excluyen.
  - `isGameOver()` se activa al llegar a 0 vidas.
- `src/game/GameUi.h/.cpp`
  - HUD `PUNTOS` / `VIDAS` en las esquinas superiores.
  - Overlays semitransparentes y textos de menú, pausa y game over.
  - Texto centrado con sombra desplazada para mejorar el contraste.
- `src/graphics/TextRenderer.h/.cpp`
  - FreeType rasteriza ASCII imprimible (32–126) a un atlas de 10×10 celdas
    (`GL_RED`, 64 px por glifo).
  - Cada glifo guarda UV, tamaño, rodamiento y avance; el dibujado emite dos
    triángulos por carácter en un VBO dinámico.
  - Shader propio: `coverage = texture(uAtlas, uv).r` modula el alpha del color.
  - `textWidth()`, `lineHeight()` y `ascent()` para centrar y alinear.
- `src/graphics/Background.h/.cpp`
  - Enum `BackgroundScene { kOutside, kRoom, kInside, kNursery }`.
  - Carga `dojo_outside.png`, `dojo_room.png`, `dojo_inside.png` y
    `dojo_nursery.png`.
  - Crossfade de 0.8 s: el fondo anterior se dibuja opaco y el nuevo encima con
    alpha 0→1. `setScene(scene, immediate)` permite saltar el fundido al arrancar.
- `src/graphics/Texture.h` y `src/graphics/Renderer.h/.cpp`
  - `Texture { id, width, height, valid() }` como tipo de valor.
  - `loadTexture()` con `stb_image` (implementación única en `Renderer.cpp`),
    `destroyTexture()` y `drawTexture(center, size, alpha)`.
  - Shader de textura con `uAlpha`; las primitivas sólidas pasan a `vec4` para
    admitir overlays transparentes.
- `src/core/AssetPaths.h/.cpp`
  - Prueba `FRUITNINJA_SOURCE_DIR/src/graphics/assets/` y rutas relativas
    (`src/`, `../src/`, `../../src/`) hasta encontrar el archivo.
- `src/core/Input.h/.cpp`
  - Callback de teclado y `wasKeyPressed()` con flanco de subida por frame;
    `beginFrame()` limpia el registro antes de `pollEvents()`.
- `src/core/Window.h/.cpp`
  - `requestClose()` para que `Esc` cierre la ventana desde el menú.
- `src/entities/Blade.h`
  - `clearTrail()` para no arrastrar puntos entre estados o partidas.
- `src/main.cpp`
  - `transitionFor()` centraliza las teclas de los cuatro estados.
  - Solo `Playing` actualiza la sesión; `Paused` y `GameOver` congelan la escena.
  - `backgroundFor()` mapea estado → escenario: menú/outside, partida/room,
    pausa/inside, game over/nursery.
- `CMakeLists.txt`
  - `find_package(Freetype REQUIRED)` y `Freetype::Freetype`.
  - Include de `external/stb` y define `FRUITNINJA_SOURCE_DIR`.
  - Advertencias `-Wall -Wextra -Wpedantic` en GCC/Clang.
- Recursos
  - `src/graphics/assets/fonts/ZenAntique-Regular.ttf` + `OFL.txt` (SIL OFL 1.1).
  - `external/stb/stb_image.h` (header-only, dominio público/MIT).

### Checklist
- [x] Cuatro fondos cargados y asociados a los estados.
- [x] Crossfade de 0.8 s entre escenarios.
- [x] FreeType integrado y tipografía Zen Antique con licencia OFL.
- [x] Máquina de estados completa con teclado.
- [x] HUD de puntos y vidas; game over alcanzable con 3 vidas.
- [x] Reinicio de partida sin reiniciar el proceso.
- [x] Renderer con texturas, alpha y liberación de recursos.
- [x] Compila sin advertencias (`-Wall -Wextra -Wpedantic`).
- [x] Verificado visualmente con capturas de los cuatro estados y del crossfade.

### Decisiones de diseño
- **Fondo por estado, no por dificultad**: los escenarios son cosméticos; la
  progresión no altera el spawner ni la física.
- **Crossfade directo**: dibujar el fondo viejo opaco y el nuevo con alpha creciente
  evita parpadeos y no necesita una tercera textura.
- **Atlas de celdas fijas**: una cuadrícula 10×10 para 95 glifos es más simple que
  un empaquetador general y sobra para ASCII. El margen de 4 px evita sangrado.
- **Un shader propio para texto**: el shader de texturas del Renderer aplica alpha
  uniforme; el texto necesita muestrear cobertura (`GL_RED`) y tintar con color.
- **Texto en píxeles de pantalla**: reutiliza la misma proyección que el resto del
  juego y evita mezclar unidades.
- **`stb_image` header-only**: evita añadir libpng/libjpeg y mantiene la compilación
  portable; la implementación se compila una sola vez.
- **`AssetPaths` con ruta de CMake**: el ejecutable puede lanzarse desde `build/` o
  `build/bin/` sin depender del directorio de trabajo.
- **Game over por frutas perdidas**: da un objetivo claro al HUD sin necesitar
  bombas; las bombas ya se respetan en el código para el siguiente paso.

### Verificación
- Compilación limpia con `-Wall -Wextra -Wpedantic` sin advertencias.
- Arnés temporal de captura con un FBO de 1920×1080 (no versionado):
  - `build/sprint4_menu.png`: menú sobre `outside`.
  - `build/sprint4_playing.png`: partida sobre `room` con frutas y HUD.
  - `build/sprint4_fade.png`: crossfade a mitad de camino `room → inside`.
  - `build/sprint4_paused.png`: pausa sobre `inside`.
  - `build/sprint4_gameover.png`: game over sobre `nursery`.
- Juego real ejecutado sin errores de carga de texturas ni de fuente.

### Pendiente (parte 2)
- Partículas de jugo al cortar.
- Bombas en el spawner y penalización al cortarlas.
- Optimización con AVX del cálculo de colisiones o física.
- Sonido y pulido final (Sprint 5).

## Sprint 5 – Polish & Juice
> (To be filled after completion)

---

## Bibliography
- *The Nature of Code* – Daniel Shiffman.
- OpenGL 4.6 Documentation.
- GLFW, GLAD, GLM official docs.

---

*This file will be expanded incrementally as we implement each sprint.*
