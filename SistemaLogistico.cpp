#include "SistemaLogistico.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cmath>

SistemaLogistico::SistemaLogistico()
    : tablaHash(15013), heapDespacho(16384), arbolCronologico(),
      totalPedidos(0), montoTotalAcumulado(0.0) {}

SistemaLogistico::~SistemaLogistico() {
    reiniciarSistema();
}

void SistemaLogistico::reiniciarSistema() {
    heapDespacho.limpiar();
    arbolCronologico.limpiar();
    tablaHash.liberarPedidosYLimpiar(); // Libera cada Pedido* exactamente una vez
    totalPedidos = 0;
    montoTotalAcumulado = 0.0;
}

std::string SistemaLogistico::recortarEspacios(const std::string& str) const {
    size_t inicio = 0;
    while (inicio < str.length() && std::isspace(static_cast<unsigned char>(str[inicio]))) {
        inicio++;
    }
    size_t fin = str.length();
    while (fin > inicio && std::isspace(static_cast<unsigned char>(str[fin - 1]))) {
        fin--;
    }
    return str.substr(inicio, fin - inicio);
}

bool SistemaLogistico::validarDNI(const std::string& dni) const {
    if (dni.length() < 8 || dni.length() > 12) return false;
    for (size_t i = 0; i < dni.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(dni[i]))) return false;
    }
    return true;
}

bool SistemaLogistico::validarFecha(const std::string& fecha) const {
    // 1. Verificar el formato YYYY-MM-DD.
    if (fecha.length() != 10) return false;
    if (fecha[4] != '-' || fecha[7] != '-') return false;

    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;

        if (!std::isdigit(static_cast<unsigned char>(fecha[i]))) {
            return false;
        }
    }

    // 2. Extraer los componentes de la fecha.
    int anio = std::stoi(fecha.substr(0, 4));
    int mes = std::stoi(fecha.substr(5, 2));
    int dia = std::stoi(fecha.substr(8, 2));

    if (anio < 1 || mes < 1 || mes > 12) return false;

    // 3. Cantidad de dias de cada mes en un año no bisiesto.
    int diasPorMes[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    // 4. Ajustar febrero si el año es bisiesto.
    bool esBisiesto = (anio % 400 == 0) ||
                     (anio % 4 == 0 && anio % 100 != 0);

    if (esBisiesto) {
        diasPorMes[1] = 29;
    }

    // 5. Comprobar que el dia exista dentro de ese mes.
    return dia >= 1 && dia <= diasPorMes[mes - 1];
}

bool SistemaLogistico::validarHora(const std::string& hora) const {
    // Formato estricto HH:MM:SS
    if (hora.length() != 8) return false;
    if (hora[2] != ':' || hora[5] != ':') return false;
    for (int i = 0; i < 8; ++i) {
        if (i == 2 || i == 5) continue;
        if (!std::isdigit(static_cast<unsigned char>(hora[i]))) return false;
    }
    int hh = std::stoi(hora.substr(0, 2));
    int mm = std::stoi(hora.substr(3, 2));
    int ss = std::stoi(hora.substr(6, 2));
    return (hh >= 0 && hh <= 23) && (mm >= 0 && mm <= 59) && (ss >= 0 && ss <= 59);
}

bool SistemaLogistico::validarEstado(const std::string& estado) const {
    return (estado == "Pendiente" ||
            estado == "Iniciado" ||
            estado == "Enviado" ||
            estado == "EnTransito" ||
            estado == "En camino" ||
            estado == "Entregado" ||
            estado == "Cancelado");
}

