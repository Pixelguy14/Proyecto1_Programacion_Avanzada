cmake_minimum_required(VERSION 3.15)

# ------------------------------------------------------------
# Guardia contra in-source builds
# Obliga a usar:  mkdir build && cd build && cmake ..
# ------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "\n[ERROR] In-source builds no están permitidos.\n"
        "Por favor crea un directorio separado:\n"
        "  mkdir build && cd build && cmake ..\n"
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

# ------------------------------------------------------------
# Fuentes del proyecto
# ------------------------------------------------------------
file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
    "src/*.c"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

# ------------------------------------------------------------
# Cabeceras
# ------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

# ------------------------------------------------------------
# Enlazado
# ------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    glfw
    glad_gl_core_33   # generado por glad_add_library() arriba
    glm::glm
    OpenGL::GL
)

# ------------------------------------------------------------
# Directorio de salida
# ------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)
