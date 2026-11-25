#include "Sound.h"

#include "AudioEngine.h"

// miniaudio es una cabecera de terceros; se silencian sus advertencias.
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpedantic"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wunused-function"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#  pragma GCC diagnostic ignored "-Wsign-compare"
#  pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
#include <miniaudio.h>
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>

/**
 * @file Sound.cpp
 * @brief Implementación del pool de voces de un efecto de sonido.
 */

struct Sound::Impl {
    static constexpr std::size_t kVoices = 4;

    std::array<ma_sound, kVoices> voices{};   ///< Voces precargadas.
    std::array<bool, kVoices> ready{};        ///< true si la voz se inicializó.
    std::size_t next = 0;                     ///< Siguiente voz en round-robin.
    bool valid = false;                       ///< true si al menos una voz sirve.
};

Sound::Sound() = default;

Sound::~Sound() {
    unload();
}

Sound::Sound(Sound&&) noexcept = default;

Sound& Sound::operator=(Sound&&) noexcept = default;

bool Sound::load(AudioEngine& engine, const std::string& path) {
    unload();

    ma_engine* native = engine.nativeEngine();
    if (native == nullptr) {
        return false;
    }

    m_impl = std::make_unique<Impl>();

    for (std::size_t i = 0; i < Impl::kVoices; ++i) {
        if (ma_sound_init_from_file(native, path.c_str(), MA_SOUND_FLAG_DECODE,
                                    nullptr, nullptr, &m_impl->voices[i]) == MA_SUCCESS) {
            m_impl->ready[i] = true;
            ma_sound_set_volume(&m_impl->voices[i], 1.0f);
        }
    }

    m_impl->valid = std::any_of(m_impl->ready.begin(), m_impl->ready.end(),
                                [](bool readyVoice) { return readyVoice; });
    if (!m_impl->valid) {
        std::cerr << "[Sound] No se pudo cargar el efecto: " << path << std::endl;
        m_impl.reset();
        return false;
    }

    return true;
}

void Sound::play(float volume, float pitch) {
    if (!valid()) {
        return;
    }

    // Primero se busca una voz libre; si todas suenan, se roba la siguiente
    // voz válida del ciclo. El índice nunca debe quedar en una voz sin init.
    std::size_t index = Impl::kVoices;
    for (std::size_t i = 0; i < Impl::kVoices; ++i) {
        const std::size_t candidate = (m_impl->next + i) % Impl::kVoices;
        if (m_impl->ready[candidate]
            && !ma_sound_is_playing(&m_impl->voices[candidate])) {
            index = candidate;
            break;
        }
    }

    if (index == Impl::kVoices) {
        for (std::size_t i = 0; i < Impl::kVoices; ++i) {
            const std::size_t candidate = (m_impl->next + i) % Impl::kVoices;
            if (m_impl->ready[candidate]) {
                index = candidate;
                break;
            }
        }
    }

    if (index == Impl::kVoices) {
        return;
    }

    m_impl->next = (index + 1) % Impl::kVoices;

    ma_sound_stop(&m_impl->voices[index]);
    ma_sound_seek_to_pcm_frame(&m_impl->voices[index], 0);
    ma_sound_set_volume(&m_impl->voices[index], std::clamp(volume, 0.0f, 1.0f));
    ma_sound_set_pitch(&m_impl->voices[index], std::clamp(pitch, 0.25f, 4.0f));
    ma_sound_start(&m_impl->voices[index]);
}

void Sound::stop() {
    if (m_impl == nullptr) {
        return;
    }
    for (std::size_t i = 0; i < Impl::kVoices; ++i) {
        if (m_impl->ready[i]) {
            ma_sound_stop(&m_impl->voices[i]);
        }
    }
}

void Sound::unload() {
    if (m_impl == nullptr) {
        return;
    }
    for (std::size_t i = 0; i < Impl::kVoices; ++i) {
        if (m_impl->ready[i]) {
            ma_sound_uninit(&m_impl->voices[i]);
            m_impl->ready[i] = false;
        }
    }
    m_impl.reset();
}

bool Sound::valid() const {
    return m_impl != nullptr && m_impl->valid;
}