bool SistemaLogistico::parsearLineaCSV(const std::string& linea,
                                       std::string& id,
                                       std::string& dni,
                                       std::string& tipo,
                                       int& prioridad,
                                       double& monto,
                                       std::string& fecha,
                                       std::string& hora,
                                       std::string& estado) const {
    std::stringstream ss(linea);
    std::string campoPrioridad, campoMonto;

    if (!std::getline(ss, id, ',')) return false;
    if (!std::getline(ss, dni, ',')) return false;
    if (!std::getline(ss, tipo, ',')) return false;
    if (!std::getline(ss, campoPrioridad, ',')) return false;
    if (!std::getline(ss, campoMonto, ',')) return false;
    if (!std::getline(ss, fecha, ',')) return false;
    if (!std::getline(ss, hora, ',')) return false;
    if (!std::getline(ss, estado)) return false;

    id = recortarEspacios(id);
    dni = recortarEspacios(dni);
    tipo = recortarEspacios(tipo);
    campoPrioridad = recortarEspacios(campoPrioridad);
    campoMonto = recortarEspacios(campoMonto);
    fecha = recortarEspacios(fecha);
    hora = recortarEspacios(hora);
    estado = recortarEspacios(estado);

    if (id.empty() || tipo.empty()) return false;
    if (!validarDNI(dni) || !validarFecha(fecha) || !validarHora(hora) || !validarEstado(estado)) {
        return false;
    }

    try {
        std::size_t posPrioridad = 0;
        std::size_t posMonto = 0;

        prioridad = std::stoi(campoPrioridad, &posPrioridad);
        monto = std::stod(campoMonto, &posMonto);

        // Rechazar campos que tengan caracteres sobrantes.
        if (posPrioridad != campoPrioridad.length() ||
            posMonto != campoMonto.length()) {
            return false;
        }
    } catch (...) {
        return false;
    }

    // La prioridad debe ser positiva.
    if (prioridad < 1) return false;

    // El monto debe ser finito y no negativo.
    if (!std::isfinite(monto) || monto < 0.0) return false;

    return true;
}

int SistemaLogistico::cargarDesdeCSV(const std::string& rutaArchivo,
                                     int& registrosOmitidos,
                                     double& tiempoMs,
                                     bool limpiarPrevio) {
    registrosOmitidos = 0;
    tiempoMs = 0.0;

    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        return -1; // Error al abrir archivo
    }

    if (limpiarPrevio) {
        reiniciarSistema();
    }

    auto inicio = std::chrono::high_resolution_clock::now();

    std::string linea;
    int cargados = 0;
    bool primeraLinea = true;

    while (std::getline(archivo, linea)) {
        std::string limpia = recortarEspacios(linea);
        if (limpia.empty()) continue;

        // Omitir cabecera si existe
        if (primeraLinea) {
            primeraLinea = false;
            if (limpia.find("idPedido") != std::string::npos) {
                continue;
            }
        }

        std::string id, dni, tipo, fecha, hora, estado;
        int prioridad = 0;
        double monto = 0.0;

        if (!parsearLineaCSV(limpia, id, dni, tipo, prioridad, monto, fecha, hora, estado)) {
            registrosOmitidos++;
            continue;
        }

        // Control de duplicados en O(1) promedio
        if (tablaHash.buscar(id) != nullptr) {
            registrosOmitidos++;
            continue;
        }

        Pedido* nuevoPedido = new Pedido(id, dni, tipo, prioridad, monto, fecha, hora, estado);

        // Insertar simultáneamente en las 3 estructuras
        tablaHash.insertar(nuevoPedido);
        arbolCronologico.insertar(nuevoPedido);
        if (estado == "Pendiente") {
            heapDespacho.insertar(nuevoPedido);
        }

        totalPedidos++;
        montoTotalAcumulado += monto;
        cargados++;
    }

    auto fin = std::chrono::high_resolution_clock::now();
    tiempoMs = std::chrono::duration<double, std::milli>(fin - inicio).count();

    archivo.close();
    return cargados;
}

