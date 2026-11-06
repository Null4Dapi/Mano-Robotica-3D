# Capítulo I: Aspectos Generales del Proyecto

### 1.1. Descripción de la Problemática

En el ámbito de la ingeniería de software aplicada al control cinemático y a la robótica de manipuladores articulados, la programación de rutinas motrices requiere un alto grado de determinismo y seguridad operacional. Sin embargo, en la práctica común del desarrollo de prototipos y simuladores, la transmisión de órdenes de movimiento suele gestionarse mediante scripts interpretados de forma ad-hoc o mediante la inyección directa de valores numéricos hacia los actuadores mecánicos o motores de renderizado gráfico. Esta aproximación carece de una arquitectura de compilación intermedia, lo que introduce discrepancias lógicas y fallas estructurales que comprometen la estabilidad del sistema (Aho et al., 2007).

En primer lugar, la ausencia de una especificación formal de lenguaje impide validar la estructura de las instrucciones antes de su ejecución. Al no implementarse analizadores sintácticos basados en gramáticas libres de contexto, el sistema procesa cadenas incompletas o mal formadas sin reportar errores de manera temprana. De acuerdo con Carranza Sahagún (2024), la falta de un análisis sintáctico determinista traslada la detección de fallas al tiempo de ejecución, lo cual, en un entorno cinemático de múltiples grados de libertad, genera configuraciones articulares impredecibles y desordenadas.

En segundo lugar, se evidencia una deficiencia crítica en el análisis semántico y en la administración del contexto de las variables. Los entornos directos carecen de una tabla de símbolos estructurada dinámicamente que soporte ámbitos locales y globales, así como de un sistema de comprobación de tipos de datos. Esta carencia imposibilita la verificación estricta de restricciones físicas propias del dominio biomecánico, tales como los límites angulares de deflexión en articulaciones falángicas, cuyo rango operativo admisible está confinado entre los 0° y 90°. La ausencia de estas comprobaciones en tiempo de compilación permite que órdenes que exceden los límites mecánicos alcancen la etapa de accionamiento, provocando colisiones virtuales en la simulación o sobreesfuerzo de actuadores en implementaciones físicas (Cooper & Torczon, 2022).

Finalmente, el acoplamiento directo entre la entrada del usuario y la plataforma de simulación impide la generación de una representación intermedia independiente de la arquitectura. Al no descomponerse las sentencias complejas en código de tres direcciones o cuádruplas estandarizadas, resulta inviable aplicar algoritmos de optimización sobre bloques básicos. En consecuencia, el sistema emite instrucciones redundantes y ejecuta operaciones aritméticas estáticas que saturan innecesariamente el procesamiento computacional, en lugar de precomputar valores constantes y depurar movimientos nulos de forma previa a la generación del código objeto (Sánchez Dueñas & Valverde Andreu, 2022).

---

### 1.2. Planteamiento del Problema

#### 1.2.1. Problema General

¿De qué manera el desarrollo de un compilador en C++ basado en las fases canónicas de análisis sintáctico, análisis semántico, generación de código intermedio y optimización permite validar formalmente y ejecutar de forma eficiente rutinas cinemáticas para una mano robótica 3D?

#### 1.2.2. Problemas Específicos

¿Cómo estructurar y validar una gramática libre de contexto mediante el cálculo formal de los conjuntos de predicción PRIMERO y SIGUIENTE y la construcción de la tabla sintáctica predictiva LL(1), garantizando la ausencia de conflictos en el reconocimiento de sentencias cinemáticas base?

¿Cómo modelar el reconocimiento ascendente de expresiones de movimiento articular empleando una gramática aumentada, la colección canónica de estados LR(0) y las tablas ACTION y GOTO bajo la lógica de operaciones de desplazamiento y reducción?

¿Cómo implementar la integración automatizada entre un analizador léxico y un analizador sintáctico mediante herramientas generadoras y archivos de especificación formal con resolución de precedencias para la construcción del árbol de sintaxis abstracta?

¿Cómo diseñar e implementar una tabla de símbolos dinámica con gestión de ámbitos y reglas de verificación de tipos que permitan comprobar la coherencia lógica de las instrucciones y restringir los límites angulares articulares en tiempo de compilación?

¿Cómo traducir el árbol de sintaxis abstracta a una representación intermedia de código de tres direcciones estructurada en registros lineales de cuádruplas mediante el empleo de variables temporales?

