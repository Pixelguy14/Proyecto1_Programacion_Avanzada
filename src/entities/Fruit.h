#pragma once

#include "core/GameObject.h"
#include "physics/Physics2D.h"

#include <glm/glm.hpp>

/**
 * @file Fruit.h
 * @brief Entidad de fruta del Sprint 1: física parabólica y rotación.
 *
 * Fruit es la primera clase concreta que hereda de GameObject. Solo contiene
 * estado y comportamiento de una fruta; no conoce la ventana ni OpenGL.
 *
 *   GameObject (abstracta)
 *     └── Fruit
 *           ├── PhysicsBody  (posición, velocidad, aceleración)
 *           └── m_angle      (rotación visual)
 */
class Fruit : public GameObject {
public:
    /**
     * @brief Crea una fruta con estado inicial ya definido por el Spawner.
     * @param position        Posición inicial en píxeles de pantalla.
     * @param velocity        Velocidad inicial en píxeles/segundo.
     * @param radius          Radio en píxeles.
     * @param angularVelocity Velocidad angular en radianes/segundo.
     * @param color           Color RGB en el rango [0, 1].
     */
    Fruit(const glm::vec2& position, const glm::vec2& velocity, float radius,
          float angularVelocity, const glm::vec3& color);

    /**
     * @brief Aplica gravedad, integra la posición y actualiza la rotación.
     * @param dt Delta time en segundos.
     *
     * Si la fruta cae por debajo de la pantalla, se marca como no viva para que
     * el bucle la elimine.
     */
    void update(float dt) override;

    /**
     * @brief Dibuja el cuerpo circular y el tallo rotado.
     * @param renderer Renderer compartido.
     */
    void render(Renderer& renderer) override;

    /// @return Radio de la fruta en píxeles.
    float radius() const { return m_radius; }

    /// @return Posición actual en píxeles (la usará la colisión en el Sprint 3).
    const glm::vec2& position() const { return m_body.position; }

private:
    /**
     * @brief Comprueba si la fruta ya salió por la parte inferior de la pantalla.
     * @return true si cayó lo suficiente como para poder eliminarla.
     *
     * Solo se considera caída cuando la velocidad es descendente, para no matar
     * a la fruta recién lanzada desde debajo de la pantalla.
     */
    bool hasFallenOffScreen() const;

    /**
     * @brief Calcula el desplazamiento del tallo según el ángulo actual.
     * @return Offset desde el centro de la fruta hasta el centro del tallo.
     */
    glm::vec2 stemOffset() const;

private:
    PhysicsBody m_body;              ///< Estado cinemático (posición/velocidad).
    float m_radius;                  ///< Radio en píxeles.
    float m_angle = 0.0f;            ///< Rotación actual en radianes.
    float m_angularVelocity;         ///< Rotación en radianes/segundo.
    glm::vec3 m_color;               ///< Color del cuerpo.
};
