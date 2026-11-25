#pragma once

/**
 * @file GameState.h
 * @brief Estados de alto nivel de la aplicación (Sprint 4).
 *
 * La máquina de estados vive en `main.cpp`; este enum solo nombra los estados
 * para que la UI, los fondos y el bucle principal compartan el mismo tipo.
 *
 *   Menu ──Enter/Espacio──▶ Playing ──Esc/P──▶ Paused
 *     ▲                        │                  │
 *     │                        │ vidas = 0        │ Esc/P
 *     │                        ▼                  │
 *     └──────Esc──────────── GameOver ◀──────────┘
 *                              │
 *                              └──Enter/Espacio──▶ Playing (reinicio)
 */
enum class GameState {
    kMenu,      ///< Pantalla de título; no hay simulación.
    kPlaying,   ///< Partida activa: física, cortes y HUD.
    kPaused,    ///< Partida congelada con overlay de pausa.
    kGameOver,  ///< Sin vidas; se muestra el puntaje final.
};
