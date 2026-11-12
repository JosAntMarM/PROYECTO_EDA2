#include "Pedido.h"
#include <iomanip>

Pedido::Pedido()
    : idPedido(""), dniCliente(""), tipoEnvio("Normal"),
      nivelPrioridad(1), montoDolares(0.0),
      fecha("1970-01-01"), hora("00:00:00"),
      estado("Pendiente"), indiceHeap(-1) {}

Pedido::Pedido(const std::string& idPedido,
               const std::string& dniCliente,
               const std::string& tipoEnvio,
               int nivelPrioridad,
               double montoDolares,
               const std::string& fecha,
               const std::string& hora,
               const std::string& estado)
    : idPedido(idPedido), dniCliente(dniCliente), tipoEnvio(tipoEnvio),
      nivelPrioridad(nivelPrioridad), montoDolares(montoDolares),
      fecha(fecha), hora(hora), estado(estado), indiceHeap(-1) {}

std::string Pedido::getIdPedido() const { return idPedido; }
std::string Pedido::getDniCliente() const { return dniCliente; }
std::string Pedido::getTipoEnvio() const { return tipoEnvio; }
int Pedido::getNivelPrioridad() const { return nivelPrioridad; }
double Pedido::getMontoDolares() const { return montoDolares; }
std::string Pedido::getFecha() const { return fecha; }
std::string Pedido::getHora() const { return hora; }
std::string Pedido::getEstado() const { return estado; }
int Pedido::getIndiceHeap() const { return indiceHeap; }

void Pedido::setEstado(const std::string& nuevoEstado) {
    estado = nuevoEstado;
}

void Pedido::setIndiceHeap(int indice) {
    indiceHeap = indice;
}

std::string Pedido::getClaveCronologica() const {
    return fecha + "|" + hora + "|" + idPedido;
}

void Pedido::mostrarDetalle() const {
    std::cout << "  --------------------------------------------------\n";
    std::cout << "  ID Pedido       : " << idPedido << "\n";
    std::cout << "  DNI Cliente     : " << dniCliente << "\n";
    std::cout << "  Tipo de Envio   : " << tipoEnvio << "\n";
    std::cout << "  Nivel Prioridad : " << nivelPrioridad << "\n";
    std::cout << "  Monto (USD)     : $" << std::fixed << std::setprecision(2) << montoDolares << "\n";
    std::cout << "  Fecha y Hora    : " << fecha << " " << hora << "\n";
    std::cout << "  Estado Actual   : " << estado << "\n";
    std::cout << "  --------------------------------------------------\n";
}

void Pedido::mostrarFilaTabla() const {
    std::cout << std::left
              << std::setw(12) << idPedido
              << std::setw(11) << dniCliente
              << std::setw(11) << tipoEnvio
              << std::setw(6)  << nivelPrioridad
              << "$" << std::right << std::setw(9) << std::fixed << std::setprecision(2) << montoDolares << "  "
              << std::left << std::setw(12) << fecha
              << std::setw(10) << hora
              << std::setw(12) << estado << "\n";
}
