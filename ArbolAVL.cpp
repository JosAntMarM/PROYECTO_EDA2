#include "ArbolAVL.h"
#include <iostream>
#include <iomanip>

ArbolAVL::ArbolAVL() : raiz(nullptr), cantidadNodos(0) {}

ArbolAVL::~ArbolAVL() {
    limpiar();
}

int ArbolAVL::obtenerAltura(NodoAVL* nodo) const {
    return (nodo != nullptr) ? nodo->altura : 0;
}

int ArbolAVL::obtenerBalance(NodoAVL* nodo) const {
    if (nodo == nullptr) return 0;
    return obtenerAltura(nodo->izquierdo) - obtenerAltura(nodo->derecho);
}

int ArbolAVL::maximo(int a, int b) const {
    return (a > b) ? a : b;
}

void ArbolAVL::actualizarAltura(NodoAVL* nodo) {
    if (nodo != nullptr) {
        nodo->altura = 1 + maximo(obtenerAltura(nodo->izquierdo), obtenerAltura(nodo->derecho));
    }
}

// 1. Rotación Simple a la Derecha (Caso Izquierda-Izquierda)
NodoAVL* ArbolAVL::rotacionDerecha(NodoAVL* y) {
    NodoAVL* x = y->izquierdo;
    NodoAVL* T2 = x->derecho;

    x->derecho = y;
    y->izquierdo = T2;

    actualizarAltura(y);
    actualizarAltura(x);

    return x;
}

// 2. Rotación Simple a la Izquierda (Caso Derecha-Derecha)
NodoAVL* ArbolAVL::rotacionIzquierda(NodoAVL* x) {
    NodoAVL* y = x->derecho;
    NodoAVL* T2 = y->izquierdo;

    y->izquierdo = x;
    x->derecho = T2;

    actualizarAltura(x);
    actualizarAltura(y);

    return y;
}

// 3. Rotación Doble Izquierda-Derecha (Caso Izquierda-Derecha)
NodoAVL* ArbolAVL::rotacionIzquierdaDerecha(NodoAVL* z) {
    z->izquierdo = rotacionIzquierda(z->izquierdo);
    return rotacionDerecha(z);
}

// 4. Rotación Doble Derecha-Izquierda (Caso Derecha-Izquierda)
NodoAVL* ArbolAVL::rotacionDerechaIzquierda(NodoAVL* z) {
    z->derecho = rotacionDerecha(z->derecho);
    return rotacionIzquierda(z);
}

NodoAVL* ArbolAVL::insertarRecursivo(NodoAVL* nodo, Pedido* pedido, bool& insertado) {
    if (nodo == nullptr) {
        insertado = true;
        return new NodoAVL(pedido);
    }

    std::string claveNueva = pedido->getClaveCronologica();
    std::string claveActual = nodo->pedido->getClaveCronologica();

    if (claveNueva < claveActual) {
        nodo->izquierdo = insertarRecursivo(nodo->izquierdo, pedido, insertado);
    } else if (claveNueva > claveActual) {
        nodo->derecho = insertarRecursivo(nodo->derecho, pedido, insertado);
    } else {
        // Clave duplicada (no debería ocurrir si TablaHash valida primero)
        insertado = false;
        return nodo;
    }

    actualizarAltura(nodo);
    int balance = obtenerBalance(nodo);

    // Caso LL
    if (balance > 1 && claveNueva < nodo->izquierdo->pedido->getClaveCronologica()) {
        return rotacionDerecha(nodo);
    }
    // Caso RR
    if (balance < -1 && claveNueva > nodo->derecho->pedido->getClaveCronologica()) {
        return rotacionIzquierda(nodo);
    }
    // Caso LR
    if (balance > 1 && claveNueva > nodo->izquierdo->pedido->getClaveCronologica()) {
        return rotacionIzquierdaDerecha(nodo);
    }
    // Caso RL
    if (balance < -1 && claveNueva < nodo->derecho->pedido->getClaveCronologica()) {
        return rotacionDerechaIzquierda(nodo);
    }

    return nodo;
}

bool ArbolAVL::insertar(Pedido* pedido) {
    if (pedido == nullptr) return false;
    bool insertado = false;
    raiz = insertarRecursivo(raiz, pedido, insertado);
    if (insertado) {
        cantidadNodos++;
    }
    return insertado;
}

NodoAVL* ArbolAVL::obtenerMinimoNodo(NodoAVL* nodo) const {
    NodoAVL* actual = nodo;
    while (actual != nullptr && actual->izquierdo != nullptr) {
        actual = actual->izquierdo;
    }
    return actual;
}

