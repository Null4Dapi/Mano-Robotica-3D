## 

![][image1]

FACULTAD DE INGENIERÍA Y ARQUITECTURA ESCUELA PROFESIONAL DE INGENIERÍA DE SISTEMAS

AUTOMATAS Y COMPILADORES

**Compilador de Comandos Gestuales para el Control de una Mano Robótica 3D**

Integrantes del Equipo N°1:

| Apellidos y Nombres |  | % Participación |  |
| :---: | ----- | :---: | :---: |
| Mendo Ruiz Victor Manuel |  | 100% |  |
| Cueva Fabian Eduardo |  | 100% |  |
| Pando Iglesia Diego Alonso |  | 100% |  |
| Vilchez Francia Gian Franco |  | 100% |  |
| Tapullima Herrera Luciana |  | 100% |  |
|  |  |  |  |

**DOCENTE:**

Charles Dummar Camasca Macedo


**Ciclo V**

**LIMA – 2026**

**Índice**

## 

[**Introducción	3**](#introducción)

[**Capítulo I	3**](#capítulo-i)

[1.1. Descripción de la problemática	3](#1.1.-descripción-de-la-problemática)

[1.2. Planteamiento del problema	4](#1.2.-planteamiento-del-problema)

[1.3. Objetivos	4](#1.3.-objetivos)

[1.3.1. Objetivo general	4](#1.3.1.-objetivo-general)

[1.3.2. Objetivos específicos	5](#1.3.2.-objetivos-específicos)

[1.4. Justificación	5](#1.4.-justificación)

[1.4.1. Técnica	5](#1.4.1.-técnica)

[1.4.2. Social	5](#1.4.2.-social)

[1.4.3. Financiera	5](#1.4.3.-financiera)

## 

## Resumen ejecutivo

## Introducción {#introducción}

El control de sistemas robóticos mediante visión artificial suele trasladar las coordenadas capturadas por la cámara directamente hacia los actuadores mecánicos o motores de renderizado. En escenarios reales, esta conexión directa traslada también el ruido óptico, las oclusiones parciales de los dedos y las lecturas fuera de rango, provocando movimientos erráticos o configuraciones físicamente inválidas en la estructura articulada.

El presente proyecto aborda esta limitación mediante el desarrollo un MVP que intercala un compilador especializado entre el módulo de captura visual y la mano robótica tridimensional. En lugar de procesar vectores crudos sin control, los gestos del usuario se discretizan en sentencias de un lenguaje de control cinemático que atraviesa todas las fases formales de compilación.

## Capítulo I {#capítulo-i}

### 1.1. Descripción de la problemática {#1.1.-descripción-de-la-problemática}

En las interfaces de teleoperación gestual basadas en cámaras web estándar, los modelos de estimación de pose entregan coordenadas espaciales continuas que fluctúan ante variaciones de iluminación, movimientos rápidos o superposición de los dedos. Cuando estos valores alimentan directamente el modelo tridimensional de una mano robótica de 15 articulaciones falángicas y una muñeca de 3 ejes, se presentan tres problemas críticos:

1. Inconsistencia biomecánica: El sensor óptico puede reportar ángulos negativos o superiores al límite físico de una falange (${0}^{\circ }$ a $9{0}^{\circ }$), generando hiperextensiones irreales en la simulación o daños mecánicos si se conecta a servomotores físicos.  
2. Falta de estructura sintáctica: No existe un mecanismo formal que distinga entre un gesto intencional completo y una transición incompleta o ruidosa, ejecutando órdenes fragmentadas sin validación previa.  
3. Acoplamiento tecnológico: Al no existir una representación intermedia estandarizada entre la visión artificial y el motor gráfico, las instrucciones no pueden depurarse, optimizarse en bloques lineales ni exportarse hacia otras arquitecturas de hardware.

### 1.2. Planteamiento del problema {#1.2.-planteamiento-del-problema}

* Problema general: ¿De qué manera el desarrollo de un compilador web de comandos gestuales permite validar sintáctica y semánticamente las instrucciones captadas por cámara y traducirlas a código intermedio optimizado para el control seguro de una mano robótica 3D?

* Problemas específicos:

  1. ¿Cómo estructurar una gramática libre de contexto sin ambigüedad que permita reconocer secuencias válidas de hasta 10 comandos cinemáticos base a partir de entradas gestuales?

  2. ¿Cómo garantizar la consistencia y confiabilidad de los movimientos articulares mediante una tabla de símbolos y reglas de verificación de tipos que detecten errores semánticos cuando los ángulos excedan el rango de ${0}^{\circ }$ a $9{0}^{\circ }$?

  3. ¿Cómo transformar el árbol sintáctico abstracto (AST) de los comandos validados en código intermedio de tres direcciones empleando variables temporales y un archivo de registro de cuádruplas?

  4. ¿Cómo optimizar los bloques básicos de instrucciones lineales para generar el código objeto final que articule los 18 grados de libertad de la mano robótica 3D en tiempo real?

### 1.3. Objetivos {#1.3.-objetivos}

#### 1.3.1. Objetivo general {#1.3.1.-objetivo-general}

Desarrollar un MVP de compilador web de comandos gestuales integrado a un entorno WebGL que valide sintáctica y semánticamente los gestos captados por cámara y genere código intermedio en cuádruplas para el control de una mano robótica 3D.

#### 1.3.2. Objetivos específicos {#1.3.2.-objetivos-específicos}

1. Diseñar una gramática libre de contexto no ambigua y su modelo de reconocimiento para construir árboles sintácticos de cadenas de prueba sobre un conjunto de 10 comandos cinemáticos base.  
2. Implementar una tabla de símbolos y reglas de verificación de tipos que detecten errores semánticos y restrinjan los ángulos de las 15 articulaciones falángicas al rango operativo de ${0}^{\circ }$ a $9{0}^{\circ }$.  
3. Generar código intermedio de tres direcciones a partir del árbol sintáctico abstracto (AST) utilizando variables temporales y un archivo de registro de cuádruplas.  
4. Aplicar técnicas de optimización sobre bloques básicos y propagación de constantes para emitir el código objeto que acciona el modelo 3D en el navegador.

### 1.4. Justificación {#1.4.-justificación}

#### 1.4.1. Técnica {#1.4.1.-técnica}

La incorporación de un pipeline de compilación desacopla la captura óptica de la ejecución cinemática. El análisis sintáctico asegura que solo se procesen rutinas bien formadas, mientras que el análisis semántico respaldado por la tabla de símbolos garantiza la consistencia y confiabilidad del programa al bloquear valores fuera de los límites mecánicos antes de su renderizado. Asimismo, el uso de código de tres direcciones en cuádruplas y la optimización de bloques básicos eliminan instrucciones redundantes entre fotogramas y producen un código estandarizado adaptable a controladores físicos.

#### 1.4.2. Social {#1.4.2.-social}

El proyecto entrega una herramienta accesible desde cualquier navegador web con cámara estándar, eliminando las barreras de acceso a laboratorios especializados de robótica. Permite que estudiantes y desarrolladores experimenten de forma interactiva con el control biónico, la validación de lenguajes de control y la simulación de interfaces gestuales aplicables a rehabilitación motora o traducción de señas.

#### 1.4.3. Financiera {#1.4.3.-financiera}

El enfoque de validación mediante un simulador 3D gobernado por un compilador reduce a cero el costo de adquisición de componentes físicos (servomotores, microcontroladores y piezas impresas en 3D) durante la etapa de diseño. Además, al detectar errores semánticos de rango y sobrecarga articular por software antes de emitir el código objeto, previene el desgaste prematuro y la rotura de actuadores en una futura implementación en hardware real.

[image1]: <data:image/png;base64,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>