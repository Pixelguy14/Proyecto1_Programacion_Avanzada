#pragma once

#include "core/GameObject.h"
#include "math/Vector2D.h"
#include "physics/Physics2D.h"

#include <glm/glm.hpp>

#include <memory>
#include <utility>

class FruitHalf;
class Renderer;

/**
 * @file Fruit.h
 * @brief Fruta con física parabólica, rotación y estados por banderas (Sprints 1–3).
 *
 * `Fruit` solo contiene estado y comportamiento de una fruta; no conoce la
 * ventana ni OpenGL.
 *
 *   GameObject (abstracta)
 *     └── Fruit
 *           ├── PhysicsBody   (posición, velocidad, aceleración)
 *           ├── m_flags       (registro de banderas de estado)
 *           └── m_angle       (rotación visual)
 *
 * ## Estados con operaciones a nivel de bits
 * El estado no se guarda en varios `bool`, sino en un registro de bits
 * (`unsigned int`). Cada bandera ocupa un bit distinto, de modo que una fruta
 * puede estar "activa y bomba" a la vez y la interacción con la espada se
 * resuelve con una sola comprobación:
 *
 * @code
 *   if (fruit->hasFlag(FruitFlag::kActive) && !fruit->hasFlag(FruitFlag::kBomb)) {
 *       // la espada puede cortarla
 *   }
 * @endcode
 *
 * Operaciones utilizadas:
 * - `|`  para añadir una bandera (`addFlag`).
 * - `&`  para consultar una bandera (`hasFlag`, `isAlive`).
 * - `&~` para quitar una bandera (`removeFlag`).
 * - `^`  para alternar una bandera (`toggleFlag`).
 */
namespace FruitFlag {

/// Banderas de estado de una fruta; cada una ocupa un bit independiente.
enum : unsigned int {
    kNone = 0u,          ///< Sin estado (valor neutro para combinar).
    kActive = 1u << 0,   ///< Viva y disponible para ser cortada.
    kSliced = 1u << 1,   ///< Ya fue cortada; se sustituye por mitades.
    kBomb = 1u << 2,     ///< Es una bomba (no debe cortarse).
};

}  // namespace FruitFlag

class Fruit : public GameObject {
public:
    /**
     * @brief Crea una fruta con estado inicial ya definido por el Spawner.
     * @param position        Posición inicial en píxeles de pantalla.
     * @param velocity        Velocidad inicial en píxeles/segundo.
     * @param radius          Radio en píxeles.
     * @param angularVelocity Velocidad angular en radianes/segundo.
     * @param color           Color RGB en el rango [0, 1].
     *
     * La fruta nace con la bandera `kActive`; el Spawner puede añadir `kBomb`.
     */
    Fruit(const Vector2D<float>& position, const Vector2D<float>& velocity, float radius,
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

    /**
     * @brief Indica si la fruta sigue activa según su registro de banderas.
     * @return true si tiene la bandera `kActive`.
     *
     * Sobrescribe `GameObject::isAlive()` para que el estado lógico viva en un
     * único sitio: las banderas. Una fruta cortada deja de estar viva sin
     * necesidad de un `bool` aparte.
     */
    bool isAlive() const override;

    /// @return Radio de la fruta en píxeles.
    float radius() const { return m_radius; }

    /// @return Posición actual en píxeles.
    const Vector2D<float>& position() const { return m_body.position; }

    /// @return Registro completo de banderas.
    unsigned int flags() const { return m_flags; }

    /**
     * @brief Comprueba si una bandera está activa.
     * @param flag Bandera a consultar (o combinación con `|`).
     * @return true si todos los bits de `flag` están presentes.
     */
    bool hasFlag(unsigned int flag) const {
        return flag != FruitFlag::kNone && (m_flags & flag) == flag;
    }

    /**
     * @brief Añade una bandera al registro.
     * @param flag Bandera a activar.
     */
    void addFlag(unsigned int flag) { m_flags |= flag; }

    /**
     * @brief Quita una bandera del registro.
     * @param flag Bandera a desactivar.
     */
    void removeFlag(unsigned int flag) { m_flags &= ~flag; }

    /**
     * @brief Alterna una bandera (si está activa la apaga y viceversa).
     * @param flag Bandera a alternar.
     */
    void toggleFlag(unsigned int flag) { m_flags ^= flag; }

    /**
     * @brief Cambia el estado a "cortada": activa `kSliced` y quita `kActive`.
     *
     * Tras esta llamada `isAlive()` devuelve false y el bucle elimina la fruta.
     */
    void markSliced();

    /**
     * @brief Marca la fruta como cortada y crea las dos mitades.
     * @param cutDirection Dirección del segmento de espada que produjo el corte.
     * @return Par de mitades, o `{nullptr, nullptr}` si la fruta no era cortable.
     *
     * El diámetro de cada semicírculo se alinea con `cutDirection`; las
     * velocidades se separan a lo largo de la perpendicular, simulando la
     * fuerza del tajo. La segunda mitad gira 180° respecto a la primera.
     */
    std::pair<std::unique_ptr<FruitHalf>, std::unique_ptr<FruitHalf>>
    slice(const Vector2D<float>& cutDirection);

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
    Vector2D<float> stemOffset() const;

private:
    PhysicsBody m_body;              ///< Estado cinemático (posición/velocidad).
    float m_radius;                  ///< Radio en píxeles.
    float m_angle = 0.0f;            ///< Rotación actual en radianes.
    float m_angularVelocity;         ///< Rotación en radianes/segundo.
    glm::vec3 m_color;               ///< Color del cuerpo.
    unsigned int m_flags = FruitFlag::kActive;  ///< Registro de estado por bits.
};
