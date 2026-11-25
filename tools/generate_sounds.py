#!/usr/bin/env python3
"""Genera los efectos de sonido del juego (Sprint 5).

Uso:
    python3 tools/generate_sounds.py

Los archivos resultantes son WAV PCM mono, 16 bits, 44.1 kHz y se escriben en
`src/graphics/assets/sounds/`. La síntesis usa solo la biblioteca estándar de
Python (sin numpy) y es determinista: los mismos parámetros producen los mismos
archivos. Los efectos son de dominio público.
"""

from pathlib import Path
import math
import random
import struct
import wave

SR = 44100
ROOT = Path(__file__).resolve().parent.parent
OUT_DIR = ROOT / "src" / "graphics" / "assets" / "sounds"


def write_wav(path: Path, samples: list[float], peak: float = 0.85) -> None:
    """Normaliza las muestras a `peak` y escribe un WAV PCM de 16 bits."""
    maximum = max(abs(sample) for sample in samples) or 1.0
    scale = peak / maximum
    data = bytearray()
    for sample in samples:
        value = int(max(-1.0, min(1.0, sample * scale)) * 32767)
        data += struct.pack("<h", value)

    with wave.open(str(path), "wb") as handle:
        handle.setnchannels(1)
        handle.setsampwidth(2)
        handle.setframerate(SR)
        handle.writeframes(bytes(data))


def fade(samples: list[float], milliseconds: float = 4.0) -> None:
    """Aplica un fundido de entrada y salida para evitar clicks."""
    count = int(SR * milliseconds / 1000.0)
    for i in range(min(count, len(samples))):
        gain = i / count
        samples[i] *= gain
        samples[-1 - i] *= gain


def make_slice() -> list[float]:
    """Ruido filtrado + barrido 2.2→0.7 kHz: el 'swish' del corte."""
    duration = 0.18
    count = int(SR * duration)
    rng = random.Random(1)
    samples: list[float] = []
    previous_noise = 0.0

    for i in range(count):
        t = i / SR
        noise = rng.uniform(-1.0, 1.0)
        high_pass = noise - 0.92 * previous_noise
        previous_noise = noise

        envelope = math.exp(-t * 26.0)
        sweep_frequency = 2200.0 - 1500.0 * (t / duration)
        sweep = math.sin(2.0 * math.pi * sweep_frequency * t) * 0.35

        burst = 0.0
        if 0.02 <= t <= 0.07:
            burst = rng.uniform(-1.0, 1.0) * math.exp(-(t - 0.02) * 60.0) * 0.5

        samples.append((high_pass * 0.8 + sweep + burst) * envelope)

    fade(samples, 3.0)
    return samples


def make_bomb() -> list[float]:
    """Seno 95→35 Hz + ruido grave y click: la explosión."""
    duration = 0.95
    count = int(SR * duration)
    rng = random.Random(2)
    samples: list[float] = []
    low_pass = 0.0
    sweep = (35.0 - 95.0) / duration

    for i in range(count):
        t = i / SR
        phase = 2.0 * math.pi * (95.0 * t + 0.5 * sweep * t * t)
        low = math.sin(phase) * math.exp(-t * 3.2)

        low_pass = low_pass * 0.92 + rng.uniform(-1.0, 1.0) * 0.08
        rumble = low_pass * math.exp(-t * 5.0) * 2.2

        click = 0.0
        if t < 0.012:
            click = rng.uniform(-1.0, 1.0) * (1.0 - t / 0.012)

        samples.append(math.tanh((low * 0.9 + rumble + click * 0.5) * 1.8) * 0.8)

    fade(samples, 5.0)
    return samples


def make_miss() -> list[float]:
    """Tono descendente 200→110 Hz + golpe suave: fruta perdida."""
    duration = 0.30
    count = int(SR * duration)
    rng = random.Random(3)
    samples: list[float] = []
    sweep = (110.0 - 200.0) / duration
    low_pass = 0.0

    for i in range(count):
        t = i / SR
        phase = 2.0 * math.pi * (200.0 * t + 0.5 * sweep * t * t)
        tone = math.sin(phase) * math.exp(-t * 11.0)
        low_pass = low_pass * 0.85 + rng.uniform(-1.0, 1.0) * 0.15
        thud = low_pass * math.exp(-t * 18.0) * 0.5
        samples.append((tone * 0.7 + thud) * 0.8)

    fade(samples, 4.0)
    return samples


def make_gameover() -> list[float]:
    """Motivo descendente G4–E4–C4–G3 con drone: fin de partida."""
    duration = 1.20
    count = int(SR * duration)
    notes = [(0.00, 392.0), (0.22, 329.63), (0.44, 261.63), (0.66, 196.0)]
    samples: list[float] = []

    for i in range(count):
        t = i / SR
        total = 0.0
        for start, frequency in notes:
            local = t - start
            if local < 0.0:
                continue
            envelope = math.exp(-local * 5.5)
            total += (math.sin(2.0 * math.pi * frequency * local)
                      + 0.35 * math.sin(4.0 * math.pi * frequency * local)) * envelope * 0.35

        drone = math.sin(2.0 * math.pi * 98.0 * t) * math.exp(-t * 1.5) * 0.25
        samples.append((total + drone) * 0.8)

    fade(samples, 6.0)
    return samples


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    write_wav(OUT_DIR / "slice.wav", make_slice(), 0.80)
    write_wav(OUT_DIR / "bomb.wav", make_bomb(), 0.90)
    write_wav(OUT_DIR / "miss.wav", make_miss(), 0.70)
    write_wav(OUT_DIR / "gameover.wav", make_gameover(), 0.80)
    print(f"Efectos generados en {OUT_DIR}")


if __name__ == "__main__":
    main()