NodoAVL* ArbolAVL::eliminarRecursivo(NodoAVL* nodo, const std::string& claveCronologica, bool& eliminado) {
    if (nodo == nullptr) {
        return nullptr;
    }

    std::string claveActual = nodo->pedido->getClaveCronologica();

    if (claveCronologica < claveActual) {
        nodo->izquierdo = eliminarRecursivo(nodo->izquierdo, claveCronologica, eliminado);
    } else if (claveCronologica > claveActual) {
        nodo->derecho = eliminarRecursivo(nodo->derecho, claveCronologica, eliminado);
    } else {
        eliminado = true;
        // Nodo con 0 o 1 hijo
        if (nodo->izquierdo == nullptr || nodo->derecho == nullptr) {
            NodoAVL* temp = (nodo->izquierdo != nullptr) ? nodo->izquierdo : nodo->derecho;
            if (temp == nullptr) {
                delete nodo;
                return nullptr;
            } else {
                NodoAVL* aBorrar = nodo;
                nodo = temp;
                delete aBorrar;
            }
        } else {
            // Nodo con 2 hijos: obtener sucesor In-Order
            NodoAVL* sucesor = obtenerMinimoNodo(nodo->derecho);
            nodo->pedido = sucesor->pedido;
            bool dummy = false;
            nodo->derecho = eliminarRecursivo(nodo->derecho, sucesor->pedido->getClaveCronologica(), dummy);
        }
    }

    actualizarAltura(nodo);
    int balance = obtenerBalance(nodo);

    // Rebalanceo tras eliminación
    // Caso LL
    if (balance > 1 && obtenerBalance(nodo->izquierdo) >= 0) {
        return rotacionDerecha(nodo);
    }
    // Caso LR
    if (balance > 1 && obtenerBalance(nodo->izquierdo) < 0) {
        return rotacionIzquierdaDerecha(nodo);
    }
    // Caso RR
    if (balance < -1 && obtenerBalance(nodo->derecho) <= 0) {
        return rotacionIzquierda(nodo);
    }
    // Caso RL
    if (balance < -1 && obtenerBalance(nodo->derecho) > 0) {
        return rotacionDerechaIzquierda(nodo);
    }

    return nodo;
}

bool ArbolAVL::eliminar(const Pedido* pedido) {
    if (pedido == nullptr) return false;
    bool eliminado = false;
    raiz = eliminarRecursivo(raiz, pedido->getClaveCronologica(), eliminado);
    if (eliminado) {
        cantidadNodos--;
    }
    return eliminado;
}

void ArbolAVL::inOrderLimitadoRecursivo(NodoAVL* nodo, int& mostrados, int limite) const {
    if (nodo == nullptr || mostrados >= limite) {
        return;
    }

    inOrderLimitadoRecursivo(nodo->izquierdo, mostrados, limite);

    if (mostrados < limite) {
        nodo->pedido->mostrarFilaTabla();
        mostrados++;
    }

    if (mostrados < limite) {
        inOrderLimitadoRecursivo(nodo->derecho, mostrados, limite);
    }
}

void ArbolAVL::mostrarInOrder(int limite) const {
    if (raiz == nullptr) {
        std::cout << "  [Aviso] El arbol AVL se encuentra vacio.\n";
        return;
    }

    std::cout << std::left
              << std::setw(12) << "ID Pedido"
              << std::setw(11) << "DNI"
              << std::setw(11) << "Envio"
              << std::setw(6)  << "Prio"
              << std::right << std::setw(10) << "Monto" << "  "
              << std::left << std::setw(12) << "Fecha"
              << std::setw(10) << "Hora"
              << std::setw(12) << "Estado" << "\n";
    std::cout << std::string(86, '-') << "\n";

    int mostrados = 0;
    inOrderLimitadoRecursivo(raiz, mostrados, limite);
    std::cout << std::string(86, '-') << "\n";
    std::cout << "  Mostrando " << mostrados << " de " << cantidadNodos << " pedidos ordenados cronologicamente.\n";
}

void ArbolAVL::consultarRangoRecursivo(NodoAVL* nodo,
                                       const std::string& fechaInicio,
                                       const std::string& fechaFin,
                                       int& conteoPedidos,
                                       double& montoTotal) const {
    if (nodo == nullptr) {
        return;
    }

    const std::string fechaActual = nodo->pedido->getFecha();

    // Poda izquierda: solo explorar si pueden existir fechas >= fechaInicio
    if (fechaActual >= fechaInicio) {
        consultarRangoRecursivo(nodo->izquierdo, fechaInicio, fechaFin, conteoPedidos, montoTotal);
    }

    // Procesar nodo actual si cae dentro del rango cerrado [fechaInicio, fechaFin]
    if (fechaActual >= fechaInicio && fechaActual <= fechaFin) {
        conteoPedidos++;
        montoTotal += nodo->pedido->getMontoDolares();
    }

    // Poda derecha: solo explorar si pueden existir fechas <= fechaFin
    if (fechaActual <= fechaFin) {
        consultarRangoRecursivo(nodo->derecho, fechaInicio, fechaFin, conteoPedidos, montoTotal);
    }
}

void ArbolAVL::consultarPorRangoFechas(const std::string& fechaInicio,
                                       const std::string& fechaFin,
                                       int& conteoPedidos,
                                       double& montoTotal) const {
    conteoPedidos = 0;
    montoTotal = 0.0;
    consultarRangoRecursivo(raiz, fechaInicio, fechaFin, conteoPedidos, montoTotal);
}

void ArbolAVL::destruirRecursivo(NodoAVL* nodo) {
    if (nodo == nullptr) return;
    destruirRecursivo(nodo->izquierdo);
    destruirRecursivo(nodo->derecho);
    delete nodo;
}

void ArbolAVL::limpiar() {
    destruirRecursivo(raiz);
    raiz = nullptr;
    cantidadNodos = 0;
}

int ArbolAVL::obtenerCantidadNodos() const {
    return cantidadNodos;
}

int ArbolAVL::obtenerAlturaArbol() const {
    return obtenerAltura(raiz);
}