¿Cómo aplicar técnicas de optimización sobre bloques básicos en las cuádruplas intermedias para generar el código objeto final que gobierne la simulación tridimensional con eficiencia computacional?

---

### 1.3. Objetivos

#### 1.3.1. Objetivo General

Desarrollar un sistema de compilación y simulación cinemática tridimensional que garantice la validación biomecánica y el control articular seguro de una mano robótica de 18 grados de libertad.

#### 1.3.2. Objetivos Específicos

* Construir el módulo de análisis predictivo LL(1) y ascendente LR(0) que permita verificar formalmente el determinismo de la gramática cinemática libre de contexto, calculando los conjuntos PRIMERO y SIGUIENTE y construyendo las tablas canónicas de control.
* Implementar el procesador léxico y sintáctico mediante especificaciones formales de Flex y Bison en C++20 para la construcción determinista del Árbol de Sintaxis Abstracta (AST) a partir de programas cinemáticos (`.rob`).
* Implementar la tabla de símbolos con gestión dinámica de ámbitos (global y local) y el analizador semántico que restrinja estrictamente los ángulos de flexión articular al intervalo biomecánico seguro de 0° a 90°.
* Desarrollar el generador de código intermedio que transforme el AST en un registro lineal de cuádruplas `(op, arg1, arg2, res)` mediante variables temporales.
* Desarrollar el optimizador de bloques básicos que aplique plegado de constantes, propagación de valores y eliminación de código muerto cinemático, emitiendo la trayectoria compilada en formato JSON.
* Construir el entorno de simulación virtual tridimensional en Three.js con WebGL que desacople la visualización de la compilación y anime los 18 grados de libertad coordinados en tiempo real.

---

### 1.4. Justificación

#### 1.4.1. Justificación Técnica

La justificación técnica del proyecto se fundamenta en los requerimientos específicos de programación, infraestructura de software, hardware y viabilidad de acceso:

* **Componentes que se van a programar:**
  1. *Compilador nativo modular en C++20:* Estructurado en un *frontend* (analizador léxico con Flex y sintáctico LALR(1) con Bison), una tabla de símbolos con gestión de ámbitos para comprobación estricta de tipos y límites físicos biomecánicos [0°, 90°], y un *backend* de generación de Código de Tres Direcciones (TAC) estructurado en cuádruplas lineales con un optimizador de bloques básicos (*constant folding*, propagación y eliminación de código muerto).
  2. *Visor cinemático 3D y entorno web:* Motor de renderizado en Three.js con WebGL que desacopla la visualización de la compilación, interpretando el archivo `trayectoria.json` emitido por el compilador para animar los 18 grados de libertad (flexión y abducción lateral de los 5 dedos y muñeca).

* **Requerimientos de Software:**
  * *Para el entorno de desarrollo y compilación:* Compilador GCC/G++ con soporte estándar C++20, generador léxico Flex (v2.6 o superior), generador sintáctico GNU Bison (v3.8 o superior) y el gestor de compilación GNU Make.
  * *Para el entorno de ejecución del usuario:* Cualquier navegador web moderno con soporte nativo para WebGL 2.0 y ECMAScript 6 (Google Chrome, Mozilla Firefox, Microsoft Edge o navegadores basados en Chromium). No se requiere instalación de software propietario, licencias de terceros ni controladores de hardware especiales.

* **Requerimientos de Hardware:**
  * *Máquina de desarrollo / compilación:* Computadora con procesador x86_64 o ARM64 dual-core a 2.0 GHz o superior, 4 GB de memoria RAM y 500 MB de espacio disponible en almacenamiento.
  * *Máquina del usuario final:* Computadora de escritorio o portátil estándar con gráficos integrados (Intel HD/UHD Graphics, AMD Radeon Vega o superior) capaz de renderizar gráficos 3D en WebGL a 60 FPS, y pantalla con resolución mínima de 1280×720 píxeles. De manera opcional para la captura gestual, una cámara web estándar (720p a 30 FPS).

* **Criterio de Viabilidad y Adquisición:**
  El proyecto cumple estrictamente con el principio de viabilidad técnica:
  * El usuario final y las instituciones educativas **ya poseen** computadoras estándar con navegadores web comerciales instalados.
  * Todo el software utilizado en el desarrollo y despliegue (Linux, GCC, Flex, Bison, Three.js) es de **código abierto y distribución libre**, por lo que el costo de adquisición de software es nulo (S/. 0.00).
  * No se impone al usuario la necesidad de adquirir una mano robótica física o tarjetas electrónicas propietarias para experimentar y evaluar las rutinas cinemáticas, garantizando una disponibilidad operativa del 100%.

