# CyberHand 3D — Compilador Cinemático y Simulación Robótica

Sistema de alto rendimiento implementado en **C++20** estándar acoplado a un entorno de simulación interactivo en **WebGL / Three.js** para el control biomecánico seguro de una mano robótica articulada de 18 grados de libertad (GDL).

---

## Características Principales

* **DSL Cinemático (`.rob`):** Lenguaje estructurado para definir rutinas de movimiento articular (`DEDO`, `ROTAR`, `FALANGE`, `REPETIR`).
* **Frontend Formal:**
  * Análisis sintáctico predictivo **LL(1)** con cálculo de PRIMERO y SIGUIENTE.
  * Análisis sintáctico ascendente **LR(0)** con colección canónica y tablas ACTION/GOTO.
  * Escáner **Flex** y parser **Bison** generadores del Árbol de Sintaxis Abstracta (AST).
* **Análisis Semántico y Seguridad:** Tabla de símbolos con gestión de ámbitos que bloquea ángulos fuera del límite biomecánico físico `[0°, 90°]`.
* **Backend y Optimización:** Generación de Código de Tres Direcciones (TAC) en cuádruplas `(op, arg1, arg2, res)` y optimizador de bloques básicos (*constant folding*, propagación y eliminación de código muerto).
* **Simulador 3D Web:** Visor interactivo en Three.js con visualización cinemática en tiempo real, abducción lateral (*splay*) y reproducción de `trayectoria.json`.

---

## Requisitos del Sistema

* **C++20 Compiler:** `g++` (GCC 11+) o `clang++`
* **Herramientas de Análisis:** `flex` (v2.6+), `bison` (v3.8+), `make`
* **Visualización Web:** Cualquier navegador moderno con soporte WebGL (Chrome, Firefox, Edge).

---

## Compilación y Ejecución

### 1. Compilar el compilador en C++
```bash
make
```

### 2. Ejecutar la suite de pruebas E2E
```bash
make test
```

### 3. Compilar una rutina cinemática (.rob)
```bash
./bin/cyberhand_compiler --compilar examples/rutina_valida.rob -o web/trayectoria.json
```

Opciones de diagnóstico académico disponibles:
* `--analisis-ll1`: Muestra conjuntos PRIMERO, SIGUIENTE y tabla LL(1).
* `--analisis-lr0`: Muestra autómata canónico y tablas ACTION/GOTO.
* `--tabla-simbolos <archivo.rob>`: Imprime la tabla de símbolos y comprobación de límites físicos.
* `--cuadruplas <archivo.rob>`: Imprime el código intermedio (TAC) sin optimizar.
* `--optimizar <archivo.rob>`: Muestra cuádruplas optimizadas y métricas de reducción.

### 4. Iniciar el Visor 3D y Túnel Público
```bash
# Iniciar visor local en http://localhost:8000
make serve

# Compilar rutina y levantar visor en un solo paso
make run

# Exponer el visor local a internet con Cloudflare Tunnel
make tunnel
```
Abre en tu navegador: [http://localhost:8000](http://localhost:8000)

---

## Documentación y Guías

* [Guía de Descarga, Instalación en Windows y Sustentación](docs/GUIA_WINDOWS_SUSTENTACION.md)
* [Principios de Arquitectura de Compiladores (AGENTS.md)](AGENTS.md)
* [Especificación y Tokens de Diseño de la Interfaz (DESIGN.md)](DESIGN.md)

---

## Estructura del Repositorio

```text
├── bin/                 # Binarios generados (cyberhand_compiler)
├── docs/                # Documentación académica y especificaciones formales
├── examples/            # Rutinas de prueba cinemáticas (.rob)
├── src/
│   ├── core/            # Tipos comunes, AST, Tokens
│   ├── grammar/         # Especificaciones Flex (scanner.l) y Bison (parser.y)
│   ├── modules/
│   │   ├── ll1_analyzer/    # Módulo predictivo LL(1)
│   │   ├── lr0_analyzer/    # Módulo ascendente LR(0)
│   │   ├── semantics/       # Tabla de símbolos y validación semántica
│   │   ├── intermediate/    # Generador de Código de Tres Direcciones (TAC)
│   │   └── optimizer/       # Optimizador de bloques básicos y trayectoria
│   └── main.cpp         # CLI unificado del compilador
├── tests/               # Scripts de prueba E2E
└── web/                 # Visor 3D (index.html, Three.js, trayectoria.json)
```

---

## Licencia

Distribuido bajo licencia MIT.
