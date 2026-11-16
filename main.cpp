#include "SistemaLogistico.h"
#include <iostream>
#include <string>
#include <limits>
#include <chrono>
#include <iomanip>

void limpiarBufferEntrada() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void mostrarMenu() {
    std::cout << "\n==========================================================\n";
    std::cout << "  SISTEMA MASIVO DE GESTION LOGISTICA Y DESPACHO (EDA II) \n";
    std::cout << "==========================================================\n";
    std::cout << "  1. Cargar pedidos desde archivo CSV\n";
    std::cout << "  2. Registrar nuevo pedido\n";
    std::cout << "  3. Buscar pedido por ID\n";
    std::cout << "  4. Despachar siguiente lote de pedidos\n";
    std::cout << "  5. Consultar pedidos ordenados por fecha\n";
    std::cout << "  6. Consultar pedidos por rango de fechas\n";
    std::cout << "  7. Actualizar estado de pedido\n";
    std::cout << "  8. Cancelar pedido\n";
    std::cout << "  9. Mostrar estadisticas generales\n";
    std::cout << " 10. Ejecutar suite de pruebas y rendimiento\n";
    std::cout << " 11. Salir\n";
    std::cout << "==========================================================\n";
    std::cout << "Seleccione una opcion: ";
}

int main() {
    SistemaLogistico sistema;
    int opcion = 0;

    do {
        mostrarMenu();
        if (!(std::cin >> opcion)) {
            std::cout << "[Error] Entrada invalida. Ingrese un numero del 1 al 11.\n";
            limpiarBufferEntrada();
            continue;
        }
        limpiarBufferEntrada();

        switch (opcion) {
            case 1: {
                std::cout << "\n--- CARGA MASIVA DESDE ARCHIVO CSV ---\n";
                std::cout << "Archivos sugeridos:\n";
                std::cout << "  - datos/pedidos_pequeno.csv\n";
                std::cout << "  - datos/pedidos_masivo.csv\n";
                std::cout << "Ingrese la ruta del archivo CSV: ";
                std::string ruta;
                std::getline(std::cin, ruta);

                std::cout << "Desea limpiar los datos actuales antes de cargar? (1 = Si, 0 = No): ";
                int limpiar = 0;
                if (!(std::cin >> limpiar)) {
                    limpiar = 0;
                }
                limpiarBufferEntrada();

                int omitidos = 0;
                double tiempoMs = 0.0;
                int cargados = sistema.cargarDesdeCSV(ruta, omitidos, tiempoMs, limpiar == 1);

                if (cargados < 0) {
                    std::cout << "[Error] No se pudo abrir el archivo '" << ruta << "'. Verifique la ruta.\n";
                } else {
                    std::cout << "[Exito] Se cargaron " << cargados << " pedidos correctamente en "
                              << std::fixed << std::setprecision(4) << tiempoMs << " ms.\n";
                    if (omitidos > 0) {
                        std::cout << "[Aviso] Registros omitidos (duplicados o formato invalido): " << omitidos << "\n";
                    }
                }
                break;
            }

            case 2: {
                std::cout << "\n--- REGISTRAR NUEVO PEDIDO ---\n";
                std::string id, dni, tipo, fecha, hora, estado;
                int prioridad = 1;
                double monto = 0.0;

                std::cout << "ID Pedido (ej. PD-010501): ";
                std::getline(std::cin, id);

                std::cout << "DNI Cliente (8 digitos): ";
                std::getline(std::cin, dni);

                std::cout << "Tipo de Envio (Normal / Express / Same Day): ";
                std::getline(std::cin, tipo);

                std::cout << "Nivel de Prioridad (1=Normal, 5=Express, 10=Same Day): ";
                if (!(std::cin >> prioridad)) {
                    std::cout << "[Error] Prioridad invalida.\n";
                    limpiarBufferEntrada();
                    break;
                }

                std::cout << "Monto en Dolares (USD): ";
                if (!(std::cin >> monto)) {
                    std::cout << "[Error] Monto invalido.\n";
                    limpiarBufferEntrada();
                    break;
                }

                std::string restoMonto;
                std::getline(std::cin, restoMonto);

                if (restoMonto.find_first_not_of(" \t\r") != std::string::npos) {
                    std::cout << "[Error] El monto no debe contener letras ni otros caracteres.\n";
                    break;
                }

                std::cout << "Fecha (YYYY-MM-DD): ";
                std::getline(std::cin, fecha);

                std::cout << "Hora (HH:MM:SS): ";
                std::getline(std::cin, hora);

                std::cout << "Estado inicial (Pendiente = listo para despacho / Iniciado = en preparacion): ";
                std::getline(std::cin, estado);

                std::string error;
                auto inicio = std::chrono::high_resolution_clock::now();
                bool ok = sistema.registrarPedido(id, dni, tipo, prioridad, monto, fecha, hora, estado, error);
                auto fin = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(fin - inicio).count();

                if (ok) {
                    std::cout << "[Exito] Pedido registrado correctamente en "
                              << std::fixed << std::setprecision(4) << ms << " ms.\n";
                } else {
                    std::cout << "[Error] " << error << "\n";
                }
                break;
            }

            case 3: {
                std::cout << "\n--- BUSCAR PEDIDO POR ID (TABLA HASH O(1)) ---\n";
                std::cout << "Ingrese el ID del pedido: ";
                std::string id;
                std::getline(std::cin, id);

                auto inicio = std::chrono::high_resolution_clock::now();
                Pedido* encontrado = sistema.buscarPedidoPorID(id);
                auto fin = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(fin - inicio).count();

                if (encontrado != nullptr) {
                    std::cout << "[Encontrado en " << std::fixed << std::setprecision(4) << ms << " ms]\n";
                    encontrado->mostrarDetalle();
                } else {
                    std::cout << "[No encontrado] No existe ningun pedido con ID '" << id
                              << "' (Tiempo: " << std::fixed << std::setprecision(4) << ms << " ms).\n";
                }
                break;
            }

            case 4: {
                std::cout << "\n--- DESPACHAR SIGUIENTE LOTE DE PEDIDOS (MIN-HEAP) ---\n";
                std::cout << "Cantidad de pedidos urgentes a despachar: ";
                int cantidad = 0;
                if (!(std::cin >> cantidad) || cantidad <= 0) {
                    std::cout << "[Error] Ingrese una cantidad entera positiva.\n";
                    limpiarBufferEntrada();
                    break;
                }
                limpiarBufferEntrada();

                auto inicio = std::chrono::high_resolution_clock::now();
                int despachados = sistema.despacharLoteUrgente(cantidad, true);
                auto fin = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(fin - inicio).count();

                if (despachados == 0) {
                    std::cout << "[Aviso] No hay pedidos pendientes en la cola de despacho.\n";
                } else {
                    std::cout << "Tiempo de despacho: " << std::fixed << std::setprecision(4) << ms << " ms.\n";
                }
                break;
            }

            case 5: {
                std::cout << "\n--- CONSULTA CRONOLOGICA (PRIMEROS 30) ---\n";
                auto inicio = std::chrono::high_resolution_clock::now();
                sistema.consultarOrdenadosPorFecha(30);
                auto fin = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(fin - inicio).count();
                std::cout << "Tiempo de ejecucion: " << std::fixed << std::setprecision(4) << ms << " ms.\n";
                break;
            }

            case 6: {
                std::cout << "\n--- CONSULTA POR RANGO DE FECHAS (ARBOL AVL) ---\n";
                std::string fInicio, fFin;
                std::cout << "Ingrese fecha inicial (YYYY-MM-DD): ";
                std::getline(std::cin, fInicio);
                std::cout << "Ingrese fecha final   (YYYY-MM-DD): ";
                std::getline(std::cin, fFin);

                int conteo = 0;
                double montoTotal = 0.0;

                auto inicio = std::chrono::high_resolution_clock::now();
                bool valido = sistema.consultarPorRangoFechas(fInicio, fFin, conteo, montoTotal);
                auto fin = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(fin - inicio).count();

                if (!valido) {
                    std::cout << "[Error] Rango de fechas invalido. Verifique formato YYYY-MM-DD y que fecha inicial <= fecha final.\n";
                } else {
                    std::cout << "\n  Resultados del periodo [" << fInicio << " al " << fFin << "]:\n";
                    std::cout << "  - Cantidad de pedidos : " << conteo << "\n";
                    std::cout << "  - Monto total vendido : $" << std::fixed << std::setprecision(2) << montoTotal << "\n";
                    std::cout << "  - Tiempo de consulta  : " << std::fixed << std::setprecision(4) << ms << " ms\n";
                }
                break;
            }

            case 7: {
                std::cout << "\n--- ACTUALIZAR ESTADO DE PEDIDO ---\n";
                std::string id, nuevoEstado, mensaje;
                std::cout << "Ingrese el ID del pedido: ";
                std::getline(std::cin, id);
                std::cout << "Estados disponibles: Pendiente, Iniciado, Enviado, EnTransito, En camino, Entregado, Cancelado\n";
                std::cout << "Ingrese el nuevo estado: ";
                std::getline(std::cin, nuevoEstado);

                auto inicio = std::chrono::high_resolution_clock::now();
                bool ok = sistema.actualizarEstadoPedido(id, nuevoEstado, mensaje);
                auto fin = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(fin - inicio).count();

                std::cout << (ok ? "[Exito] " : "[Error] ") << mensaje
                          << " (" << std::fixed << std::setprecision(4) << ms << " ms)\n";
                break;
            }

            case 8: {
                std::cout << "\n--- CANCELAR Y ELIMINAR PEDIDO ---\n";
                std::string id, mensaje;
                std::cout << "Ingrese el ID del pedido a cancelar: ";
                std::getline(std::cin, id);

                auto inicio = std::chrono::high_resolution_clock::now();
                bool ok = sistema.cancelarYEliminarPedido(id, mensaje);
                auto fin = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(fin - inicio).count();

                std::cout << (ok ? "[Exito] " : "[Error] ") << mensaje
                          << " (" << std::fixed << std::setprecision(4) << ms << " ms)\n";
                break;
            }

            case 9: {
                sistema.mostrarEstadisticasGenerales();
                break;
            }

            case 10: {
                sistema.ejecutarPruebasRendimiento("datos/pedidos_masivo.csv");
                break;
            }

            case 11:
            case 0: {
                std::cout << "Liberando memoria y cerrando el Sistema Logistico. Hasta pronto!\n";
                opcion = 11;
                break;
            }

            default:
                std::cout << "[Aviso] Opcion no reconocida. Seleccione un numero del 1 al 11.\n";
                break;
        }
    } while (opcion != 11);

    return 0;
}
