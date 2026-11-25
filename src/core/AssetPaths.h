#pragma once

#include <string>

/**
 * @file AssetPaths.h
 * @brief Localización de los recursos (imágenes, fuentes y sonidos).
 *
 * El ejecutable puede lanzarse desde el directorio del proyecto, desde
 * `build/` o desde `build/bin/`. En lugar de asumir un único directorio de
 * trabajo, `AssetPaths::resolve()` prueba varias raíces candidatas y devuelve
 * la primera que contiene el archivo.
 *
 * CMake define `FRUITNINJA_SOURCE_DIR`, así que la ruta absoluta del código
 * fuente siempre es la primera candidata; las rutas relativas sirven de
 * respaldo cuando el proyecto se mueve o se compila de otra forma.
 */
namespace AssetPaths {

/**
 * @brief Resuelve la ruta de un recurso dentro de `src/graphics/assets/`.
 * @param relativePath Ruta relativa al directorio de assets, por ejemplo
 *                     `"dojo_room.png"` o `"fonts/ZenAntique-Regular.ttf"`.
 * @return Ruta existente si se encontró alguna candidata; en caso contrario,
 *         la primera ruta probada (el fallo se reporta al cargar el archivo).
 */
std::string resolve(const std::string& relativePath);

}  // namespace AssetPaths
