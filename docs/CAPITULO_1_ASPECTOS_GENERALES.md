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

Desarrollar un compilador en C++ basado en las fases canónicas de análisis sintáctico, análisis semántico, generación de código intermedio y optimización para la validación formal y ejecución eficiente de rutinas cinemáticas aplicadas a una mano robótica 3D.

#### 1.3.2. Objetivos Específicos

* Diseñar una gramática libre de contexto y evaluar su determinismo mediante el cálculo formal de los conjuntos de predicción PRIMERO y SIGUIENTE y la construcción de la tabla predictiva LL(1), garantizando la ausencia de conflictos en el reconocimiento de sentencias cinemáticas base.
* Modelar el reconocimiento sintáctico ascendente de expresiones de movimiento articular a través de una gramática aumentada, la colección canónica de estados LR(0) y las tablas de control ACTION y GOTO mediante la lógica de operaciones de desplazamiento y reducción.
* Implementar la integración modular entre el analizador léxico y el analizador sintáctico mediante generadores automáticos y archivos de especificación formal con configuración de precedencias para la generación del árbol de sintaxis abstracta.
* Construir una tabla de símbolos dinámica con administración de ámbitos locales y globales, incorporando reglas de verificación de tipos que permitan comprobar la coherencia de las operaciones e imponer restricciones sobre los rangos angulares físicos permitidos.
* Desarrollar el módulo de generación de código intermedio encargado de recorrer el árbol de sintaxis abstracta y traducirlo a instrucciones lineales de código de tres direcciones estructuradas en registros de cuádruplas mediante variables temporales.
* Diseñar e implementar un plan de optimización sobre bloques básicos que aplique técnicas de plegado de constantes, propagación de valores y eliminación de código muerto en las cuádruplas para generar el código objeto final que controle la simulación tridimensional.

---

### 1.4. Justificación

#### 1.4.1. Justificación Técnica

Desde el punto de vista de la ingeniería de software y los sistemas ciberfísicos, la implementación de un compilador formal proporciona un desacoplamiento riguroso entre la especificación de alto nivel de las trayectorias y el módulo de accionamiento cinemático. El uso de C++ como lenguaje de desarrollo responde a la necesidad de garantizar una ejecución determinista, gestión directa y predecible de memoria y un alto rendimiento computacional sin sobrecargas por recolección de basura (*garbage collection*), aspectos indispensables en el procesamiento de sistemas de tiempo real (Cooper & Torczon, 2022).

Asimismo, la estructura modular del compilador permite aislar las etapas de validación estructural y semántica de la capa de ejecución. La verificación semántica guiada por la sintaxis asegura que las transgresiones de tipos o de límites biomecánicos sean interceptadas de forma estática en tiempo de compilación, impidiendo que comandos potencialmente dañinos o físicamente inconsistentes alcancen la fase de renderizado. Por su parte, la adopción de una representación intermedia basada en código de tres direcciones y registros de cuádruplas confiere independencia respecto a la máquina de destino, facilitando la aplicación sistemática de algoritmos de optimización de bloques básicos antes de emitir las órdenes de movimiento al efector final (Aho et al., 2007).

#### 1.4.2. Justificación Social

En el contexto de la educación universitaria y la investigación aplicada, el acceso a plataformas físicas de experimentación robótica suele estar condicionado por elevados requerimientos presupuestales y disponibilidad de laboratorios especializados. La creación de un sistema de compilación acoplado a un entorno de simulación tridimensional accesible proporciona una herramienta formativa de libre acceso para estudiantes e investigadores en el campo de la computación, la biomecánica y los sistemas articulados.

Esta plataforma permite a los usuarios diseñar, verificar y evaluar patrones motores complejos en un marco metodológico estructurado, promoviendo el aprendizaje práctico de la teoría formal de compiladores aplicada a problemas contemporáneos de control de mecanismos. De este modo, se democratiza el acceso a herramientas de simulación determinista que pueden servir como base para futuros desarrollos en interfaces de teleoperación o prótesis asistenciales sin barreras de infraestructura física (Sánchez Dueñas & Valverde Andreu, 2022).

#### 1.4.3. Justificación Financiera

En los proyectos de desarrollo robótico y mecatrónico, la etapa de pruebas empíricas directas sobre prototipos físicos representa un foco constante de sobrecostos financieros debido al desgaste prematuro o rotura de servomotores, desgranamiento de engranajes y fallas estructurales originadas por comandos con sobreextensión mecánica o trayectorias no sincronizadas. Cada incidente mecánico en hardware real demanda la sustitución de piezas, adquisición de repuestos electrónicos y tiempos prolongados de inactividad técnica.

La incorporación de un compilador semántico que valide los límites físicos y un simulador tridimensional que visualice la cinemática de forma previa a cualquier despliegue reduce a cero la tasa de deterioro de componentes durante la fase de diseño y programación. Adicionalmente, las técnicas de optimización de código intermedio suprimen instrucciones redundantes y consolidan movimientos antes de la ejecución, disminuyendo el número total de ciclos de procesamiento requeridos y optimizando el consumo de recursos computacionales de la plataforma (Carranza Sahagún, 2024).

---

### Referencias Bibliográficas (Normas APA 7.ª Edición)

* Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2007). *Compilers: Principles, techniques, and tools* (2.ª ed.). Pearson Addison-Wesley.
* Carranza Sahagún, D. U. (2024). *Compiladores: Fases de análisis*. Editorial Transdigital.
* Cooper, K. D., & Torczon, L. (2022). *Engineering a compiler* (3.ª ed.). Morgan Kaufmann.
* Gaudioso Vázquez, E. (2022). *Introducción a la teoría de autómatas, gramáticas y lenguajes* (2.ª ed.). Centro de Estudios Ramón Areces.
* Sánchez Dueñas, G., & Valverde Andreu, J. A. (2022). *Compiladores e intérpretes: Un enfoque pragmático*. Ediciones Díaz de Santos.
