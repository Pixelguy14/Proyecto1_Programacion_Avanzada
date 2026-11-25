# Efectos de sonido

Generados proceduralmente para este proyecto; no contienen samples de terceros.
Formato: **WAV PCM mono, 16 bits, 44.1 kHz**.

| Archivo | Uso | Síntesis |
|---|---|---|
| `slice.wav` | Corte de fruta | Ruido filtrado + barrido 2.2→0.7 kHz, 0.18 s |
| `bomb.wav` | Explosión | Seno 95→35 Hz + ruido grave y click, 0.95 s |
| `miss.wav` | Fruta perdida | Tono descendente 200→110 Hz + golpe suave, 0.30 s |
| `gameover.wav` | Fin de partida | Motivo descendente G4–E4–C4–G3 con drone, 1.2 s |

Los efectos son de dominio público y se pueden regenerar de forma determinista con:

```bash
python3 tools/generate_sounds.py
```
