#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

/**
 * @file HighScoreTable.h
 * @brief Tabla persistente del top 10 de puntajes (extensión final).
 *
 * La tabla guarda como máximo diez entradas ordenadas de mayor a menor. Se
 * serializa en un archivo de texto plano (`puntuacion<TAB>nombre`) dentro del
 * directorio de datos del usuario:
 *
 * - Linux: `$XDG_DATA_HOME/fruitninja/highscores.txt` o
 *   `~/.local/share/fruitninja/highscores.txt`.
 * - macOS: `~/Library/Application Support/FruitNinja/highscores.txt`.
 * - Windows: `%APPDATA%\FruitNinja\highscores.txt`.
 *
 * La variable de entorno `FRUITNINJA_HIGHSCORE_PATH` permite usar otra ruta
 * (útil para pruebas o para una instalación portable).
 */
class HighScoreTable {
public:
    /// Número máximo de entradas conservadas.
    static constexpr std::size_t kMaxEntries = 10;

    /// Longitud máxima del nombre mostrado.
    static constexpr std::size_t kMaxNameLength = 10;

    /// Entrada individual del top 10.
    struct Entry {
        int score = 0;       ///< Puntaje.
        std::string name;    ///< Nombre en mayúsculas (máx. 10 caracteres).
    };

    /**
     * @brief Crea una tabla ligada a una ruta.
     * @param path Archivo de récords; si está vacío se usa la ruta del usuario.
     */
    explicit HighScoreTable(std::filesystem::path path = {});

    /**
     * @brief Carga el archivo y reconstruye el top 10.
     * @return true si el archivo existía y se pudo leer; false en el primer
     *         arranque o si hubo un error.
     *
     * Las líneas mal formadas se ignoran. Tras cargar, la lista se ordena y se
     * recorta a diez entradas.
     */
    bool load();

    /**
     * @brief Guarda el top 10 en disco.
     * @return true si el archivo se escribió correctamente.
     *
     * Crea los directorios intermedios si hace falta. Un fallo de escritura se
     * reporta por `std::cerr` y no interrumpe la partida.
     */
    bool save() const;

    /**
     * @brief Comprueba si un puntaje entraría al top 10.
     * @param score Puntaje a comprobar.
     * @return true si hay hueco o si el puntaje supera al último clasificado.
     */
    bool qualifies(int score) const;

    /**
     * @brief Inserta un puntaje y devuelve su posición.
     * @param score Puntaje a insertar (debe ser mayor que cero).
     * @param name  Nombre tecleado; se sanea y se recorta a 10 caracteres.
     * @return Índice 0–9 de la entrada insertada, o -1 si no entró al top 10.
     */
    int add(int score, const std::string& name);

    /// @return Entradas ordenadas de mayor a menor puntaje.
    const std::vector<Entry>& entries() const { return m_entries; }

    /// @return Ruta del archivo de récords.
    const std::filesystem::path& path() const { return m_path; }

    /**
     * @brief Sanea un nombre para mostrarlo y guardarlo.
     * @param name Texto tecleado.
     * @return Nombre en mayúsculas, solo letras/dígitos/espacios, sin espacios
     *         en los extremos y con un máximo de 10 caracteres. Si queda vacío,
     *         devuelve `"JUGADOR"`.
     */
    static std::string sanitizeName(const std::string& name);

private:
    std::filesystem::path m_path;      ///< Archivo de persistencia.
    std::vector<Entry> m_entries;      ///< Top 10 en memoria.
};
