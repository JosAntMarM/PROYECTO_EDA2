#ifndef SISTEMA_LOGISTICO_H
#define SISTEMA_LOGISTICO_H

#include "Pedido.h"
#include "TablaHash.h"
#include "Heap.h"
#include "ArbolAVL.h"
#include <string>

/**
 * @class SistemaLogistico
 * @brief Orquestador central que coordina y sincroniza la Tabla Hash, el Min-Heap
 *        y el Árbol AVL, manteniendo la propiedad única de memoria sobre los objetos Pedido*.
 */
class SistemaLogistico {
private:
    TablaHash tablaHash;
    MinHeap heapDespacho;
    ArbolAVL arbolCronologico;

    // Estadísticas mantenidas en O(1)
    int totalPedidos;
    double montoTotalAcumulado;

    // Utilidades internas de validación y parsing
    std::string recortarEspacios(const std::string& str) const;
    bool validarFecha(const std::string& fecha) const;
    bool validarHora(const std::string& hora) const;
    bool validarEstado(const std::string& estado) const;
    bool validarDNI(const std::string& dni) const;
    bool parsearLineaCSV(const std::string& linea,
                         std::string& id,
                         std::string& dni,
                         std::string& tipo,
                         int& prioridad,
                         double& monto,
                         std::string& fecha,
                         std::string& hora,
                         std::string& estado) const;

public:
    SistemaLogistico();
    ~SistemaLogistico();

    // Reinicia el sistema liberando toda la memoria dinámica de forma segura
    void reiniciarSistema();

    // 1. Carga masiva desde archivo CSV
    int cargarDesdeCSV(const std::string& rutaArchivo,
                       int& registrosOmitidos,
                       double& tiempoMs,
                       bool limpiarPrevio = false);

    // 2. Registro individual
    bool registrarPedido(const std::string& idPedido,
                         const std::string& dniCliente,
                         const std::string& tipoEnvio,
                         int nivelPrioridad,
                         double montoDolares,
                         const std::string& fecha,
                         const std::string& hora,
                         const std::string& estado,
                         std::string& mensajeError);

    // 3. Búsqueda por ID en O(1) promedio
    Pedido* buscarPedidoPorID(const std::string& idPedido) const;

    // 4. Despacho de N pedidos urgentes mediante Min-Heap
    int despacharLoteUrgente(int cantidadSolicitada, bool mostrarDetalle = true);

    // 5. Consulta cronológica In-Order (primeros N registros)
    void consultarOrdenadosPorFecha(int limite = 30) const;

    // 6. Auditoría por rango de fechas en Árbol AVL
    bool consultarPorRangoFechas(const std::string& fechaInicio,
                                 const std::string& fechaFin,
                                 int& conteoPedidos,
                                 double& montoTotal) const;

    // 7. Actualización de estado manteniendo sincronizadas las 3 estructuras
    bool actualizarEstadoPedido(const std::string& idPedido,
                                const std::string& nuevoEstado,
                                std::string& mensaje);

    // 8. Cancelación y eliminación total sincronizada entre Hash, AVL y Heap
    bool cancelarYEliminarPedido(const std::string& idPedido, std::string& mensaje);

    // 9. Estadísticas generales en O(1)
    void mostrarEstadisticasGenerales() const;

    // 10. Suite automatizada de pruebas de rendimiento exigidas en el PDF
    void ejecutarPruebasRendimiento(const std::string& rutaCSV = "datos/pedidos_masivo.csv");
};

#endif // SISTEMA_LOGISTICO_H
