#ifndef TABLA_HASH_H
#define TABLA_HASH_H

#include "Pedido.h"
#include <string>
#include <cstddef>

/**
 * @struct NodoHash
 * @brief Nodo de lista simplemente enlazada para resolución de colisiones
 *        por encadenamiento separado (Separate Chaining).
 */
struct NodoHash {
    Pedido* pedido;
    NodoHash* siguiente;

    NodoHash(Pedido* p, NodoHash* sig = nullptr)
        : pedido(p), siguiente(sig) {}
};

/**
 * @class TablaHash
 * @brief Tabla Hash implementada desde cero con memoria dinámica y punteros.
 *
 * Permite inserción, búsqueda y eliminación de pedidos por idPedido en O(1) promedio.
 * Incluye rehashing dinámico cuando el factor de carga supera 0.75 para mantener
 * rendimiento constante incluso con cientos de miles de pedidos.
 */
class TablaHash {
private:
    NodoHash** buckets;
    size_t capacidad;
    size_t cantidadElementos;
    size_t colisionesTotales;

    // Función hash propia (polinomial base primo 131)
    size_t calcularHash(const std::string& clave) const;

    // Utilidades para redimensionamiento dinámico (rehashing)
    bool esPrimo(size_t n) const;
    size_t siguientePrimo(size_t n) const;
    void rehash();

public:
    explicit TablaHash(size_t capacidadInicial = 15013);
    ~TablaHash();

    // Evitar copias superficiales peligrosas de punteros
    TablaHash(const TablaHash&) = delete;
    TablaHash& operator=(const TablaHash&) = delete;

    // Operaciones principales O(1) promedio
    bool insertar(Pedido* pedido);
    Pedido* buscar(const std::string& idPedido) const;
    bool eliminar(const std::string& idPedido);

    // Libera todos los objetos Pedido* almacenados (usado al destruir o reiniciar el sistema)
    void liberarPedidosYLimpiar();

    // Métricas y diagnóstico
    size_t obtenerCantidad() const;
    size_t obtenerCapacidad() const;
    size_t obtenerColisionesTotales() const;
    double obtenerFactorCarga() const;
    size_t obtenerLongitudMaximaCadena() const;
};

#endif // TABLA_HASH_H