bool SistemaLogistico::registrarPedido(const std::string& idPedido,
                                       const std::string& dniCliente,
                                       const std::string& tipoEnvio,
                                       int nivelPrioridad,
                                       double montoDolares,
                                       const std::string& fecha,
                                       const std::string& hora,
                                       const std::string& estado,
                                       std::string& mensajeError) {
    std::string id = recortarEspacios(idPedido);
    std::string dni = recortarEspacios(dniCliente);
    std::string tipo = recortarEspacios(tipoEnvio);
    std::string fec = recortarEspacios(fecha);
    std::string hor = recortarEspacios(hora);
    std::string est = recortarEspacios(estado);

    if (id.empty()) {
        mensajeError = "El ID del pedido no puede estar vacio.";
        return false;
    }
    if (tablaHash.buscar(id) != nullptr) {
        mensajeError = "Ya existe un pedido registrado con el ID '" + id + "'.";
        return false;
    }
    if (!validarDNI(dni)) {
        mensajeError = "DNI invalido. Debe contener entre 8 y 12 digitos numericos.";
        return false;
    }
    if (tipo.empty()) {
        mensajeError = "El tipo de envio no puede estar vacio.";
        return false;
    }
    if (nivelPrioridad < 1) {
        mensajeError = "El nivel de prioridad debe ser un entero positivo (>= 1).";
        return false;
    }
    if (!std::isfinite(montoDolares) || montoDolares < 0.0) {
        mensajeError = "El monto debe ser un numero finito y no negativo.";
        return false;
    }
    if (!validarFecha(fec)) {
        mensajeError = "Fecha invalida. Use el formato estricto YYYY-MM-DD (ej. 2026-06-15).";
        return false;
    }
    if (!validarHora(hor)) {
        mensajeError = "Hora invalida. Use el formato estricto HH:MM:SS (ej. 14:30:00).";
        return false;
    }
    if (!validarEstado(est)) {
        mensajeError = "Estado invalido. Valores permitidos: Pendiente, Iniciado, Enviado, EnTransito, En camino, Entregado, Cancelado.";
        return false;
    }

    Pedido* nuevo = new Pedido(id, dni, tipo, nivelPrioridad, montoDolares, fec, hor, est);
    tablaHash.insertar(nuevo);
    arbolCronologico.insertar(nuevo);
    if (est == "Pendiente") {
        heapDespacho.insertar(nuevo);
    }

    totalPedidos++;
    montoTotalAcumulado += montoDolares;
    return true;
}

Pedido* SistemaLogistico::buscarPedidoPorID(const std::string& idPedido) const {
    return tablaHash.buscar(recortarEspacios(idPedido));
}

int SistemaLogistico::despacharLoteUrgente(int cantidadSolicitada, bool mostrarDetalle) {
    if (cantidadSolicitada <= 0) return 0;

    int despachados = 0;
    if (mostrarDetalle) {
        std::cout << "\n--- DESPACHO DE PEDIDOS URGENTES (MIN-HEAP) ---\n";
        std::cout << std::left
                  << std::setw(12) << "ID Pedido"
                  << std::setw(11) << "DNI"
                  << std::setw(11) << "Envio"
                  << std::setw(6)  << "Prio"
                  << std::right << std::setw(10) << "Monto" << "  "
                  << std::left << std::setw(12) << "Fecha"
                  << std::setw(10) << "Hora"
                  << std::setw(12) << "NuevoEstado" << "\n";
        std::cout << std::string(86, '-') << "\n";
    }

    while (despachados < cantidadSolicitada && !heapDespacho.estaVacio()) {
        Pedido* urgente = heapDespacho.extraerMin();
        if (urgente == nullptr) break;

        // Actualizar estado a EnTransito (reflejado automáticamente en Hash y AVL)
        urgente->setEstado("EnTransito");
        despachados++;

        if (mostrarDetalle) {
            urgente->mostrarFilaTabla();
        }
    }

    if (mostrarDetalle) {
        std::cout << std::string(86, '-') << "\n";
        std::cout << "Total despachado en este lote: " << despachados << " pedido(s).\n";
    }

    return despachados;
}

void SistemaLogistico::consultarOrdenadosPorFecha(int limite) const {
    arbolCronologico.mostrarInOrder(limite);
}

bool SistemaLogistico::consultarPorRangoFechas(const std::string& fechaInicio,
                                               const std::string& fechaFin,
                                               int& conteoPedidos,
                                               double& montoTotal) const {
    std::string ini = recortarEspacios(fechaInicio);
    std::string fin = recortarEspacios(fechaFin);

    if (!validarFecha(ini) || !validarFecha(fin)) {
        return false;
    }
    if (ini > fin) {
        return false; // La fecha inicial no puede ser posterior a la final
    }

    arbolCronologico.consultarPorRangoFechas(ini, fin, conteoPedidos, montoTotal);
    return true;
}

