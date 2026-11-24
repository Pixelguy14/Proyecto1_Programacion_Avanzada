#pragma once

#include <cmath>

/**
 * @file Vector2D.h
 * @brief Vector 2D genérico con sobrecarga de operadores (Sprint 2).
 *
 * Se usa en la lógica de la espada para calcular deltas y distancias sin
 * depender de GLM. GLM sigue usándose en el renderer, donde sus funciones
 * matriciales (`glm::ortho`, `glm::rotate`) son la herramienta adecuada.
 *
 * Ejemplo:
 * @code
 *   Vector2D<float> delta = actual - anterior;
 *   float distancia = delta.length();
 *   Vector2D<float> normal = delta.normalized().perpendicular();
 * @endcode
 *
 * @tparam T Tipo de las componentes. Pensado para `float` (con `int` la
 *           longitud se trunca al convertir desde double).
 */
template <typename T>
class Vector2D {
public:
    T x{};  ///< Componente horizontal.
    T y{};  ///< Componente vertical.

    /// @brief Vector nulo (0, 0).
    constexpr Vector2D() = default;

    /**
     * @brief Construye un vector con componentes explícitas.
     * @param xValue Componente horizontal.
     * @param yValue Componente vertical.
     */
    constexpr Vector2D(T xValue, T yValue) : x(xValue), y(yValue) {}

    /**
     * @brief Asignación de copia.
     *
     * Se declara explícitamente para documentar la operación `=` que pide el
     * enunciado; el compilador generaría una equivalente.
     */
    Vector2D& operator=(const Vector2D&) = default;

    /// @brief Suma de vectores componente a componente.
    constexpr Vector2D operator+(const Vector2D& other) const {
        return {x + other.x, y + other.y};
    }

    /// @brief Resta de vectores componente a componente (delta de movimiento).
    constexpr Vector2D operator-(const Vector2D& other) const {
        return {x - other.x, y - other.y};
    }

    /// @brief Negación del vector.
    constexpr Vector2D operator-() const { return {-x, -y}; }

    /// @brief Multiplicación por un escalar.
    constexpr Vector2D operator*(T scalar) const { return {x * scalar, y * scalar}; }

    /// @brief División por un escalar.
    constexpr Vector2D operator/(T scalar) const { return {x / scalar, y / scalar}; }

    /// @brief Suma acumulada.
    Vector2D& operator+=(const Vector2D& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    /// @brief Resta acumulada.
    Vector2D& operator-=(const Vector2D& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    /// @brief Multiplicación acumulada por un escalar.
    Vector2D& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    /// @brief División acumulada por un escalar.
    Vector2D& operator/=(T scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    /// @brief Comparación exacta componente a componente.
    constexpr bool operator==(const Vector2D& other) const {
        return x == other.x && y == other.y;
    }

    /// @brief Desigualdad exacta componente a componente.
    constexpr bool operator!=(const Vector2D& other) const { return !(*this == other); }

    /**
     * @brief Producto punto.
     * @param other Otro vector.
     * @return Producto escalar.
     */
    constexpr T dot(const Vector2D& other) const { return x * other.x + y * other.y; }

    /**
     * @brief Longitud al cuadrado (evita la raíz y es más rápida para comparar).
     * @return |v|²
     */
    constexpr T lengthSquared() const { return x * x + y * y; }

    /**
     * @brief Longitud euclidiana.
     * @return |v|
     */
    T length() const { return static_cast<T>(std::sqrt(static_cast<double>(lengthSquared()))); }

    /**
     * @brief Distancia euclidiana a otro punto.
     * @param other Punto destino.
     * @return Distancia entre ambos puntos.
     */
    T distanceTo(const Vector2D& other) const { return (*this - other).length(); }

    /**
     * @brief Devuelve el vector unitario en la misma dirección.
     * @return Vector normalizado, o (0,0) si la longitud es cero.
     */
    Vector2D normalized() const {
        const T lengthValue = length();
        return lengthValue > T{} ? (*this) / lengthValue : Vector2D{};
    }

    /**
     * @brief Vector perpendicular (rotado 90° en sentido antihorario).
     * @return (-y, x)
     */
    constexpr Vector2D perpendicular() const { return {-y, x}; }
};

/// @brief Multiplicación escalar × vector (conmutativa con `vector * escalar`).
template <typename T>
constexpr Vector2D<T> operator*(T scalar, const Vector2D<T>& vector) {
    return vector * scalar;
}

/// @brief Alias habitual para coordenadas de pantalla.
using Vector2f = Vector2D<float>;
