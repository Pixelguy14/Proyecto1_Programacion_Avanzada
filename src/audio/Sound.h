#pragma once

#include <memory>
#include <string>

class AudioEngine;

/**
 * @file Sound.h
 * @brief Efecto de sonido con un pequeño pool de voces (Sprint 5).
 *
 * Un `Sound` carga el mismo efecto varias veces (4 voces) para que los cortes
 * seguidos se solapen sin cortar el anterior. `play()` elige una voz libre en
 * round-robin; si todas están ocupadas, roba la siguiente.
 *
 *   AssetManager → Sound::load(engine, "sounds/slice.wav")
 *                    └── play(volume, pitch)
 *
 * La cabecera no incluye miniaudio; los detalles viven en el `.cpp` mediante
 * PIMPL, así que el resto del proyecto compila sin arrastrar sus 4 MB.
 */
class Sound {
public:
    Sound();
    ~Sound();

    Sound(Sound&&) noexcept;
    Sound& operator=(Sound&&) noexcept;

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    /**
     * @brief Carga el efecto y prepara sus voces.
     * @param engine Motor de audio ya inicializado.
     * @param path   Ruta del archivo WAV/OGG/MP3.
     * @return true si al menos una voz quedó lista.
     *
     * Carga el archivo completo en memoria (`MA_SOUND_FLAG_DECODE`) porque los
     * efectos son cortos; así `play()` no toca el disco.
     */
    bool load(AudioEngine& engine, const std::string& path);

    /**
     * @brief Reproduce el efecto en una voz libre.
     * @param volume Volumen en [0, 1].
     * @param pitch  Factor de tono/velocidad (1 = original).
     *
     * Si todas las voces están sonando, reutiliza la siguiente del ciclo para
     * no dejar el corte sin sonido.
     */
    void play(float volume = 1.0f, float pitch = 1.0f);

    /// @brief Detiene todas las voces sin liberar el efecto.
    void stop();

    /// @brief Libera las voces de miniaudio.
    void unload();

    /// @return true si el efecto está cargado y puede reproducirse.
    bool valid() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
