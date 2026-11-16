# Sistema Masivo de Gestión Logística y Despacho de Pedidos (E-Commerce) — EDA II

Proyecto universitario de **Estructura de Datos y Algoritmos II** desarrollado íntegramente en **C++** mediante programación orientada a objetos, punteros y gestión manual de memoria dinámica (`new` / `delete`), sin utilizar contenedores de la STL (`std::map`, `std::set`, `std::unordered_map`, `std::priority_queue`, `std::vector`, etc.).

## 1. Estructura del Proyecto

```text
SistemaLogistico/
│
├── main.cpp                  # Interfaz de consola, menú interactivo y control de entrada
├── Pedido.h / Pedido.cpp     # Entidad principal Pedido con clave cronológica e índice en Heap
├── TablaHash.h / .cpp        # Tabla Hash con encadenamiento separado y rehashing dinámico O(1)
├── Heap.h / Heap.cpp         # Min-Heap binario dinámico con clave invertida (-nivelPrioridad)
├── ArbolAVL.h / .cpp         # Árbol AVL auto-balanceado (4 rotaciones) + consultas por rango
├── SistemaLogistico.h / .cpp # Orquestador central, validaciones, sincronización y benchmark
│
├── datos/
│   ├── pedidos_pequeno.csv   # Dataset de 35 registros para verificación manual
│   └── pedidos_masivo.csv    # Dataset de 10,500 registros para pruebas de rendimiento
│
└── README.md                 # Documentación de compilación, uso y pruebas
```

## 2. Requisitos y Compilación

### Requisitos
- Compilador **g++** (GCC 9) o **clang++** con soporte para **C++17**.
- Terminal en Linux, macOS o Windows (MinGW / MSYS2 / WSL).

### Comando de Compilación (Linux / macOS / MinGW)
Desde la carpeta raíz `SistemaLogistico/`, ejecute:

```bash
g++ -std=c++17 -O2 -Wall -Wextra main.cpp Pedido.cpp TablaHash.cpp Heap.cpp ArbolAVL.cpp SistemaLogistico.cpp -o sistema_logistico
```

### Ejecución
```bash
./sistema_logistico
```
*(En Windows PowerShell o CMD: `sistema_logistico.exe`)*

> **Importante:** Ejecute el binario desde la carpeta `SistemaLogistico/` para que las rutas relativas `datos/pedidos_pequeno.csv` y `datos/pedidos_masivo.csv` sean reconocidas automáticamente.

## 3. Verificación de Ausencia de Fugas de Memoria (Opcional)

En Linux/macOS puede auditar la gestión dinámica de memoria con **AddressSanitizer** o **Valgrind**:

```bash
g++ -std=c++17 -g -fsanitize=address,leak main.cpp Pedido.cpp TablaHash.cpp Heap.cpp ArbolAVL.cpp SistemaLogistico.cpp -o sistema_logistico_asan
./sistema_logistico_asan
```

## 4. Guía Rápida de Funcionalidades y Pruebas

1. **Opción 1 (Carga masiva):** Ingrese `datos/pedidos_masivo.csv` para cargar los 10,500 pedidos simultáneamente en la Tabla Hash, el Árbol AVL y el Min-Heap.
2. **Opción 2 (Registro individual):** Permite dar de alta un nuevo pedido verificando unicidad en `O(1)` y formatos estrictos (`YYYY-MM-DD`, `HH:MM:SS`).
3. **Opción 3 (Búsqueda por ID):** Consulta instantánea en `O(1)` promedio mediante Tabla Hash.
4. **Opción 4 (Despacho urgente):** Extrae los $N$ pedidos de mayor `nivelPrioridad` usando `extraerMin()` con clave `-nivelPrioridad` y actualiza su estado a `EnTransito`.
5. **Opción 5 (Consulta cronológica):** Recorrido `In-Order` del Árbol AVL limitado a los primeros 30 pedidos ordenados por fecha y hora.
6. **Opción 6 (Auditoría por rango de fechas):** Calcula cantidad de pedidos y monto total vendido en un lapso `[fechaInicio, fechaFin]` podando ramas fuera de rango.
7. **Opción 7 (Actualizar estado):** Modifica el estado de un pedido y sincroniza automáticamente el Min-Heap si deja de estar `Pendiente`.
8. **Opción 8 (Cancelar pedido):** Elimina el pedido de la Tabla Hash, Árbol AVL y Min-Heap, liberando su memoria dinámica sin referencias colgantes.
9. **Opción 9 (Estadísticas generales):** Muestra totales globales en `O(1)` y métricas internas de colisiones y altura del AVL.
10. **Opción 10:** Ejecuta automáticamente todos los escenarios de prueba exigidos en la guía del proyecto midiendo tiempos con `std::chrono`.
