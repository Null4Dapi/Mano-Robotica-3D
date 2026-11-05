# ACTA DE CONSTITUCIÓN DEL PROYECTO (PROJECT CHARTER)

---

## 1. INFORMACIÓN GENERAL DEL PROYECTO

| Campo | Detalle |
| :--- | :--- |
| **Nombre del Proyecto** | Compilador en C++ y Entorno de Simulación Cinemática 3D para Mano Robótica de 18 GDL |
| **Código del Proyecto** | AC-2026-EQUIPO01 |
| **Curso / Asignatura** | Autómatas y Compiladores (Ciclo V) |
| **Institución** | Universidad César Vallejo - Facultad de Ingeniería y Arquitectura |
| **Docente Patrocinador** | Ing. Charles Dummar Camasca Macedo |
| **Equipo de Desarrollo** | Equipo N° 1 |
| **Fecha de Elaboración** | Mayo de 2026 |

---

## 2. JUSTIFICACIÓN Y PROPÓSITO DEL PROYECTO

En el desarrollo y control de manipuladores articulados biomecánicos, la programación de patrones de movimiento suele gestionarse mediante scripts directos no estructurados. Esta práctica traslada errores de sintaxis, discrepancias de tipo e instrucciones con ángulos mecánicamente destructivos (> 90° en falanges) directamente a los actuadores o al motor de renderizado.

El presente proyecto tiene como propósito construir una capa de software formal de base: un compilador canónico en C++ que procese un Lenguaje de Dominio Específico (DSL) denominado *RoboHandScript*, verificando formalmente la sintaxis (LL(1), LR(0), Flex/Bison), validando restricciones biomecánicas mediante una tabla de símbolos dinámicas con ámbitos, traduciendo el movimiento a cuádruplas intermedias optimizadas (reducción de código muerto y plegado de constantes) y proyectando el resultado final en un simulador visual tridimensional de 18 grados de libertad en WebGL.

---

## 3. OBJETIVOS DEL PROYECTO (CRITERIOS SMART)

### 3.1. Objetivo General
Desarrollar e implementar un sistema de compilación en C++ y visualización 3D durante el ciclo académico 2026-I, capaz de procesar programas en un lenguaje cinemático formal (`.rob`) logrando un **100% de detección estática de errores sintácticos y transgresiones biomecánicas** de manera previa a la generación de la trayectoria ejecutable.

### 3.2. Objetivos Específicos
1. **Sintaxis LL(1):** Diseñar una gramática cinemática de al menos 10 producciones y calcular formalmente sus conjuntos PRIMERO y SIGUIENTE, construyendo una tabla predictiva con **0% de conflictos en sus celdas**.
2. **Sintaxis LR(0):** Construir la colección canónica de estados LR(0) y las tablas ACTION y GOTO para el reconocimiento ascendente de expresiones motrices, evidenciando formalmente al menos **3 trazas de análisis Shift/Reduce** sobre la pila.
3. **Generadores Automáticos:** Configurar la integración de Flex y Bison en C++ para parsear programas cinemáticos, logrando la construcción del Árbol de Sintaxis Abstracta (AST) en un tiempo de procesamiento **menor a 50 ms para scripts de hasta 500 líneas**.
4. **Semántica y Tabla de Símbolos:** Implementar una tabla de símbolos con soporte de **2 niveles de ámbito (global y local)** y reglas semánticas que bloqueen el **100% de asignaciones angulares fuera del intervalo seguro de $0^\circ$ a $90^\circ$**.
5. **Código Intermedio:** Desarrollar el generador de Código de Tres Direcciones que transforme el AST en un registro lineal de **cuádruplas `(op, arg1, arg2, res)`**, desagregando el 100% de las expresiones complejas y bucles mediante variables temporales.
6. **Optimización y Código Objeto:** Aplicar algoritmos de análisis de bloques básicos (plegado de constantes, propagación y eliminación de código muerto), logrando una **reducción mínima del 15% en el número total de cuádruplas ejecutables** antes de emitir la trayectoria hacia el simulador 3D.

---

## 4. DELIMITACIÓN DEL ALCANCE

