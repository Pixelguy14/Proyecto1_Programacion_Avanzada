# Proyecto 1 — Programación Avanzada

Clon de **Fruit Ninja** en C++17 con OpenGL 3.3 core, basado en *The Nature of Code*
(cap. 2.6) y el trackeo del mouse.

Estado actual: **Sprint 4 terminado** — máquina de estados, cuatro fondos con
crossfade, tipografía FreeType (Zen Antique), HUD de puntos/vidas, pantallas de menú,
pausa y game over, partículas de jugo, bombas y detección de colisiones con AVX
(con respaldo escalar). Ver [`process.md`](process.md) para el detalle.

---

## 1. Requisitos

### Comunes a todos los sistemas
| Herramienta | Versión mínima | Para qué |
|---|---|---|
| Compilador C++17 | GCC 9+, Clang 10+ o MSVC 2019+ | Compilar el juego |
| CMake | **3.16** | Sistema de compilación (GLFW lo exige) |
| Git | cualquiera reciente | Descargar los submódulos |
| Python | 3.8+ | GLAD genera el loader de OpenGL al compilar |
| **Jinja2** (paquete de Python) | 2.7 – 3.x | Dependencia de GLAD (`ModuleNotFoundError` si falta) |
| **FreeType** | 2.8+ | Rasterizar la tipografía del HUD (Sprint 4) |
| GPU y driver | **OpenGL 3.3 core** | Requisito de los shaders `#version 330` |

> GLAD2 **no** es una librería precompilada: se ejecuta como generador de Python
> durante la configuración/compilación. Por eso Python y Jinja2 son obligatorios.

> **FreeType sí se instala con el gestor de paquetes** (o se obtiene con Homebrew /
> vcpkg en macOS y Windows). CMake lo busca con `find_package(Freetype REQUIRED)`.
> `stb_image` está incluido en `external/stb/` y no requiere instalación.

> **GLFW, GLAD y GLM no se instalan con el gestor de paquetes.** Están versionados como
> submódulos de Git en `external/`, así que todos los que compilen usan exactamente las
> mismas versiones. No instales las versiones del sistema: podrían entrar en conflicto
> con las cabeceras del proyecto. Solo se necesita conexión a Internet para el primer
> clonado de los submódulos; compilar después funciona sin red.

### Linux (Debian / Ubuntu / Mint)
```bash
sudo apt update
sudo apt install build-essential cmake git pkg-config python3 python3-venv \
                 libgl1-mesa-dev libwayland-dev libxkbcommon-dev xorg-dev \
                 libfreetype-dev
# opcional, para comprobar la versión de OpenGL:
sudo apt install mesa-utils
```

### Linux (Fedora / RHEL)
```bash
sudo dnf install gcc-c++ make cmake git pkgconf-pkg-config python3 python3-pip \
                 mesa-libGL-devel wayland-devel libxkbcommon-devel \
                 libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel \
                 freetype-devel
```

### Linux (Arch / Manjaro)
```bash
sudo pacman -S --needed base-devel cmake git pkgconf python \
                       mesa wayland libxkbcommon \
                       libxcursor libxi libxinerama libxrandr freetype2
```

### macOS
```bash
xcode-select --install   # compilador (Clang) y herramientas
brew install cmake python freetype
```
No hay que instalar OpenGL: macOS incluye `OpenGL.framework`. Apple lo marcó como
obsoleto, pero sigue disponible el perfil 3.3 core que usa el proyecto.
Funciona tanto en Intel como en Apple Silicon.

### Windows
1. **Visual Studio 2022** con la carga de trabajo *Desarrollo para escritorio con C++*
   (incluye MSVC y el Windows SDK). Alternativa: **MinGW-w64**.
