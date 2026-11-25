#include "AudioEngine.h"

// La implementación de miniaudio se compila una sola vez en este archivo.
// La cabecera es de terceros: se silencian advertencias que no controlamos.
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpedantic"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wunused-function"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#  pragma GCC diagnostic ignored "-Wsign-compare"
#  pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <iostream>

/**
 * @file AudioEngine.cpp
 * @brief Implementación del motor de audio basado en miniaudio.
 */

struct AudioEngine::Impl {
    ma_engine engine{};          ///< Motor de miniaudio.
    bool initialized = false;    ///< true tras ma_engine_init().
};

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::init() {
    if (m_impl != nullptr && m_impl->initialized) {
        return true;
    }

    m_impl = std::make_unique<Impl>();
    if (ma_engine_init(nullptr, &m_impl->engine) != MA_SUCCESS) {
        std::cerr << "[AudioEngine] No se pudo inicializar el dispositivo de audio." << std::endl;
        m_impl.reset();
        return false;
    }

    m_impl->initialized = true;
    ma_engine_set_volume(&m_impl->engine, m_masterVolume);
    return true;
}

void AudioEngine::shutdown() {
    if (m_impl != nullptr && m_impl->initialized) {
        ma_engine_uninit(&m_impl->engine);
    }
    m_impl.reset();
}

bool AudioEngine::ready() const {
    return m_impl != nullptr && m_impl->initialized;
}

void AudioEngine::setMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    if (ready()) {
        ma_engine_set_volume(&m_impl->engine, m_masterVolume);
    }
}

ma_engine* AudioEngine::nativeEngine() {
    return ready() ? &m_impl->engine : nullptr;
}