### 4.1. Lo que SÍ Incluye el Proyecto (Entregables Comprometidos)
* **Especificación formal del DSL:** Gramática formal del lenguaje cinemático en archivos de prueba (`.rob`).
* **Módulo CLI de Inspección Teórica en C++:** Banderas de línea de comandos para evidenciar el paso a paso de cada semana (`--analisis-ll1`, `--analisis-lr0`, `--tabla-simbolos`, `--cuadruplas`, `--optimizar`).
* **Núcleo del Compilador C++ (Monolito Modular):**
  * Analizador léxico con Flex (`scanner.l`).
  * Analizador sintáctico con Bison (`parser.y`) y construcción del AST.
  * Analizador semántico y Tabla de Símbolos orientada a objetos con control de ámbitos.
  * Generador de Código de Tres Direcciones en Cuádruplas.
  * Optimizador sobre bloques básicos.
* **Exportador de Código Objeto:** Generación de un archivo de trayectoria estructurado (`trayectoria.json`).
* **Simulador Gráfico 3D Web:** Entorno interactivo en WebGL/Three.js para la reproducción cinemática de los 18 GDL de la mano robótica.

### 4.2. Exclusiones Explícitas (Lo que NO Incluye el Proyecto)
* **No incluye conexión a hardware físico:** No se conectará a microcontroladores (Arduino, ESP32) ni servomotores reales durante esta fase académica.
* **No incluye visión artificial:** No se utilizará procesamiento de video ni captura de cámara web, manteniendo el foco estricto en la teoría de compiladores.
* **No incluye cinemática inversa:** El cálculo cinemático es directo (rotaciones angulares explícitas por falange y muñeca).
* **No incluye entorno IDE gráfico:** La herramienta de compilación se ejecuta por consola (CLI).

---

## 5. ROLES Y RESPONSABILIDADES DEL EQUIPO

| Rol en el Proyecto | Responsabilidades Principales |
| :--- | :--- |
| **Arquitecto de Software & Líder de Proyecto** | Coordinación general, diseño de interfaces C++, flujo del pipeline principal (`main.cpp`). |
| **Ingeniero de Sintaxis Formal** | Cálculo de FIRST/FOLLOW (LL1), autómata LR(0), especificaciones Flex/Bison. |
| **Ingeniero de Semántica** | Tabla de símbolos dinámica, validación de tipos y restricciones angulares [0°, 90°]. |
| **Ingeniero de Código Intermedio y Optimización** | Recorrido del AST, emisión de cuádruplas, particionado en bloques básicos y optimización. |
| **Ingeniero de Simulación y Calidad (QA)** | Visor 3D de la mano robótica, integración de trayectorias y redacción del informe técnico bajo APA 7. |

---

## 6. GESTIÓN DE RIESGOS INICIALES

| ID | Riesgo Identificado | Probabilidad | Impacto | Estrategia de Mitigación |
| :---: | :--- | :---: | :---: | :--- |
| **R01** | Ambigüedad o conflictos Shift/Reduce en la gramática de Bison | Media | Alto | Factorizar la gramática y establecer precedencias explícitas con `%left`/`%right`. |
| **R02** | Acoplamiento rígido entre el compilador C++ y la simulación gráfica | Baja | Medio | Desacoplar completamente las capas mediante un contrato de datos estandarizado (`trayectoria.json`). |
| **R03** | Sobrecarga de complejidad en la estructura de clases del AST | Media | Medio | Seguir el patrón Visitor o herencia polimórfica simple respetando los principios SOLID. |

---

## 7. CRITERIOS DE ACEPTACIÓN Y CIERRE

El proyecto se considerará formalmente concluido y aceptado cuando:
1. El compilador en C++ compile limpiamente y ejecute las banderas de diagnóstico de las semanas 10 a 15.
2. Los scripts de prueba válidos generen la trayectoria optimizada y se reproduzcan en el simulador 3D de la mano robótica.
3. Los scripts con errores de sintaxis o ángulos prohibidos sean rechazados con mensajes informativos de línea y columna.
4. El informe técnico cumpla integralmente con la estructura requerida por la universidad y las normas APA (7.ª edición).

---

### Aprobación Formal del Proyecto

_____________________________  
**Director del Proyecto / Equipo N° 1**  

_____________________________  
**Docente de la Asignatura**  
Ing. Charles Dummar Camasca Macedo
