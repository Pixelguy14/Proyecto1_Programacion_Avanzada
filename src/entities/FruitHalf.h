#pragma once

#include "core/GameObject.h"
#include "graphics/FruitMesh.h"
#include "math/Vector2D.h"
#include "physics/Physics2D.h"

#include <glm/glm.hpp>

class Renderer;

/**
 * @file FruitHalf.h
 * @brief Mitad de fruta generada al cortar (Sprint 3, ajustada en Sprint 5).
 *
 * Cuando la espada corta una `Fruit`, esta se sustituye por dos `FruitHalf`.
 * Cada mitad es una silueta 2D con el borde plano alineado con el tajo, y lleva
 * su propia física: gravedad, velocidad inicial divergente y rotación.
 *
 *   Fruit::slice()
 *     └── FruitHalf (dos instancias con velocidades opuestas)
 *           ├── Physics2D::integrate()
 *           ├── Physics2D::integrateRotation()
 *           └── Renderer::drawPolygon()
 *
 * La silueta depende del tipo de fruta: el plátano es alargado, el limón
 * estirado y el melón ovalado. Se dibuja una capa de cáscara y otra de pulpa
 * más pequeña, ambas con el color correspondiente.
 */
class FruitHalf : public GameObject {
public:
    /**
     * @brief Crea una mitad con estado inicial ya resuelto por `Fruit::slice`.
     * @param position        Centro del borde plano en píxeles.
     * @param velocity        Velocidad inicial en píxeles/segundo.
     * @param radius          Radio/altura base de la mitad en píxeles.
     * @param angle           Orientación del borde plano en radianes.
     * @param angularVelocity Velocidad angular en radianes/segundo.
     * @param shellColor      Color RGB de la cáscara en el rango [0, 1].
     * @param fleshColor      Color RGB de la pulpa en el rango [0, 1].
     * @param meshId          Tipo de fruta, para elegir la silueta de la mitad.
     */
    FruitHalf(const Vector2D<float>& position, const Vector2D<float>& velocity,
              float radius, float angle, float angularVelocity,
              const glm::vec3& shellColor, const glm::vec3& fleshColor,
              FruitMeshId meshId);

    /**
     * @brief Aplica gravedad, integra posición y avanza la rotación.
     * @param dt Delta time en segundos.
     *
     * Si la mitad cae por debajo de la pantalla, se marca como no viva para que
     * el bucle la elimine.
     */
    void update(float dt) override;

    /**
     * @brief Dibuja la cáscara y la pulpa como dos siluetas concéntricas.
     * @param renderer Renderer compartido.
     */
    void render(Renderer& renderer) override;

    /// @return Centro actual en píxeles de pantalla.
    const Vector2D<float>& position() const { return m_body.position; }

    /// @return Velocidad actual en píxeles/segundo.
    const Vector2D<float>& velocity() const { return m_body.velocity; }

    /// @return Radio/altura base de la mitad en píxeles.
    float radius() const { return m_radius; }

    /// @return Orientación del borde plano en radianes.
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
    float m_radius;            ///< Radio/altura base de la mitad en píxeles.
    float m_angle = 0.0f;      ///< Orientación del diámetro en radianes.
    float m_angularVelocity;   ///< Giro en radianes/segundo.
    glm::vec3 m_shellColor;    ///< Color de la cáscara.
    glm::vec3 m_fleshColor;    ///< Color de la pulpa.
    FruitMeshId m_meshId;      ///< Tipo de fruta (silueta de la mitad).
};
