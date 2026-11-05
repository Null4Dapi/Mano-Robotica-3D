# Feature: Compilador en C++ y Máquina Cinemática para Mano Robótica 3D
Módulo integral de compilación, verificación semántica, optimización de cuádruplas y enlace con visor 3D.
Alineado con el temario formal de Autómatas y Compiladores (Semanas 10 a 15) bajo principios SOLID y ODD.

## Specs
S1: "vamos a ser un monolito... pero eso sí vamos a seguir las buenas prácticas, no colocar todo el código de un solo archivo, desacoplarlo, usar la estructura de carpetas Scrum / Screaming Architecture donde la estructura grita lo que es y debe estar modularizado como piezas de lego... seguir principios SOLID, aplicar POO correctamente y mantener tipado estricto en C++."
S2: "Semana 10: Analizadores Sintácticos LL(1). Cálculo formal de conjuntos PRIMERO y SIGUIENTE, construcción de la tabla predictiva LL(1), detección y justificación de ausencia de conflictos."
S3: "Semana 11: Análisis Sintáctico Ascendente LR(0). Gramática aumentada, colección canónica de estados mediante CLAUSURA e IR_A, tablas ACTION y GOTO y traza de operaciones Shift y Reduce."
S4: "Semana 12: Generadores de Analizadores Sintácticos. Integración de Flex y Bison en C++ para parsear el DSL cinemático (.rob) y generar el Árbol de Sintaxis Abstracta (AST) en memoria."
S5: "Semana 13: Análisis Semántico y Tabla de Símbolos. Tabla de símbolos dinámica con gestión de ámbitos (local y global), verificación de tipos y restricción estricta de límites angulares físicos en rango [0°, 90°]."
S6: "Semana 14: Generación de Código Intermedio. Traducción del AST a Código de Tres Direcciones (TAC) estructurado en matriz y archivo de registro de Cuádruplas (op, arg1, arg2, res) con temporales."
S7: "Semana 15: Optimización y Código Objeto. Particionado en bloques básicos, plegado y propagación de constantes, eliminación de código muerto cinemático (reducción >= 15%) y emisión de trayectoria.json compatible con el visor 3D web."
S8: "Cada que hagas una modificación a nivel de estructura de proyecto has los comandos codegraph init y codegraph sync. Además trabajar con Git mediante Work Unit Commits con tipificación correcta y subagentes para preservar contexto."

## Tasks
- [x] T1 [S1, S8] Diseñar el núcleo común de tipos (Token, AST base) y el sistema de compilación Makefile en C++. | route: inline | commit: cd43cec
- [ ] T2 [S2] Implementar el módulo de análisis predictivo LL(1) con cálculo de PRIMERO, SIGUIENTE y tabla sintáctica. | route: worker | commit: pending
- [ ] T3 [S3] Implementar el módulo de análisis ascendente LR(0) con gramática aumentada, autómata canónico y tablas ACTION/GOTO. | route: worker | commit: pending
- [ ] T4 [S4] Implementar las especificaciones Flex (scanner.l) y Bison (parser.y) para el DSL cinemático y construcción del AST. | route: worker | commit: pending
- [ ] T5 [S5] Implementar la Tabla de Símbolos con ámbitos y el Analizador Semántico con verificación de tipos y rangos biomecánicos [0°, 90°]. | route: worker | commit: pending
- [ ] T6 [S6] Implementar la generación de Código de Tres Direcciones (TAC) y la estructura de Cuádruplas a partir del AST. | route: worker | commit: pending
- [ ] T7 [S7] Implementar el Optimizador de Bloques Básicos (folding, propagación, movimientos muertos) y exportador de trayectoria.json. | route: worker | commit: pending
- [ ] T8 [S7, S8] Implementar el CLI principal (main.cpp) con banderas de diagnóstico de semanas 10-15 y validar con el visor web 3D. | route: worker | commit: pending

## Log
L1: "Ya resolví los docs, todo conforme. Empieza hacer la planificación para el desarrollo completo del proyecto. Utiliza agente gentle ai, ODD, usa subagentes para que no se ocupe tu contexto, algo que acotar, es que RDD tiene un límite de tamaño para revisar, cada cambio importante debe entrar en ese tamaño para la revisión de RDD. No olvides trabajar con engram, aprovecha las skills según la necesidad."
