#ifndef PEDIDO_H
#define PEDIDO_H

#include <string>
#include <iostream>

/**
 * @class Pedido
 * @brief Representa una orden de compra dentro del sistema logístico de E-Commerce.
 *
 * Cada instancia se crea dinámicamente una sola vez y es referenciada mediante
 * punteros desde la Tabla Hash, el Min-Heap y el Árbol AVL, garantizando
 * consistencia inmediata de datos y cero duplicación en memoria.
 */
class Pedido {
private:
    std::string idPedido;
    std::string dniCliente;
    std::string tipoEnvio;
    int nivelPrioridad;
    double montoDolares;
    std::string fecha;   // Formato YYYY-MM-DD
    std::string hora;    // Formato HH:MM:SS
    std::string estado;  // Pendiente, Iniciado, Enviado, EnTransito, En camino, Entregado, Cancelado

    // Índice actual del pedido dentro del arreglo dinámico del Min-Heap (-1 si no está en el Heap)
    int indiceHeap;

public:
    Pedido();
    Pedido(const std::string& idPedido,
           const std::string& dniCliente,
           const std::string& tipoEnvio,
           int nivelPrioridad,
           double montoDolares,
           const std::string& fecha,
           const std::string& hora,
           const std::string& estado);

    // Getters
    std::string getIdPedido() const;
    std::string getDniCliente() const;
    std::string getTipoEnvio() const;
    int getNivelPrioridad() const;
    double getMontoDolares() const;
    std::string getFecha() const;
    std::string getHora() const;
    std::string getEstado() const;
    int getIndiceHeap() const;

    // Setters
    void setEstado(const std::string& nuevoEstado);
    void setIndiceHeap(int indice);

    // Clave compuesta para ordenamiento cronológico único en el Árbol AVL
    std::string getClaveCronologica() const;

    // Utilidades de visualización
    void mostrarDetalle() const;
    void mostrarFilaTabla() const;
};

#endif // PEDIDO_H