bool SistemaLogistico::actualizarEstadoPedido(const std::string& idPedido,
                                              const std::string& nuevoEstado,
                                              std::string& mensaje) {
    std::string id = recortarEspacios(idPedido);
    std::string est = recortarEspacios(nuevoEstado);

    if (!validarEstado(est)) {
        mensaje = "Estado no valido. Opciones: Pendiente, Iniciado, Enviado, EnTransito, En camino, Entregado, Cancelado.";
        return false;
    }

    // Si el nuevo estado es Cancelado, delegamos a la cancelación completa
    if (est == "Cancelado") {
        return cancelarYEliminarPedido(id, mensaje);
    }

    Pedido* pedido = tablaHash.buscar(id);
    if (pedido == nullptr) {
        mensaje = "No se encontro ningun pedido con ID '" + id + "'.";
        return false;
    }

    std::string estadoAnterior = pedido->getEstado();
    if (estadoAnterior == est) {
        mensaje = "El pedido ya se encuentra en estado '" + est + "'.";
        return true;
    }

    // Sincronización con el Min-Heap:
    // Si estaba Pendiente y pasa a otro estado (EnTransito, Enviado, Entregado, etc.),
    // se extrae del Min-Heap para que nunca pueda ser despachado nuevamente.
    if (estadoAnterior == "Pendiente" && est != "Pendiente") {
        int idx = pedido->getIndiceHeap();
        if (idx != -1) {
            heapDespacho.eliminarEnIndice(idx);
        }
    }

    pedido->setEstado(est);

    // Si vuelve a estado Pendiente desde un estado no despachado/no cancelado, reinsertar en Heap
    if (estadoAnterior != "Pendiente" && est == "Pendiente") {
        heapDespacho.insertar(pedido);
    }

    mensaje = "Estado del pedido '" + id + "' actualizado de '" + estadoAnterior + "' a '" + est + "'.";
    return true;
}

bool SistemaLogistico::cancelarYEliminarPedido(const std::string& idPedido, std::string& mensaje) {
    std::string id = recortarEspacios(idPedido);
    Pedido* pedido = tablaHash.buscar(id);

    if (pedido == nullptr) {
        mensaje = "No existe un pedido activo con ID '" + id + "'.";
        return false;
    }

    // 1. Marcar como cancelado y eliminar del Min-Heap en O(log n) si estaba pendiente
    pedido->setEstado("Cancelado");
    int idxHeap = pedido->getIndiceHeap();
    if (idxHeap != -1) {
        heapDespacho.eliminarEnIndice(idxHeap);
    }

    // 2. Eliminar del Árbol AVL en O(log n)
    arbolCronologico.eliminar(pedido);

    // 3. Eliminar de la Tabla Hash en O(1) promedio
    tablaHash.eliminar(id);

    // 4. Actualizar contadores globales O(1)
    totalPedidos--;
    montoTotalAcumulado -= pedido->getMontoDolares();
    if (montoTotalAcumulado < 0.0) montoTotalAcumulado = 0.0;

    // 5. Liberar memoria del objeto Pedido de forma segura (sin dangling pointers)
    delete pedido;

    mensaje = "Pedido '" + id + "' cancelado y eliminado de la Tabla Hash, Min-Heap y Arbol AVL.";
    return true;
}

void SistemaLogistico::mostrarEstadisticasGenerales() const {
    std::cout << "\n======================================================\n";
    std::cout << "         ESTADISTICAS GENERALES DEL SISTEMA (O(1))    \n";
    std::cout << "======================================================\n";
    std::cout << "  Total de pedidos activos en sistema : " << totalPedidos << "\n";
    std::cout << "  Pedidos pendientes de despacho      : " << heapDespacho.obtenerTamano() << "\n";
    std::cout << "  Pedidos fuera de cola de despacho  : " << (totalPedidos - heapDespacho.obtenerTamano()) << "\n";
    std::cout << "  Monto total acumulado (USD)         : $" << std::fixed << std::setprecision(2) << montoTotalAcumulado << "\n";
    std::cout << "  ------------------------------------------------------\n";
    std::cout << "  [Diagnostico Estructuras]\n";
    std::cout << "  Tabla Hash -> Capacidad: " << tablaHash.obtenerCapacidad()
            << " | Factor carga: " << std::setprecision(3)
            << tablaHash.obtenerFactorCarga()
            << " | Colisiones: " << tablaHash.obtenerColisionesTotales()
            << "\n";
    std::cout << "  Arbol AVL  -> Nodos: " << arbolCronologico.obtenerCantidadNodos()
              << " | Altura: " << arbolCronologico.obtenerAlturaArbol() << "\n";
    std::cout << "  Min-Heap   -> Elementos pendientes: " << heapDespacho.obtenerTamano() << "\n";
    std::cout << "======================================================\n";
}

