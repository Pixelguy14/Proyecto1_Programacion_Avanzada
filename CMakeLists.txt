# GLFW 3.5 exige CMake >= 3.16, aunque el resto del proyecto funcione con menos.
cmake_minimum_required(VERSION 3.16)

# ------------------------------------------------------------
# Guardia contra in-source builds
# Obliga a usar:  mkdir build && cd build && cmake ..
# ------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "\n[ERROR] In-source builds no están permitidos.\n"
        "Usa un directorio separado, por ejemplo:\n"
        "  cmake -S . -B build && cmake --build build --parallel\n"
    )
endif()

project(FruitNinja LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------
# Dependencias externas (compiladas desde sub-módulos)
# ------------------------------------------------------------

# GLFW – manejo de ventana y contexto OpenGL
add_subdirectory(external/glfw)

# GLAD2 – loader de funciones OpenGL
#   GladConfig.cmake expone glad_add_library() en lugar de un target fijo.
#   Hay que incluir el cmake/ de glad e invocar la función para generar
#   un target con el perfil deseado (OpenGL 3.3 core).
add_subdirectory(external/glad/cmake)
glad_add_library(glad_gl_core_33 STATIC REPRODUCIBLE API gl:core=3.3)

# GLM – matemáticas 3D (header-only)
add_subdirectory(external/glm)

# OpenGL del sistema (solo el enlazador)
find_package(OpenGL REQUIRED)

# FreeType – rasterizado de la tipografía del HUD (Sprint 4)
find_package(Freetype REQUIRED)

# Hilos y carga dinámica – miniaudio los usa en Linux/BSD (Sprint 5)
find_package(Threads REQUIRED)

# ------------------------------------------------------------
# Fuentes del proyecto
# ------------------------------------------------------------
# CONFIGURE_DEPENDS hace que CMake vuelva a buscar fuentes nuevas al compilar,
# evitando reconfigurar a mano cada vez que se añade un .cpp en un sprint.
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

# ------------------------------------------------------------
# Cabeceras
# ------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external/stb
    ${PROJECT_SOURCE_DIR}/external/miniaudio
)

# Ruta absoluta del código fuente: AssetPaths la usa para encontrar los PNG y
# la fuente aunque el ejecutable se lance desde build/ o build/bin/.
target_compile_definitions(${PROJECT_NAME} PRIVATE
    FRUITNINJA_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

# Advertencias estrictas, coherentes con la verificación de cada sprint.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ------------------------------------------------------------
# Optimización opcional con AVX
# ------------------------------------------------------------
# La ruta AVX se compila por función con target("avx") y se elige en tiempo de
# ejecución; el binario sigue arrancando en CPUs sin AVX (ruta escalar).
option(FRUITNINJA_ENABLE_AVX "Habilitar la ruta AVX de colisiones" ON)
if(FRUITNINJA_ENABLE_AVX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FRUITNINJA_ENABLE_AVX=1)
endif()

# ------------------------------------------------------------
# Enlazado
# ------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    glfw
    glad_gl_core_33   # generado por glad_add_library() arriba
    glm::glm
    OpenGL::GL
    Freetype::Freetype
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# miniaudio solo pide -lm en Linux/BSD; en macOS y Windows enlaza sus APIs en
# tiempo de ejecución, así que no hacen falta frameworks extra.
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

# ------------------------------------------------------------
# Directorio de salida
# ------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)
