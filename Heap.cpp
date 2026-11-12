#include "Heap.h"

MinHeap::MinHeap(int capacidadInicial)
    : capacidad(capacidadInicial > 16 ? capacidadInicial : 16), tamano(0) {
    datos = new Pedido*[capacidad];
}

MinHeap::~MinHeap() {
    delete[] datos;
    datos = nullptr;
}

int MinHeap::calcularClavePrioridad(const Pedido* p) const {
    // Invertimos el signo: mayor nivelPrioridad => clave más negativa (menor en el Min-Heap)
    return -p->getNivelPrioridad();
}

bool MinHeap::tieneMayorUrgencia(const Pedido* a, const Pedido* b) const {
    int claveA = calcularClavePrioridad(a);
    int claveB = calcularClavePrioridad(b);

    if (claveA != claveB) {
        return claveA < claveB;
    }
    // Desempate FIFO: ante igual urgencia, despachar primero el pedido más antiguo
    if (a->getFecha() != b->getFecha()) {
        return a->getFecha() < b->getFecha();
    }
    if (a->getHora() != b->getHora()) {
        return a->getHora() < b->getHora();
    }
    return a->getIdPedido() < b->getIdPedido();
}

void MinHeap::intercambiar(int i, int j) {
    Pedido* temp = datos[i];
    datos[i] = datos[j];
    datos[j] = temp;

    // Mantener sincronizado el índice de cada pedido en O(1)
    datos[i]->setIndiceHeap(i);
    datos[j]->setIndiceHeap(j);
}

void MinHeap::redimensionar() {
    int nuevaCapacidad = capacidad * 2;
    Pedido** nuevoArreglo = new Pedido*[nuevaCapacidad];
    for (int i = 0; i < tamano; ++i) {
        nuevoArreglo[i] = datos[i];
    }
    delete[] datos;
    datos = nuevoArreglo;
    capacidad = nuevaCapacidad;
}

void MinHeap::subir(int indice) {
    while (indice > 0) {
        int padre = (indice - 1) / 2;
        if (tieneMayorUrgencia(datos[indice], datos[padre])) {
            intercambiar(indice, padre);
            indice = padre;
        } else {
            break;
        }
    }
}

void MinHeap::bajar(int indice) {
    while (true) {
        int izquierdo = 2 * indice + 1;
        int derecho = 2 * indice + 2;
        int masUrgente = indice;

        if (izquierdo < tamano && tieneMayorUrgencia(datos[izquierdo], datos[masUrgente])) {
            masUrgente = izquierdo;
        }
        if (derecho < tamano && tieneMayorUrgencia(datos[derecho], datos[masUrgente])) {
            masUrgente = derecho;
        }

        if (masUrgente != indice) {
            intercambiar(indice, masUrgente);
            indice = masUrgente;
        } else {
            break;
        }
    }
}

void MinHeap::insertar(Pedido* pedido) {
    if (pedido == nullptr) return;

    // Solo los pedidos en estado "Pendiente" deben ingresar a la cola de despacho
    if (pedido->getEstado() != "Pendiente") return;

    // Evitar insertar dos veces el mismo pedido en el Heap
    if (pedido->getIndiceHeap() != -1) return;

    if (tamano == capacidad) {
        redimensionar();
    }

    datos[tamano] = pedido;
    pedido->setIndiceHeap(tamano);
    tamano++;
    subir(tamano - 1);
}

Pedido* MinHeap::extraerMin() {
    while (tamano > 0) {
        Pedido* minimo = datos[0];
        tamano--;

        if (tamano > 0) {
            datos[0] = datos[tamano];
            datos[0]->setIndiceHeap(0);
            bajar(0);
        }

        minimo->setIndiceHeap(-1);

        // Garantía defensiva: nunca despachar pedidos cancelados o no pendientes
        if (minimo->getEstado() == "Pendiente") {
            return minimo;
        }
    }
    return nullptr;
}

bool MinHeap::eliminarEnIndice(int indice) {
    if (indice < 0 || indice >= tamano) {
        return false;
    }

    Pedido* eliminado = datos[indice];
    eliminado->setIndiceHeap(-1);
    tamano--;

    if (indice < tamano) {
        datos[indice] = datos[tamano];
        datos[indice]->setIndiceHeap(indice);

        int padre = (indice - 1) / 2;
        if (indice > 0 && tieneMayorUrgencia(datos[indice], datos[padre])) {
            subir(indice);
        } else {
            bajar(indice);
        }
    }
    return true;
}

Pedido* MinHeap::verMin() const {
    if (tamano == 0) return nullptr;
    return datos[0];
}

int MinHeap::obtenerTamano() const {
    return tamano;
}

bool MinHeap::estaVacio() const {
    return tamano == 0;
}

void MinHeap::limpiar() {
    for (int i = 0; i < tamano; ++i) {
        if (datos[i] != nullptr) {
            datos[i]->setIndiceHeap(-1);
        }
    }
    tamano = 0;
}
