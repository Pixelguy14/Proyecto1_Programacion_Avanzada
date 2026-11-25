#include "AssetPaths.h"

#include <fstream>
#include <vector>

/**
 * @file AssetPaths.cpp
 * @brief Búsqueda de recursos en varias raíces candidatas.
 */

namespace {

/// Subdirectorio de assets relativo a la raíz del proyecto.
constexpr const char* kAssetsSubdir = "/src/graphics/assets/";

/**
 * @brief Comprueba si un archivo se puede abrir en modo lectura.
 * @param path Ruta a comprobar.
 * @return true si el archivo existe y es legible.
 */
bool fileExists(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return file.good();
}

/**
 * @brief Construye la lista ordenada de raíces donde buscar assets.
 * @return Rutas base terminadas en `/`.
 *
 * La primera candidata es la ruta absoluta inyectada por CMake; después se
 * prueban rutas relativas habituales para ejecutar desde `build/` o `build/bin/`.
 */
std::vector<std::string> candidateRoots() {
    std::vector<std::string> roots;

#ifdef FRUITNINJA_SOURCE_DIR
    roots.emplace_back(std::string(FRUITNINJA_SOURCE_DIR) + kAssetsSubdir);
#endif

    roots.emplace_back(std::string("src/graphics/assets/"));
    roots.emplace_back(std::string("../src/graphics/assets/"));
    roots.emplace_back(std::string("../../src/graphics/assets/"));
    return roots;
}

}  // namespace

std::string AssetPaths::resolve(const std::string& relativePath) {
    const std::vector<std::string> roots = candidateRoots();
    if (roots.empty()) {
        return relativePath;
    }

    for (const std::string& root : roots) {
        const std::string candidate = root + relativePath;
        if (fileExists(candidate)) {
            return candidate;
        }
    }

    // No se encontró: devolver la primera candidata para que el error de carga
    // muestre una ruta concreta y útil.
    return roots.front() + relativePath;
}
