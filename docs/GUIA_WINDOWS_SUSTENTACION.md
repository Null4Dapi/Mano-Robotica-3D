# CyberHand 3D — Guía de Descarga, Instalación en Windows y Sustentación

Manual de despliegue local en entornos Windows (Visual Studio Code / Dev-C++ / Terminal) y guión metodológico para la sustentación académica de los **18 Grados de Libertad (GDL)** y el compilador cinemático.

---

## 1. Descarga y Clonación del Proyecto

### Opción A: Mediante Git (Recomendado)
Abre PowerShell o Git Bash y clona el repositorio oficial:
```powershell
git clone https://github.com/Null4Dapi/Mano-Robotica-3D.git
cd Mano-Robotica-3D
```

### Opción B: Descarga Directa ZIP (Sin Git)
1. Ingresa a: [https://github.com/Null4Dapi/Mano-Robotica-3D](https://github.com/Null4Dapi/Mano-Robotica-3D)
2. Haz clic en el botón verde **`<> Code`** y selecciona **`Download ZIP`**.
3. Descomprime el archivo en tu carpeta de preferencia (por ejemplo `C:\Proyectos\Mano-Robotica-3D`).

---

## 2. Configuración del Entorno en Windows

El proyecto utiliza **C++20** estándar con **Flex** y **Bison** para el frontend, y **WebGL/Three.js** con un servidor HTTP ligero para la interfaz web.

### Alternativa 1: Visual Studio Code (Flujo Profesional)

1. **Instalar Extensiones de VS Code**:
   Abre VS Code (`Ctrl + Mayús + X`) e instala:
   - **`C/C++`** (`ms-vscode.cpptools`): Resaltado, IntelliSense y depuración C++20.
   - **`Live Server`** (`ritwickdey.liveserver`): Permite levantar la interfaz web 3D con un solo clic.

2. **Compilador C++20 (MinGW-w64 vía MSYS2 o w64devkit)**:
   - Si no cuentas con GCC 11+ en Windows, instala MSYS2 o descarga [w64devkit](https://github.com/skeeto/w64devkit/releases) (portable, sin instalación).
   - Asegúrate de que `g++`, `make`, `flex` y `bison` estén en el `PATH` de Windows.
   - Verifica en la terminal integrada de VS Code (`Ctrl + ñ`):
     ```powershell
     g++ --version
     make --version
     python --version
     ```

3. **Ejecución de la Web**:
   - Haz clic derecho sobre el archivo `web/index.html` y selecciona **`Open with Live Server`**.
   - O inicia el servidor integrado desde la terminal:
     ```powershell
     python -m http.server 8000 --directory web
     ```

---

### Alternativa 2: Dev-C++ (Típico en Laboratorios Universitarios)

Las computadoras universitarias suelen tener **Embarcadero Dev-C++ 6.3+**:

1. **Habilitar Estándar C++20**:
   - En Dev-C++, ve a **Herramientas (Tools) > Opciones del Compilador (Compiler Options)**.
   - En la pestaña **Configuración (Settings) > Generación de Código (Code Generation)**:
     - Busca **Language standard (-std=)** y selecciona **`ISO C++20`** (o añade `-std=c++20` en la caja de parámetros de compilación).
   - En la pestaña **Directorios (Directories) > Includes de C++**, añade la ruta absoluta de la carpeta `src` del proyecto (por ejemplo `C:\Proyectos\Mano-Robotica-3D\src`).

2. **Ejecución Directa mediante Binario Precompilado**:
   - Si la máquina de la universidad no tiene `flex` o `bison` instalados en la terminal, no te preocupes: el binario ejecutable compilado reside en `bin/cyberhand_compiler` (o `bin/cyberhand_compiler.exe` en Windows).
   - Puedes abrir una ventana de CMD o PowerShell directamente en la carpeta del proyecto y ejecutar los comandos de diagnóstico sin tener que recompilar las tablas de gramática.

---

### Alternativa 3: Instalación de Dependencias por Terminal (MSYS2)

Si tienes privilegios de instalación y deseas un entorno completo desde cero:
```powershell
# En consola MSYS2 UCRT64:
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-flex mingw-w64-ucrt-x86_64-bison python
```
Agrega `C:\msys64\ucrt64\bin` a tus Variables de Entorno del Sistema (`PATH`).

---

## 3. Comandos de Compilación y Ejecución

Desde la terminal en la raíz del proyecto:

| Acción | Comando | Resultado esperado |
| :--- | :--- | :--- |
| **Compilar el compilador C++20** | `make` (o `mingw32-make`) | Genera `bin/cyberhand_compiler` |
| **Ejecutar Suite de Pruebas E2E** | `make test` | Valida 8/8 pruebas unitarias y de integración |
| **Compilar Rutina Cinemática** | `./bin/cyberhand_compiler --compilar examples/rutina_valida.rob -o web/trayectoria.json` | Emite 43 frames con los 18 GDL |
| **Levantar Servidor Web 3D** | `make serve` o `python -m http.server 8000 --directory web` | Servidor disponible en `http://localhost:8000` |
| **Todo en uno (Build + Rutina + Web)** | `make run` | Flujo automatizado completo |

---

## 4. Guión de Sustentación para el Docente

Estructura didáctica recomendada para defender el proyecto de manera secuencial, demostrando solidez técnica tanto en el compilador como en el simulador 3D.

### Segmento 1: Sustentación del Compilador (Consola)

Abre la terminal en la raíz del proyecto y ejecuta los siguientes comandos en orden:

#### 1.1 Menú de Ayuda y Arquitectura Modular
```bash
./bin/cyberhand_compiler --help
```
* **Qué explicar al docente**: Destacar que el proyecto sigue **Screaming Architecture**: la estructura del código no depende de etiquetas académicas genéricas sino de la disciplina de compiladores (Lexer, Parser, Semántica, TAC, Optimización y Cinemática).

#### 1.2 Análisis Sintáctico Predictivo LL(1)
```bash
./bin/cyberhand_compiler --analisis-ll1
```
* **Qué explicar al docente**: Muestra el cálculo matemático formal de los conjuntos **PRIMERO** y **SIGUIENTE**, y la matriz de análisis $M[A, a]$ libre de conflictos para el subconjunto predictivo del DSL cinemático.

#### 1.3 Análisis Sintáctico Ascendente LR(0)
```bash
./bin/cyberhand_compiler --analisis-lr0
```
* **Qué explicar al docente**: Demuestra la construcción del autómata canónico con sus 10 estados, transiciones de cierre (*closure*), operaciones *Shift* / *Reduce* y las tablas de acción (**ACTION**) y saltos (**GOTO**).

#### 1.4 Análisis Semántico y Tabla de Símbolos
```bash
./bin/cyberhand_compiler --tabla-simbolos examples/rutina_valida.rob
```
* **Qué explicar al docente**: La tabla de símbolos valida ámbitos y tipos (`ANGLE` vs `TIME`). Comprueba que los actuadores pertenezcan al modelo anatómico y que ningún ángulo viole el rango biomecánico físico $[0^\circ, 90^\circ]$.

#### 1.5 Detección y Rechazo de Violaciones Semánticas
```bash
./bin/cyberhand_compiler --compilar examples/rutina_error_semantico.rob
```
* **Qué explicar al docente**: El compilador aborta con código de salida `1` cuando detecta un intento de hiperextensión ($> 90^\circ$ o $< 0^\circ$) o un actuador inexistente, previniendo daños mecánicos en la mano robótica.

#### 1.6 Código de Tres Direcciones (TAC: Cuádruplas y Triplas)
```bash
./bin/cyberhand_compiler --cuadruplas examples/rutina_valida.rob
```
* **Qué explicar al docente**: Exhibe la transformación del AST a cuádruplas `(operador, arg1, arg2, resultado)` y la representación compacta en memoria mediante tabla de triplas indexadas.

#### 1.7 Optimización en Bloques Básicos
```bash
./bin/cyberhand_compiler --optimizar examples/rutina_valida.rob
```
* **Qué explicar al docente**: Partición del código intermedio en líderes y bloques básicos, aplicando propagación de constantes y eliminación de código muerto (*DCE*).

---

### Segmento 2: Sustentación del Simulador 3D Web (18 GDL)

Abre el navegador en `http://localhost:8000`:

#### 2.1 Demostración Aislada de los 18 GDL (Panel Manual)
1. Haz clic en el ícono de **Mano** en la barra lateral derecha para abrir el cajón de **Articulaciones**.
2. Explica y mueve los deslizadores frente al docente:
   - **Pulgar**: Nudillo Proximal (MCP), Falange Media (PIP) y Falange Distal (DIP) $\rightarrow$ **3 GDL**.
   - **Índice**: MCP, PIP y DIP $\rightarrow$ **3 GDL**.
   - **Medio**: MCP, PIP y DIP $\rightarrow$ **3 GDL**.
   - **Anular**: MCP, PIP y DIP $\rightarrow$ **3 GDL**.
   - **Meñique**: MCP, PIP y DIP $\rightarrow$ **3 GDL**.
   - *Subtotal dedos*: $5 \times 3 = \mathbf{15\ GDL}$.
   - **Muñeca**: 
     - **Pitch (Eje Y)**: Flexión/extensión palmar vertical.
     - **Roll (Eje X)**: Pronación/supinación lateral.
     - **Yaw (Eje Z)**: Desviación radial y cubital.
   - *Total articulaciones*: $15 + 3 = \mathbf{18\ GDL\ totales}$.

#### 2.2 Sincronización de Poses Predefinidas
- Haz clic en los botones de pose: **Reposo**, **Abierta**, **Puño**, **Pinza**, **Paz**, **OK** y **Señalar**.
- Muestra cómo los 18 deslizadores se actualizan automáticamente en tiempo real reflejando los ángulos matemáticos de cada postura.

#### 2.3 Reproducción de la Trayectoria Cinemática (43 Frames)
1. Presiona el botón de **Play** ($\blacktriangleright$) en la barra de tiempo inferior.
2. Explica la coreografía cinemática de 43 frames generada por el compilador:
   - **Fase 1 (Frames 1–15)**: Calibración individual dedo por dedo (se observa la flexión aislada de cada falange distal, medial y proximal).
   - **Fase 2 (Frames 16–22)**: Calibración tridimensional de la muñeca (se aprecian los tres ejes de rotación del carpo).
   - **Fase 3 (Frames 23–32)**: Cascada biomecánica en onda (*finger wave* desde el meñique hasta el pulgar y retorno armónico).
   - **Fase 4 (Frames 33–43)**: Gestos funcionales de agarre (puño cerrado completo, pinza fina de precisión y señal de paz).
3. Muestra que puedes arrastrar el cursor del deslizador a cualquier fotograma específico (por ejemplo `Frame 20 / 43`) o usar los botones de paso `<` y `>` para pausar y sustentar la postura articular exacta.

#### 2.4 Control de Cámaras e Inspección de Malla
- Alterna entre las vistas de cámara: **Isométrica**, **Superior**, **Lateral** y **Frontal**.
- Haz clic en **Malla (Wireframe)** en la esquina superior derecha para mostrarle al docente el esqueleto geométrico poligonal, las rótulas esféricas y los vástagos lineales de los actuadores.
