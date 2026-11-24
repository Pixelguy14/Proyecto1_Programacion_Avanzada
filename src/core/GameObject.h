#pragma once

/**
 * @file GameObject.h
 * @brief Clase base abstracta para todas las entidades del juego.
 *
 * Define la interfaz mínima que cualquier objeto del juego debe implementar.
 * Inspirado en el patrón de entidad del libro "The Nature of Code" (Cap. 2),
 * donde cada agente tiene un ciclo de actualización (física) y renderizado.
 *
 * Al ser abstracta (métodos virtuales puros), no puede instanciarse directamente.
 * Las clases concretas (Fruit, FruitHalf, Particle…) heredan de aquí e implementan
 * su propia lógica de update/render.
 *
 * Uso del polimorfismo:
 * @code
 *   std::vector<GameObject*> objects;
 *   objects.push_back(new Fruit(...));
 *   for (auto* obj : objects) obj->update(dt); // llama al update de cada tipo
 * @endcode
 */
class GameObject {
public:
    /**
     * @brief Actualiza el estado del objeto (física, animación, lógica).
     * @param dt Delta time en segundos desde el último frame.
     */
    virtual void update(float dt) = 0;

    /**
     * @brief Dibuja el objeto en pantalla.
     */
    virtual void render() = 0;

    /**
     * @brief Indica si el objeto está activo (vivo/visible) en la escena.
     * @return true si debe seguir procesándose, false si puede eliminarse.
     */
    virtual bool isAlive() const { return m_alive; }

    /**
     * @brief Destructor virtual para garantizar la destrucción correcta
     *        de subclases a través de punteros a la clase base.
     */
    virtual ~GameObject() = default;

protected:
    /// Estado de actividad del objeto. Las subclases lo gestionan directamente.
    bool m_alive = true;
};
