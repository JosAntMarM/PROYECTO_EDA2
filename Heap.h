#ifndef HEAP_H
#define HEAP_H

#include "Pedido.h"

/**
 * @class MinHeap
 * @brief Cola de prioridad implementada manualmente como un Min-Heap binario
 *        utilizando un arreglo dinámico de punteros (Pedido**).
 *
 * REGLA DE CLAVE DE PRIORIDAD:
 * El sistema requiere despachar primero los pedidos con MAYOR nivelPrioridad
 * (ej. Same Day = 10 antes que Express = 5 y Normal = 1) usando un Min-Heap.
 * Para ello se define la clave de prioridad:
 *     clavePrioridad(p) = -p->getNivelPrioridad()
 * Así, un nivelPrioridad = 10 tiene clave -10, que es menor que -5 y -1,
 * ubicándose siempre en la raíz del Min-Heap. En caso de empate en prioridad,
 * se prioriza el pedido cronológicamente más antiguo (FIFO).
 */
class MinHeap {
private:
    Pedido** datos;
    int capacidad;
    int tamano;

    // Clave de prioridad invertida para que el Min-Heap extraiga primero la máxima urgencia
    int calcularClavePrioridad(const Pedido* p) const;

    // Comparador estricto: true si 'a' debe estar más arriba que 'b' en el Min-Heap
    bool tieneMayorUrgencia(const Pedido* a, const Pedido* b) const;

    // Operaciones internas de reorganización del árbol binario completo
    void subir(int indice);   // Sift-Up / Heapify-Up
    void bajar(int indice);   // Sift-Down / Heapify-Down
    void intercambiar(int i, int j);
    void redimensionar();

public:
    explicit MinHeap(int capacidadInicial = 16384);
    ~MinHeap();

    MinHeap(const MinHeap&) = delete;
    MinHeap& operator=(const MinHeap&) = delete;

    // Operaciones principales O(log n)
    void insertar(Pedido* pedido);
    Pedido* extraerMin();
    bool eliminarEnIndice(int indice);

    // Consultas O(1)
    Pedido* verMin() const;
    int obtenerTamano() const;
    bool estaVacio() const;
    void limpiar();
};

#endif // HEAP_H
