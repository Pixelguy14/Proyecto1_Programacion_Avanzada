#pragma once

#include <memory>

struct ma_engine;

/**
 * @file AudioEngine.h
 * @brief Inicialización de miniaudio y volumen maestro (Sprint 5).
 *
 * `AudioEngine` envuelve el motor de miniaudio con una interfaz mínima. La
 * cabecera de miniaudio no se expone: la implementación vive en el `.cpp` y los
 * `Sound` acceden al motor nativo a través de un método privado amistoso.
 *
 *   main
 *     ├── audio.init()
 *     ├── assets.loadSound(audio, "sfx_slice", path)
 *     ├── audio.setMasterVolume(0.8f)
 *     └── audio.shutdown()
 *
 * El motor debe sobrevivir a todos los `Sound` y al `AssetManager`; si no hay
 * dispositivo de audio disponible, `init()` devuelve false y el juego sigue
 * funcionando en silencio.
 */
class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    /**
     * @brief Inicializa el motor de miniaudio con el dispositivo por defecto.
     * @return true si el motor quedó listo para reproducir sonidos.
     *
     * Es seguro llamarlo dos veces: la segunda no hace nada si ya estaba
     * inicializado. Un fallo se reporta por `std::cerr` y deja el motor inerte.
     */
    bool init();

    /// @brief Detiene el motor y libera sus recursos.
    void shutdown();

    /// @return true si el motor está inicializado.
    bool ready() const;

    /**
     * @brief Ajusta el volumen maestro.
     * @param volume Volumen en [0, 1]; se recorta si está fuera de rango.
     */
    void setMasterVolume(float volume);

    /// @return Volumen maestro actual en [0, 1].
    float masterVolume() const { return m_masterVolume; }

private:
    friend class Sound;

    /**
     * @brief Devuelve el motor nativo de miniaudio.
     * @return Puntero al motor, o nullptr si no está inicializado.
     *
     * Solo lo usan los `Sound` para inicializar sus voces.
     */
    ma_engine* nativeEngine();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;  ///< Detalles de miniaudio (ocultos).
    float m_masterVolume = 0.8f;   ///< Volumen maestro [0, 1].
};