void SistemaLogistico::ejecutarPruebasRendimiento(const std::string& rutaCSV) {
    std::cout << "\n==================================================\n";
    std::cout << "   SUITE DE PRUEBAS DE RENDIMIENTO Y SUSTENTACION   \n";
    std::cout << "====================================================\n";

    int omitidos = 0;
    double tCarga = 0.0;

    // 1. Prueba de carga masiva (>= 10,000 pedidos)
    int cargados = cargarDesdeCSV(rutaCSV, omitidos, tCarga, true);
    if (cargados < 0) {
        std::cout << "[ERROR] No se pudo abrir el archivo: " << rutaCSV << "\n";
        std::cout << "Asegurese de ejecutar el programa desde la carpeta raiz del proyecto.\n";
        return;
    }
    std::cout << "[1] Carga Masiva (" << cargados << " pedidos, omitidos: " << omitidos << "): "
              << std::fixed << std::setprecision(4) << tCarga << " ms\n";

    // 2. Prueba de Búsqueda (Hash): inicio, final e inexistente
    auto t0 = std::chrono::high_resolution_clock::now();
    Pedido* pInicio = buscarPedidoPorID("PD-000001");
    auto t1 = std::chrono::high_resolution_clock::now();
    double msInicio = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    Pedido* pFinal = buscarPedidoPorID("PD-010500");
    t1 = std::chrono::high_resolution_clock::now();
    double msFinal = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    Pedido* pInexistente = buscarPedidoPorID("PD-999999");
    t1 = std::chrono::high_resolution_clock::now();
    double msInexistente = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "[2] Busqueda Hash:\n"
              << "    - Inicio (PD-000001)     : " << (pInicio ? "ENCONTRADO" : "NO") << " en " << msInicio << " ms\n"
              << "    - Final  (PD-010500)     : " << (pFinal ? "ENCONTRADO" : "NO") << " en " << msFinal << " ms\n"
              << "    - Inexistente (PD-999999): " << (pInexistente ? "ENCONTRADO" : "NO EXISTE") << " en " << msInexistente << " ms\n";

    // 3. Prueba de Despacho (Min-Heap): 10 pedidos más urgentes (y prueba de 100 pedidos para tabla IEEE)
    t0 = std::chrono::high_resolution_clock::now();
    int despachados10 = despacharLoteUrgente(10, false);
    t1 = std::chrono::high_resolution_clock::now();
    double msDespacho10 = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    int despachados100 = despacharLoteUrgente(100, false);
    t1 = std::chrono::high_resolution_clock::now();
    double msDespacho100 = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "[3] Despacho Min-Heap:\n"
              << "    - Lote de 10 urgentes    : " << despachados10 << " despachados en " << msDespacho10 << " ms\n"
              << "    - Lote de 100 urgentes   : " << despachados100 << " despachados en " << msDespacho100 << " ms\n";

    // 4. Prueba de Consulta por Rango de Fechas (AVL): 1 semana, 1 mes, y sin resultados
    int cSemana = 0, cMes = 0, cVacio = 0;
    double mSemana = 0.0, mMes = 0.0, mVacio = 0.0;

    t0 = std::chrono::high_resolution_clock::now();
    consultarPorRangoFechas("2026-06-01", "2026-06-07", cSemana, mSemana);
    t1 = std::chrono::high_resolution_clock::now();
    double msSemana = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    consultarPorRangoFechas("2026-06-01", "2026-06-30", cMes, mMes);
    t1 = std::chrono::high_resolution_clock::now();
    double msMes = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    consultarPorRangoFechas("2028-01-01", "2028-01-31", cVacio, mVacio);
    t1 = std::chrono::high_resolution_clock::now();
    double msVacio = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "[4] Consulta por Rango AVL:\n"
              << "    - 1 Semana (2026-06-01 a 2026-06-07): " << cSemana << " pedidos | $" << std::setprecision(2) << mSemana << " | " << std::setprecision(4) << msSemana << " ms\n"
              << "    - 1 Mes    (2026-06-01 a 2026-06-30): " << cMes << " pedidos | $" << std::setprecision(2) << mMes << " | " << std::setprecision(4) << msMes << " ms\n"
              << "    - Sin resultados (2028-01-01..31)   : " << cVacio << " pedidos | $" << std::setprecision(2) << mVacio << " | " << std::setprecision(4) << msVacio << " ms\n";

    // 5. Prueba de inserción individual
    std::string msgErr;
    t0 = std::chrono::high_resolution_clock::now();
    bool insOk = registrarPedido("PD-NEW-001", "71234567", "Same Day", 10, 550.00, "2026-06-15", "12:00:00", "Pendiente", msgErr);
    t1 = std::chrono::high_resolution_clock::now();
    double msInsercion = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "[5] Insercion Individual (PD-NEW-001): " << (insOk ? "OK" : msgErr) << " en " << msInsercion << " ms\n";

    // 6. Prueba de actualización (10 pedidos)
    int actualizados = 0;
    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 500; i < 510; ++i) {
        std::ostringstream oss;
        oss << "PD-" << std::setw(6) << std::setfill('0') << i;
        std::string m;
        if (actualizarEstadoPedido(oss.str(), "Entregado", m)) {
            actualizados++;
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    double msActualizacion = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "[6] Actualizacion de " << actualizados << " pedidos: " << msActualizacion << " ms\n";

    // 7. Prueba de cancelación/eliminación (10 pedidos)
    int eliminados = 0;
    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 600; i < 610; ++i) {
        std::ostringstream oss;
        oss << "PD-" << std::setw(6) << std::setfill('0') << i;
        std::string m;
        if (cancelarYEliminarPedido(oss.str(), m)) {
            eliminados++;
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    double msEliminacion = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "[7] Cancelacion y Eliminacion de " << eliminados << " pedidos: " << msEliminacion << " ms\n";


        // Verificaciones posteriores, fuera de los tiempos medidos.
    int estadosCorrectos = 0;

    for (int i = 500; i < 510; ++i) {
        std::ostringstream id;
        id << "PD-" << std::setw(6) << std::setfill('0') << i;

        Pedido* pedido = buscarPedidoPorID(id.str());

        if (pedido != nullptr && pedido->getEstado() == "Entregado") {
            estadosCorrectos++;
        } else {
            std::cout << "[FALLO] Estado incorrecto o pedido ausente: "
                    << id.str() << "\n";
        }
    }

    int ausentesEnHash = 0;

    for (int i = 600; i < 610; ++i) {
        std::ostringstream id;
        id << "PD-" << std::setw(6) << std::setfill('0') << i;

        if (buscarPedidoPorID(id.str()) == nullptr) {
            ausentesEnHash++;
        } else {
            std::cout << "[FALLO] El pedido sigue en la tabla hash: "
                    << id.str() << "\n";
        }
    }

    std::cout << "\n[Verificacion posterior]\n";
    std::cout << "  Pedidos con estado Entregado: "
            << estadosCorrectos << "/10\n";
    std::cout << "  Pedidos eliminados ausentes en Hash: "
            << ausentesEnHash << "/10\n";

    bool actualizacionCorrecta =
        (actualizados == 10 && estadosCorrectos == 10);

    bool eliminacionCorrecta =
        (eliminados == 10 && ausentesEnHash == 10);

    std::cout << "  Actualizacion: "
            << (actualizacionCorrecta ? "OK" : "FALLO") << "\n";
    std::cout << "  Eliminacion en Hash: "
            << (eliminacionCorrecta ? "OK" : "FALLO") << "\n";
    // 8. Prueba de Consulta Ordenada In-Order (30 primeros)
    std::cout << "\n[8] Consulta Ordenada In-Order (Top 30 cronologico):\n";
    t0 = std::chrono::high_resolution_clock::now();
    consultarOrdenadosPorFecha(30);
    t1 = std::chrono::high_resolution_clock::now();
    double msInOrder = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "Tiempo Consulta Ordenada (30 registros): " << msInOrder << " ms\n";

    mostrarEstadisticasGenerales();
}
