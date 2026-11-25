# miniaudio

- **Versión:** v0.11.26 (2026-10-31)
- **Fuente:** https://github.com/mackron/miniaudio
- **Licencia:** dominio público o MIT-0 (ver el encabezado de `miniaudio.h`)
- **Uso:** librería *header-only*. `src/audio/AudioEngine.cpp` define
  `MINIAUDIO_IMPLEMENTATION` una sola vez y el resto del proyecto incluye la
  cabecera a través de las clases de `src/audio/`.

No se modifica el archivo original; para actualizar, reemplaza `miniaudio.h` por
la versión deseada y vuelve a compilar.
