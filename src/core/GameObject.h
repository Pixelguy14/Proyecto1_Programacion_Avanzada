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
 *   std::vector<std::unique_ptr<GameObject>> objects;
 *   objects.push_back(std::make_unique<Fruit>(...));
 *   for (auto& obj : objects) obj->update(dt); // llama al update de cada tipo
 * @endcode
 *
 * ## ¿Por qué render(Renderer&) y no render()?
 * La versión sin argumentos obligaría a usar un Renderer global o un singleton.
 * Pasar el renderer explícitamente mantiene las dependencias visibles y permite
 * testear las entidades con un renderer simulado. Sigue siendo un método virtual
 * puro, como pide el enunciado.
 */

class Renderer;

class GameObject {
public:
    /**
     * @brief Actualiza el estado del objeto (física, animación, lógica).
     * @param dt Delta time en segundos desde el último frame.
     */
    virtual void update(float dt) = 0;

    /**
     * @brief Dibuja el objeto en pantalla.
     * @param renderer Renderer compartido donde se dibujan las primitivas.
     */
    virtual void render(Renderer& renderer) = 0;

    /**
     * @brief Indica si el objeto está activo (vivo/visible) en la escena.
     * @return true si debe seguir procesándose, false si puede eliminarse.
     */
    virtual bool isAlive() const { return m_alive; }

    /**
     * @brief Indica si el objeto se dibuja con la pasada 3D.
     * @return true si usa mallas con depth test; false para primitivas 2D.
     *
     * La sesión separa las entidades en dos pasadas para que las mallas 3D
     * escriban el buffer de profundidad y las primitivas 2D se dibujen encima
     * sin depth test.
     */
    virtual bool uses3D() const { return false; }

    /**
     * @brief Destructor virtual para garantizar la destrucción correcta
     *        de subclases a través de punteros a la clase base.
     */
    virtual ~GameObject() = default;

protected:
    /// Estado de actividad del objeto. Las subclases lo gestionan directamente.
    bool m_alive = true;
};
