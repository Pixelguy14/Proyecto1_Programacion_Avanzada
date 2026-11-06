# Proyecto 1 - Programación Avanzada - Clon de Fruit Ninja usando C++ OpenGL

## Prerrequisitos
- Compilador C++ con soporte **C++17** (p.ej., `g++` ≥ 7 o `clang`).
- **CMake** ≥ 3.15.
- **Git** (para descargar los sub‑módulos).

## Configuración del repositorio
```bash
# Clonar el repositorio
git clone <url-del-repo> FruitNinja
cd FruitNinja

# Inicializar los sub‑módulos (GLFW, GLAD, GLM)
git submodule update --init --recursive
```

## Compilación
```bash
# Crear el directorio de compilación (out-of-source build)
mkdir -p build
cd build

# Configurar el proyecto desde dentro de build/
cmake ..

# Compilar
make -j$(nproc)
```
El binario resultante se encontrará en `build/bin/FruitNinja`.

> **Nota:** CMake **rechazará** correr desde la raíz del proyecto (`cmake .`).
> Siempre usa el directorio `build/` para mantener limpia la raíz.

## Ejecución
```bash
./bin/FruitNinja
```
Se abrirá una ventana de **1920 × 1080** con un fondo gris oscuro y un rectángulo rojo centrado.