2. **CMake** (https://cmake.org/download/ o `winget install Kitware.CMake`).
3. **Python 3** (https://www.python.org/downloads/ o `winget install Python.Python.3.12`)
   marcando *Add python.exe to PATH*.
4. OpenGL `opengl32` viene con el Windows SDK.
5. **FreeType** para el texto del HUD: instálalo con **vcpkg**
   (`vcpkg install freetype`) y configura CMake con
   `-DCMAKE_TOOLCHAIN_FILE=<ruta>/vcpkg.cmake`, o descarga un build precompilado
   y define `CMAKE_PREFIX_PATH`.

---

## 2. Dependencias de Python (Jinja2)

Se recomienda un **entorno virtual** para no depender del Python del sistema
(Ubuntu/Debian y Homebrew bloquean `pip install` global por PEP 668):

```bash
# Linux / macOS
python3 -m venv .venv
source .venv/bin/activate
# Windows (PowerShell)
py -3 -m venv .venv
.venv\Scripts\Activate.ps1

python -m pip install --upgrade pip
python -m pip install "jinja2>=2.7,<4"
```

Configura y compila **con el entorno virtual activo**, para que CMake encuentre ese
intérprete de Python.

Alternativa sin venv (instala Jinja2 con el gestor del sistema):
```bash
sudo apt install python3-jinja2     # Debian/Ubuntu
sudo dnf install python3-jinja2     # Fedora
sudo pacman -S python-jinja2        # Arch
```

---

## 3. Obtener el código

```bash
git clone --recurse-submodules <URL-del-repositorio>
cd Proyecto1_Programacion_Avanzada
```

Si ya habías clonado el repositorio sin los submódulos:
```bash
git submodule update --init --recursive
```

Los submódulos son GLFW, GLAD y GLM (ver `.gitmodules`); sin ellos CMake no puede
configurar el proyecto.

---

## 4. Compilación

El proyecto usa siempre un directorio aparte (`build/`).

```bash
cmake -S . -B build
cmake --build build --parallel
```

Estos dos comandos son portables (Linux, macOS y Windows, cualquier generador).
Variantes útiles:

```bash
# Build de depuración con símbolos
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# Reconfigurar desde cero si algo quedó en mal estado
rm -rf build            # Windows: rmdir /s /q build
```

La ruta **AVX** de las colisiones se activa por defecto (`FRUITNINJA_ENABLE_AVX=ON`).
Se compila por función con `target("avx")` y se elige en tiempo de ejecución, así que
el binario sigue arrancando en CPUs sin AVX mediante la ruta escalar. Para forzar la
ruta escalar:

```bash
cmake -S . -B build -DFRUITNINJA_ENABLE_AVX=OFF
```

**Windows con Visual Studio** (generador multi-configuración):
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```

**Windows con MinGW-w64**:
```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build --parallel
```

---

## 5. Ejecución

| Sistema | Comando |
|---|---|
| Linux / macOS | `./build/bin/FruitNinja` |
| Windows (MSVC) | `.\build\bin\Release\FruitNinja.exe` |
| Windows (MinGW / Ninja) | `.\build\bin\FruitNinja.exe` |

Al ejecutarse debe abrirse una ventana de **1920×1080** titulada *Fruit Ninja* con el
menú principal sobre el fondo del dojo exterior. La partida usa el fondo de la
habitación, la pausa el interior del dojo y el game over la guardería; cada cambio de
estado hace un **crossfade** de 0.8 s.

**Controles (Sprint 4):**

| Tecla | Menú | Jugando | Pausa | Game Over |
|---|---|---|---|---|
| `Enter` / `Espacio` | Jugar | — | — | Jugar de nuevo |
| `Esc` | Salir | Pausar | Continuar | Volver al menú |
| `P` | — | Pausar | Continuar | Volver al menú |
| `R` | — | — | Reiniciar | Jugar de nuevo |
| `M` | — | — | Volver al menú | — |

Mueve el mouse por la ventana para dibujar la estela de la espada. El cursor del
sistema se oculta y la estela se desvanece al detener el mouse. Si el recorrido
atraviesa una fruta, esta se parte en **dos mitades** con física propia y suelta una
**ráfaga de jugo** del color de la fruta. Cada fruta cortada suma un punto; cada fruta
que cae sin cortar resta una vida. Con **3 vidas** iniciales, al llegar a cero aparece
la pantalla de game over.

Las **bombas** aparecen después de unos lanzamientos: tienen cuerpo oscuro, borde rojo
y mecha encendida. Cortar una termina la partida de inmediato con una explosión de
chispas; dejarla caer no penaliza. Al arrancar, la consola indica si las colisiones
usan la ruta `AVX` o la `escalar`.

**Recursos incluidos:** los cuatro PNG de `src/graphics/assets/` y la tipografía
`fonts/ZenAntique-Regular.ttf` (SIL Open Font License 1.1, ver
`src/graphics/assets/fonts/OFL.txt`). `AssetPaths` los encuentra aunque el ejecutable
se lance desde `build/` o `build/bin/`.

---

## 6. Problemas comunes

| Síntoma | Causa y solución |
|---|---|
| `ModuleNotFoundError: No module named 'jinja2'` | Jinja2 no está en el Python que usa CMake. Activa el venv, instálalo y reconfigura: `rm -rf build && cmake -S . -B build`. |
| CMake encuentra otro Python (p. ej. el de Visual Studio) | Fuerza el intérprete: `cmake -S . -B build -DPython_EXECUTABLE=/ruta/a/python` (Windows: `-DPython_EXECUTABLE=C:\Python312\python.exe`). |
| `CMake 3.16 or higher is required` | Actualiza CMake; GLFW 3.5 no acepta versiones anteriores. |
| `[ERROR] In-source builds no están permitidos` | Estás ejecutando `cmake .` en la raíz. Usa `cmake -S . -B build`. |
| `Failed to create GLFW window` en Linux | No hay servidor gráfico o driver OpenGL: comprueba `echo $DISPLAY` / `$WAYLAND_DISPLAY` y que `glxinfo \| grep "OpenGL version"` muestre ≥ 3.3. |
| `Failed to initialize GLAD` | El contexto OpenGL no se creó correctamente (driver/entorno sin aceleración). |
| Solo funciona en local, falla por SSH | Necesita una sesión gráfica real o *X11 forwarding* con GLX; no funciona en terminal sin display. |
| El ejecutable no aparece donde se indica | Usa `cmake --build build --verbose` o busca con `find build -name FruitNinja*` (en Windows, `dir /s build\FruitNinja.exe`). |
| `GLFW/glfw3.h: No such file or directory` | Faltan los submódulos: `git submodule update --init --recursive`. |
| `Could not find a package configuration file ... Freetype` | Instala FreeType (`libfreetype-dev`, `freetype-devel`, `freetype2` o `brew install freetype`) y vuelve a configurar CMake. |
| `[TextRenderer] No se pudo cargar la fuente` | Falta `src/graphics/assets/fonts/ZenAntique-Regular.ttf`; comprueba que el directorio `assets/` esté completo. |
| Se cambió de Python/venv y el build falla | `rm -rf build` y vuelve a configurar con el entorno correcto. |

---

## 7. Documentación

- [`documentacion1.md`](documentacion1.md): alcance, sprints y entregables.
- [`process.md`](process.md): bitácora, decisiones de diseño y convenciones.
- El código de `src/` está documentado con comentarios **Doxygen**.
- licencia: [MIT](LICENSE)

---
