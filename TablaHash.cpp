#include "TablaHash.h"

TablaHash::TablaHash(size_t capacidadInicial)
    : capacidad(siguientePrimo(capacidadInicial)),
      cantidadElementos(0),
      colisionesTotales(0) {
    buckets = new NodoHash*[capacidad];
    for (size_t i = 0; i < capacidad; ++i) {
        buckets[i] = nullptr;
    }
}

TablaHash::~TablaHash() {
    for (size_t i = 0; i < capacidad; ++i) {
        NodoHash* actual = buckets[i];
        while (actual != nullptr) {
            NodoHash* siguiente = actual->siguiente;
            delete actual;
            actual = siguiente;
        }
    }
    delete[] buckets;
    buckets = nullptr;
}

size_t TablaHash::calcularHash(const std::string& clave) const {
    // Hash polinomial rolling con primo 131 (alta dispersión en prefijos similares como "PD-")
    size_t hash = 5381;
    for (size_t i = 0; i < clave.length(); ++i) {
        hash = (hash * 131) + static_cast<unsigned char>(clave[i]);
    }
    return hash % capacidad;
}

bool TablaHash::esPrimo(size_t n) const {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (size_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

size_t TablaHash::siguientePrimo(size_t n) const {
    if (n <= 2) return 2;
    size_t candidato = (n % 2 == 0) ? n + 1 : n;
    while (!esPrimo(candidato)) {
        candidato += 2;
    }
    return candidato;
}

void TablaHash::rehash() {
    size_t antiguaCapacidad = capacidad;
    NodoHash** antiguosBuckets = buckets;

    capacidad = siguientePrimo(antiguaCapacidad * 2);
    buckets = new NodoHash*[capacidad];
    for (size_t i = 0; i < capacidad; ++i) {
        buckets[i] = nullptr;
    }

    colisionesTotales = 0;

    for (size_t i = 0; i < antiguaCapacidad; ++i) {
        NodoHash* actual = antiguosBuckets[i];
        while (actual != nullptr) {
            NodoHash* siguiente = actual->siguiente;
            size_t nuevoIndice = calcularHash(actual->pedido->getIdPedido());
            if (buckets[nuevoIndice] != nullptr) {
                colisionesTotales++;
            }
            actual->siguiente = buckets[nuevoIndice];
            buckets[nuevoIndice] = actual;
            actual = siguiente;
        }
    }

    delete[] antiguosBuckets;
}

bool TablaHash::insertar(Pedido* pedido) {
    if (pedido == nullptr) return false;

    // Verificar duplicados en O(1) promedio
    if (buscar(pedido->getIdPedido()) != nullptr) {
        return false;
    }

    // Rehash si el factor de carga excede 0.75
    if (static_cast<double>(cantidadElementos + 1) / static_cast<double>(capacidad) > 0.75) {
        rehash();
    }

    size_t indice = calcularHash(pedido->getIdPedido());
    if (buckets[indice] != nullptr) {
        colisionesTotales++;
    }

    NodoHash* nuevoNodo = new NodoHash(pedido, buckets[indice]);
    buckets[indice] = nuevoNodo;
    cantidadElementos++;
    return true;
}

Pedido* TablaHash::buscar(const std::string& idPedido) const {
    size_t indice = calcularHash(idPedido);
    NodoHash* actual = buckets[indice];
    while (actual != nullptr) {
        if (actual->pedido->getIdPedido() == idPedido) {
            return actual->pedido;
        }
        actual = actual->siguiente;
    }
    return nullptr;
}

bool TablaHash::eliminar(const std::string& idPedido) {
    size_t indice = calcularHash(idPedido);
    NodoHash* actual = buckets[indice];
    NodoHash* anterior = nullptr;

    while (actual != nullptr) {
        if (actual->pedido->getIdPedido() == idPedido) {
            if (anterior == nullptr) {
                buckets[indice] = actual->siguiente;
            } else {
                anterior->siguiente = actual->siguiente;
            }
            delete actual; // Libera solo el NodoHash; SistemaLogistico libera el Pedido*
            cantidadElementos--;
            return true;
        }
        anterior = actual;
        actual = actual->siguiente;
    }
    return false;
}

void TablaHash::liberarPedidosYLimpiar() {
    for (size_t i = 0; i < capacidad; ++i) {
        NodoHash* actual = buckets[i];
        while (actual != nullptr) {
            NodoHash* siguiente = actual->siguiente;
            delete actual->pedido;
            delete actual;
            actual = siguiente;
        }
        buckets[i] = nullptr;
    }
    cantidadElementos = 0;
    colisionesTotales = 0;
}

size_t TablaHash::obtenerCantidad() const {
    return cantidadElementos;
}

size_t TablaHash::obtenerCapacidad() const {
    return capacidad;
}

size_t TablaHash::obtenerColisionesTotales() const {
    return colisionesTotales;
}

double TablaHash::obtenerFactorCarga() const {
    return static_cast<double>(cantidadElementos) / static_cast<double>(capacidad);
}

size_t TablaHash::obtenerLongitudMaximaCadena() const {
    size_t maxLen = 0;
    for (size_t i = 0; i < capacidad; ++i) {
        size_t len = 0;
        NodoHash* actual = buckets[i];
        while (actual != nullptr) {
            len++;
            actual = actual->siguiente;
        }
        if (len > maxLen) {
            maxLen = len;
        }
    }
    return maxLen;
}
