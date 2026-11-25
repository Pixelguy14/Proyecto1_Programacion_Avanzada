/Proyecto1_Programacion_Avanzada
│── /assets          (Texturas, fuentes, sonidos - si usas)
│── /src
│   ├── main.cpp     (Punto de entrada y Game Loop)
│   ├── /core
│   │   ├── Game.h/cpp       (Maneja el estado: Menú, Jugando, GameOver)
│   │   ├── Window.h/cpp     (Configuración de GLFW/SDL y contexto OpenGL)
│   │   └── Input.h/cpp      (Manejo de eventos de mouse/teclado)
│   ├── /graphics
│   │   ├── Shader.h/cpp     (Compilación de shaders GLSL)
│   │   ├── Renderer.h/cpp   (Dibujado de polígonos, líneas y texturas)
│   │   └── Texture.h/cpp    (Carga de imágenes)
│   ├── /physics
│   │   └── Physics2D.h/cpp  (Integración de Euler, gravedad)
│   ├── /entities
│   │   ├── Fruit.h/cpp      (Datos de la fruta, estado de corte)
│   │   ├── FruitHalf.h/cpp  (Las mitades al cortar)
│   │   ├── Blade.h/cpp      (Trackeo de la trayectoria del mouse)
│   │   └── Particle.h/cpp   (Efecto de jugo)
│   └── /math
│       └── Collision.h/cpp  (Intersección línea-círculo)


Sprints (Metodología Ágil) del proyecto en C++ OpenGL

Vamos a dividir el proyecto en 6 Sprints (de aproximadamente 1-2 semanas cada uno dependiendo de tu tiempo). El objetivo es tener siempre algo jugable o visible al final de cada sprint.
Sprint 0: Cimientos y OpenGL (El "Hola Mundo")

    Objetivo: Configurar el entorno, crear la ventana y renderizar un triángulo.

    Tareas:

        Configurar CMake y las librerías (GLFW, GLAD, GLM).

        Crear la clase Window y el Game Loop básico con Delta Time (dt).

        Escribir un Shader básico y renderizar un rectángulo de color en pantalla.

        Implementar la matriz de proyección ortográfica (para que las coordenadas de pantalla sean fáciles de manejar, ej. 0 a 1920 en X).

    Entregable: Ventana abierta con un rectángulo estático. (Sprint 0 terminado)

Sprint 1: Física y Movimiento (Cap. 2.6 de Nature of Code)

    Objetivo: Implementar la gravedad y el movimiento parabólico.

    Tareas:

        Crear la entidad Fruit (posición, velocidad, aceleración, radio).

        Implementar la integración de Euler semi-implícita en un gestor de física: velocity += gravity * dt; position += velocity * dt.

        Implementar un "Spawner" que lance una fruta desde abajo de la pantalla hacia arriba con un vector de velocidad aleatorio.

        Rotación visual de la fruta (velocidad angular).

    Entregable: Frutas (rectángulos o círculos) saliendo disparadas desde el suelo y cayendo por gravedad.

Sprint 2: La Espada y el Blade Trail

    Objetivo: Capturar el mouse y dibujar la estela visual.

    Tareas:

        Capturar la posición del mouse usando los callbacks de GLFW.

        Crear la clase Blade que guarde un array (o std::deque) con las últimas 10-15 posiciones del mouse.

        Renderizar esa lista de puntos como una tira de líneas (GL_LINE_STRIP) con un grosor considerable.

        Añadir un efecto visual: que la estela se vaya desvaneciendo (alpha) o afinando hacia el final.

    Entregable: Puedes "mover" una espada en pantalla y deja una estela visual.

Sprint 3: Detección de Colisiones y Corte

    Objetivo: Cortar frutas y generar mitades.

    Tareas:

        Implementar la matemática de intersección segmento-círculo (La espada es una serie de segmentos, la fruta es un círculo).

        Al detectar colisión, cambiar el estado de la fruta a "Cortada".

        Crear la clase FruitHalf. Al cortar una fruta, instanciar dos mitades con velocidades divergentes (simulando la fuerza del tajo).

        Las mitades también deben tener física (gravedad y rotación).

    Entregable: Al pasar el mouse por encima de una fruta, esta desaparece y salen dos mitades volando.

Sprint 4: Game Loop, UI y Partículas

    Objetivo: Hacer que sea un juego real.

    Tareas:

        Implementar el sistema de partículas (cubitos de "jugo" saliendo al cortar una fruta).

        Añadir lógica de juego: Puntos por fruta cortada, vidas (se pierde si tres frutas cae al suelo sin cortar).

        Implementar una máquina de estados simple (Menú -> Jugando -> Game Over) y pausa

        (Opcional) Renderizar texto básico usando una librería como stb_truetype para mostrar el puntaje.

    Entregable: Un juego completamente jugable con puntaje y condiciones de derrota.

