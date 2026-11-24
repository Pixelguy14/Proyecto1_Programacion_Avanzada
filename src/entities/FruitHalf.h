#pragma once

#include "core/GameObject.h"
#include "math/Vector2D.h"
#include "physics/Physics2D.h"

#include <glm/glm.hpp>

class Renderer;

/**
 * @file FruitHalf.h
 * @brief Mitad de fruta generada al cortar (Sprint 3).
 *
 * Cuando la espada corta una `Fruit`, esta se sustituye por dos `FruitHalf`.
 * Cada mitad es un semicírculo con el diámetro alineado con el tajo, y lleva su
 * propia física: gravedad, velocidad inicial divergente y rotación.
 *
 *   Fruit::slice()
 *     └── FruitHalf (dos instancias con velocidades opuestas)
 *           ├── Physics2D::integrate()
 *           ├── Physics2D::integrateRotation()
 *           └── Renderer::drawSemicircle()
 *
 * La forma visual es un semicírculo con un núcleo más claro: el borde exterior
 * conserva el color de la fruta y el interior simula la pulpa.
 */
class FruitHalf : public GameObject {
public:
    /**
     * @brief Crea una mitad con estado inicial ya resuelto por `Fruit::slice`.
     * @param position        Centro del semicírculo en píxeles.
     * @param velocity        Velocidad inicial en píxeles/segundo.
     * @param radius          Radio del semicírculo en píxeles.
     * @param angle           Orientación del diámetro en radianes.
     * @param angularVelocity Velocidad angular en radianes/segundo.
     * @param color           Color RGB de la cáscara en el rango [0, 1].
     */
    FruitHalf(const Vector2D<float>& position, const Vector2D<float>& velocity,
              float radius, float angle, float angularVelocity, const glm::vec3& color);

    /**
     * @brief Aplica gravedad, integra posición y avanza la rotación.
     * @param dt Delta time en segundos.
     *
     * Si la mitad cae por debajo de la pantalla, se marca como no viva para que
     * el bucle la elimine.
     */
    void update(float dt) override;

    /**
     * @brief Dibuja la cáscara y la pulpa como dos semicírculos concéntricos.
     * @param renderer Renderer compartido.
     */
    void render(Renderer& renderer) override;

    /// @return Centro actual en píxeles de pantalla.
    const Vector2D<float>& position() const { return m_body.position; }

    /// @return Velocidad actual en píxeles/segundo.
    const Vector2D<float>& velocity() const { return m_body.velocity; }

    /// @return Radio del semicírculo en píxeles.
    float radius() const { return m_radius; }

    /// @return Orientación del diámetro en radianes.
    float angle() const { return m_angle; }

private:
    /**
     * @brief Comprueba si la mitad ya salió por la parte inferior de la pantalla.
     * @return true si cayó lo suficiente como para poder eliminarla.
     *
     * Igual que en `Fruit`, solo cuenta cuando la velocidad es descendente.
     */
    bool hasFallenOffScreen() const;

private:
    PhysicsBody m_body;        ///< Estado cinemático (posición/velocidad).
    float m_radius;            ///< Radio del semicírculo en píxeles.
    float m_angle = 0.0f;      ///< Orientación del diámetro en radianes.
    float m_angularVelocity;   ///< Giro en radianes/segundo.
    glm::vec3 m_color;         ///< Color de la cáscara.
};
