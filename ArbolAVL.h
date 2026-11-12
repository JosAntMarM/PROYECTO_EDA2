#ifndef ARBOL_AVL_H
#define ARBOL_AVL_H

#include "Pedido.h"
#include <string>

/**
 * @struct NodoAVL
 * @brief Nodo del Árbol Binario de Búsqueda Auto-balanceado (AVL).
 */
struct NodoAVL {
    Pedido* pedido;
    NodoAVL* izquierdo;
    NodoAVL* derecho;
    int altura;

    explicit NodoAVL(Pedido* p)
        : pedido(p), izquierdo(nullptr), derecho(nullptr), altura(1) {}
};

/**
 * @class ArbolAVL
 * @brief Árbol AVL implementado desde cero con punteros y memoria dinámica.
 *
 * Mantiene los pedidos ordenados cronológicamente mediante la clave compuesta
 * (fecha, hora, idPedido), garantizando inserción, eliminación y búsqueda en
 * O(log n), recorrido In-Order y consultas analíticas por rango de fechas
 * con poda de subárboles fuera de rango.
 */
class ArbolAVL {
private:
    NodoAVL* raiz;
    int cantidadNodos;

    // Utilidades de altura y factor de equilibrio
    int obtenerAltura(NodoAVL* nodo) const;
    int obtenerBalance(NodoAVL* nodo) const;
    int maximo(int a, int b) const;
    void actualizarAltura(NodoAVL* nodo);

    // Las 4 rotaciones obligatorias de un Árbol AVL
    NodoAVL* rotacionDerecha(NodoAVL* y);           // Caso LL
    NodoAVL* rotacionIzquierda(NodoAVL* x);         // Caso RR
    NodoAVL* rotacionIzquierdaDerecha(NodoAVL* z);  // Caso LR
    NodoAVL* rotacionDerechaIzquierda(NodoAVL* z);  // Caso RL

    // Operaciones recursivas internas
    NodoAVL* insertarRecursivo(NodoAVL* nodo, Pedido* pedido, bool& insertado);
    NodoAVL* obtenerMinimoNodo(NodoAVL* nodo) const;
    NodoAVL* eliminarRecursivo(NodoAVL* nodo, const std::string& claveCronologica, bool& eliminado);
    void inOrderLimitadoRecursivo(NodoAVL* nodo, int& mostrados, int limite) const;
    void consultarRangoRecursivo(NodoAVL* nodo,
                                 const std::string& fechaInicio,
                                 const std::string& fechaFin,
                                 int& conteoPedidos,
                                 double& montoTotal) const;
    void destruirRecursivo(NodoAVL* nodo);

public:
    ArbolAVL();
    ~ArbolAVL();

    ArbolAVL(const ArbolAVL&) = delete;
    ArbolAVL& operator=(const ArbolAVL&) = delete;

    // Operaciones principales
    bool insertar(Pedido* pedido);
    bool eliminar(const Pedido* pedido);

    // Recorrido In-Order limitado (por defecto primeros 30 resultados)
    void mostrarInOrder(int limite = 30) const;

    // Auditoría por rango de fechas [fechaInicio, fechaFin] con poda de ramas
    void consultarPorRangoFechas(const std::string& fechaInicio,
                                 const std::string& fechaFin,
                                 int& conteoPedidos,
                                 double& montoTotal) const;

    // Consultas de estado
    int obtenerCantidadNodos() const;
    int obtenerAlturaArbol() const;
    void limpiar();
};

#endif // ARBOL_AVL_H