Sprint 5: Pulido y "Juice" (Game Feel)

    Objetivo: Hacer que se sienta bien y se vea bonito.

    Tareas:

        Efectos de sonido (corte, bomba). https://freesound.org/

        Screen shake (temblor de pantalla al cortar o explotar una bomba).

        Ajuste fino del tamaño del radio de corte y la velocidad de las frutas.

        Frutas siendo figuras 3D que giren y de esta manera representar el nucleo y la cascara.
        Figuras 3D que representen melones, manzanas, naranjas, limones, platanos, vértices e índices (VBO / EBO), puedes partir de primitivas básicas con pocas subdivisiones (por ejemplo, sectors = 8 a 12 y stacks = 6 a 8) y normales planas (flat shading) por 

    Entregable: El juego listo para presumir.



Ajuste de Sprints: Fruit Ninja (Enfoque en OOP, STL y SIMD)
- Sprint 0: Cimientos, OpenGL y Arquitectura OOPConfigurar dependencias (GLFW, GLAD) y el Game Loop básico.Integración de clase: Define una clase base abstracta GameObject o Entidad que contenga atributos protegidos y métodos virtuales puros como virtual void update(float dt) = 0; y virtual void render() = 0;.   Implementar el Shader básico y renderizar primitivas.
- Sprint 1: Física, Entidades y Gestión con STLCrear la clase Fruit heredando de GameObject e implementar la integración de Euler semi-implícita.Integración de clase: En lugar de arreglos estáticos de C, utiliza un std::vector<GameObject*> de la Standard Template Library (STL) para gestionar tu población de frutas dinámicamente.   Asegurar que la reserva de memoria para objetos iterables se libere correctamente (delete) o usar punteros inteligentes para evitar Segmentation Faults.   
- Sprint 2: La Espada y Sobrecarga de OperadoresCapturar el mouse y guardar el historial de posiciones.Integración de clase: Implementa una clase Vector2D mediante el uso de plantillas (template <class T>) para manejar coordenadas.   Aplica sobrecarga de operadores (+, -, =) en tu clase de vectores para calcular las distancias y deltas de movimiento de la espada sin depender exclusivamente de GLM.   
- Sprint 3: Colisiones y Máscaras a Nivel de BitsMatemática de intersección segmento-círculo para el corte y generación de instancias de FruitHalf.Integración de clase: Implementa el sistema de estados de la fruta (ej. activa, cortada, bomba) utilizando operaciones a nivel de bits (Bitwise &, |, ^) sobre un registro de estado o banderas para determinar de forma eficiente si interactúan con la espada.   
- Sprint 4: Sistema de Partículas y Paralelismo de Datos (AVX)Añadir la lógica central (vidas, puntaje).Integración de clase: Este es el núcleo de bajo nivel. Implementa el sistema de partículas del "jugo" alineando la memoria de sus posiciones a 32 bytes con _mm_malloc.   Utiliza Intrinsics y registros AVX (__m256, _mm256_add_pd o flotantes) para actualizar la posición y gravedad de 8 partículas simultáneamente por cada ciclo de reloj, aplicando el paralelismo de datos (SIMD) visto en clase.   
- Sprint 5: Pulido y Gestor de Recursos con Hash (STL), screen shake y ajustar el Game Feel.Integración de clase: Implementa un gestor de recursos (Asset Manager) apoyándote en los contenedores de la STL, utilizando <map> o un enfoque similar para crear una tabla Hash que asocie identificadores de texto (strings) con las texturas/sonidos cargados, evitando programar la tabla con colisiones desde cero.

Para cada una de las tareas necesarias, se deberá de esperar al terminar cada tarea el commit hecho por el usuario



Fuentes:
https://medium.com/@msayef/opengl-mouse-click-detection-on-3d-objects-715d489c784c
https://simplifycpp.org/articles/a0562/opengl-in-cpp-the-guide-for-3d-graphics-programming/
https://community.khronos.org/t/slicing-3d-model/53609
https://github.com/ifilot/opengl-cpp-course/tree/master/lesson01
https://community.khronos.org/t/3d-object-slicing-algorithm/63035/2
https://github.com/ifilot/opengl-cpp-course/blob/master/lesson04/README.md
https://www3.ntu.edu.sg/home/ehchua/programming/opengl/CG_Examples.html
https://learnopengl.com/In-Practice/2D-Game/Audio
https://www.opengl.org/archives/resources/code/samples/advanced/advanced97/notes/node181.html
https://freetype.org/

Cap. 2.6 de Nature of Code (*The Nature of Code* – Daniel Shiffman.)
Cap. 1 (Vectores y sistemas de coordenadas)


Todos los archivos de src/ tienen Doxygen: @file, @brief, @param, @return, y cada helper privado está documentado.