#### 1.4.2. Justificación Social

En el contexto de la educación universitaria y la investigación aplicada, el acceso a plataformas físicas de experimentación robótica suele estar condicionado por elevados requerimientos presupuestales y disponibilidad de laboratorios especializados. La creación de un sistema de compilación acoplado a un entorno de simulación tridimensional accesible proporciona una herramienta formativa de libre acceso para estudiantes e investigadores en el campo de la computación, la biomecánica y los sistemas articulados.

Esta plataforma permite a los usuarios diseñar, verificar y evaluar patrones motores complejos en un marco metodológico estructurado, promoviendo el aprendizaje práctico de la teoría formal de compiladores aplicada a problemas contemporáneos de control de mecanismos. De este modo, se democratiza el acceso a herramientas de simulación determinista que pueden servir como base para futuros desarrollos en interfaces de teleoperación o prótesis asistenciales sin barreras de infraestructura física (Sánchez Dueñas & Valverde Andreu, 2022).

#### 1.4.3. Justificación Financiera

La justificación financiera se sustenta en el **ahorro económico cuantificable en Soles (S/.)** y el retorno de inversión directo para el desarrollador, empresa o laboratorio de robótica mediante la prevención de siniestros de hardware y la eliminación de licencias de software privativo:

| Rubro de Costo | Enfoque Tradicional Empírico (Físico / Privativo) | Enfoque Propuesto con Compilador y Simulación 3D | Ahorro Económico Estimado (S/.) |
| :--- | :--- | :--- | :--- |
| **Actuadores y servomotores** | Rotura promedio de 4 a 6 servomotores por colisión o sobreesfuerzo angular (S/. 35.00 c/u) | **S/. 0.00:** El analizador semántico intercepta ángulos fuera de rango [0°, 90°] antes de emitir la trayectoria | **S/. 140.00 – S/. 210.00** por ciclo de prueba |
| **Chasis estructural de la mano** | Fractura mecánica de falanges impresas en resina/filamento técnico (S/. 180.00 juego completo) | **S/. 0.00:** Verificación cinemática virtual previa sin desgaste de material | **S/. 180.00** por incidente evitado |
| **Licenciamiento de software** | Licencias comerciales de simulación robótica (ej. MATLAB / Simscape: ~S/. 8,500.00 anuales) | **S/. 0.00:** Stack 100% de código abierto (C++20, Flex, Bison, Three.js) | **S/. 8,500.00 anuales** por estación de trabajo |
| **Horas de calibración y prueba** | ~25 horas/hombre de ajuste manual en banco de pruebas (a S/. 25.00/hora = S/. 625.00) | ~3 horas/hombre mediante generación automática de cuádruplas optimizadas (S/. 75.00) | **S/. 550.00** de ahorro en costo laboral por rutina |

* **Análisis de Rentabilidad para el Desarrollador/Empresario:**
  Implementar una mano robótica física de 18 GDL implica una inversión inicial de entre **S/. 1,200.00 y S/. 1,800.00** en hardware (18 actuadores, chasis articulado y microcontrolador). Someter el prototipo físico a rutinas sin verificar expone el 100% del capital invertido a pérdidas por sobrecalentamiento y bloqueo mecánico. Con la solución propuesta, el compilador cinemático y el simulador 3D reducen a cero el riesgo de daño físico durante las pruebas de diseño y automatizan la optimización de código, lo que se traduce en un **ahorro financiero neto superior a S/. 9,000.00** en el primer año de desarrollo por cada estación de trabajo (Carranza Sahagún, 2024).

---

### Referencias Bibliográficas (Normas APA 7.ª Edición)

* Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2007). *Compilers: Principles, techniques, and tools* (2.ª ed.). Pearson Addison-Wesley.
* Carranza Sahagún, D. U. (2024). *Compiladores: Fases de análisis*. Editorial Transdigital.
* Cooper, K. D., & Torczon, L. (2022). *Engineering a compiler* (3.ª ed.). Morgan Kaufmann.
* Gaudioso Vázquez, E. (2022). *Introducción a la teoría de autómatas, gramáticas y lenguajes* (2.ª ed.). Centro de Estudios Ramón Areces.
* Sánchez Dueñas, G., & Valverde Andreu, J. A. (2022). *Compiladores e intérpretes: Un enfoque pragmático*. Ediciones Díaz de Santos.
