#include "HighScoreTable.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <system_error>
#include <utility>

/**
 * @file HighScoreTable.cpp
 * @brief Carga, guardado y ordenación de los récords del juego.
 */

namespace {

/// Nombre usado cuando el jugador no escribe nada.
constexpr const char* kDefaultName = "JUGADOR";

/**
 * @brief Calcula la ruta del archivo de récords del usuario.
 * @return Ruta específica de la plataforma, o una ruta local de respaldo.
 */
std::filesystem::path defaultHighScorePath() {
    if (const char* overridePath = std::getenv("FRUITNINJA_HIGHSCORE_PATH");
        overridePath != nullptr && *overridePath != '\0') {
        return std::filesystem::path(overridePath);
    }

#if defined(_WIN32)
    if (const char* appData = std::getenv("APPDATA");
        appData != nullptr && *appData != '\0') {
        return std::filesystem::path(appData) / "FruitNinja" / "highscores.txt";
    }
#elif defined(__APPLE__)
    if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0') {
        return std::filesystem::path(home) / "Library" / "Application Support"
             / "FruitNinja" / "highscores.txt";
    }
#else
    if (const char* xdgData = std::getenv("XDG_DATA_HOME");
        xdgData != nullptr && *xdgData != '\0') {
        return std::filesystem::path(xdgData) / "fruitninja" / "highscores.txt";
    }
    if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0') {
        return std::filesystem::path(home) / ".local" / "share" / "fruitninja"
             / "highscores.txt";
    }
#endif

    return std::filesystem::current_path() / "highscores.txt";
}

/**
 * @brief Ordena las entradas de mayor a menor, conservando el orden en empates.
 * @param entries Lista a ordenar (se modifica in-place).
 */
void sortEntries(std::vector<HighScoreTable::Entry>& entries) {
    std::stable_sort(entries.begin(), entries.end(),
                     [](const HighScoreTable::Entry& a,
                        const HighScoreTable::Entry& b) {
                         return a.score > b.score;
                     });
}

}  // namespace

HighScoreTable::HighScoreTable(std::filesystem::path path)
    : m_path(path.empty() ? defaultHighScorePath() : std::move(path)) {}

bool HighScoreTable::load() {
    m_entries.clear();

    std::ifstream file(m_path);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        const std::size_t tab = line.find('\t');
        if (tab == std::string::npos || tab + 1 >= line.size()) {
            continue;
        }

        int score = 0;
        try {
            score = std::stoi(line.substr(0, tab));
        } catch (const std::exception&) {
            continue;
        }
        if (score <= 0) {
            continue;
        }

        const std::string rawName = line.substr(tab + 1);
        if (rawName.find_first_not_of(' ') == std::string::npos) {
            continue;
        }

        m_entries.push_back(Entry{score, sanitizeName(rawName)});
    }

    sortEntries(m_entries);
    if (m_entries.size() > kMaxEntries) {
        m_entries.resize(kMaxEntries);
    }

    if (!m_entries.empty()) {
        std::cout << "[HighScores] " << m_entries.size()
                  << " records cargados." << std::endl;
    }
    return true;
}

bool HighScoreTable::save() const {
    std::error_code error;
    if (!m_path.parent_path().empty()) {
        std::filesystem::create_directories(m_path.parent_path(), error);
    }

    std::ofstream file(m_path, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "[HighScores] No se pudo guardar el archivo: "
                  << m_path.string() << std::endl;
        return false;
    }

    for (const Entry& entry : m_entries) {
        file << entry.score << '\t' << entry.name << '\n';
    }

    std::cout << "[HighScores] Guardado en " << m_path.string() << std::endl;
    return true;
}

bool HighScoreTable::qualifies(int score) const {
    if (score <= 0) {
        return false;
    }
    if (m_entries.size() < kMaxEntries) {
        return true;
    }
    return score > m_entries.back().score;
}

int HighScoreTable::add(int score, const std::string& name) {
    if (score <= 0) {
        return -1;
    }

    const Entry entry{score, sanitizeName(name)};
    const auto position = std::find_if(
        m_entries.begin(), m_entries.end(),
        [score](const Entry& existing) { return score > existing.score; });
    const int rank = static_cast<int>(std::distance(m_entries.begin(), position));

    m_entries.insert(position, entry);
    if (m_entries.size() > kMaxEntries) {
        m_entries.resize(kMaxEntries);
    }

    return rank < static_cast<int>(kMaxEntries) ? rank : -1;
}

std::string HighScoreTable::sanitizeName(const std::string& name) {
    std::string result;
    result.reserve(kMaxNameLength);

    for (const char character : name) {
        if (result.size() >= kMaxNameLength) {
            break;
        }
        const unsigned char value = static_cast<unsigned char>(character);
        if (std::isalnum(value) != 0 || character == ' ') {
            result.push_back(static_cast<char>(std::toupper(value)));
        }
    }

    while (!result.empty() && result.front() == ' ') {
        result.erase(result.begin());
    }
    while (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }

    if (result.empty()) {
        result = kDefaultName;
    }
    return result;
}
