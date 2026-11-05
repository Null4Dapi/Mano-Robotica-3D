#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <cctype>

#include "grammar/ParserDriver.hpp"
#include "modules/ll1_analyzer/LL1Table.hpp"
#include "modules/lr0_analyzer/LR0Automaton.hpp"
#include "modules/semantics/SemanticAnalyzer.hpp"
#include "modules/semantics/SymbolTable.hpp"
#include "modules/intermediate/CodeGenerator.hpp"
#include "modules/optimizer/Optimizer.hpp"

namespace {

void printHelp() {
    std::cout << R"(================================================================================
 CYBERHAND COMPILER & KINEMATIC VIRTUAL MACHINE (C++20)
 Asignatura: Autómatas y Compiladores - Semanas 10 a 15
 Plataforma: Mano Robótica Articulada 3D (18 GDL)
================================================================================

USO:
  bin/cyberhand_compiler [OPCIÓN] [PARÁMETROS]

OPCIONES DISPONIBLES:
  -h, --help                     Muestra este menú de ayuda con el temario académico.
  --analisis-ll1                 (Semana 10) Ejecuta el analizador predictivo LL(1):
                                 conjuntos PRIMERO, SIGUIENTE, tabla sintáctica y traza.
  --analisis-lr0                 (Semana 11) Ejecuta el analizador ascendente LR(0):
                                 10 estados canónicos, tablas ACTION/GOTO y traza.
  --tabla-simbolos <archivo.rob> (Semana 13) Muestra la Tabla de Símbolos con ámbitos,
                                 tipos, categorías y actuadores cinemáticos.
  --cuadruplas <archivo.rob>     (Semana 14) Genera Código Intermedio (TAC) y muestra
                                 las tablas comparativas de Cuádruplas y Triplas.
  --optimizar <archivo.rob>      (Semana 15) Ejecuta optimización en bloques básicos
                                 (folding, propagación, DCE) y emite reporte.
  --compilar <archivo.rob> [-o salida.json]
                                 Compilación completa end-to-end:
                                 Parseo AST (Semana 12), Semántica [0°, 90°] (Semana 13),
                                 TAC Cuádruplas (Semana 14), Optimización (Semana 15)
                                 y exportación de trayectoria JSON para visor web 3D.
                                 (Salida por defecto: web/trayectoria.json)

DESCRIPCIÓN ACADÉMICA (SEMANAS 10 A 15):
  * Semana 10: Análisis Sintáctico Predictivo LL(1).
    Cálculo formal de conjuntos PRIMERO y SIGUIENTE, construcción de tabla
    predictiva M[A, a] y justificación de ausencia de conflictos.
  * Semana 11: Análisis Sintáctico Ascendente LR(0).
    Gramática aumentada, autómata canónico (10 estados vía CLAUSURA e IR_A),
    tablas ACTION / GOTO y simulación Shift/Reduce.
  * Semana 12: Generadores de Analizadores Sintácticos (Flex & Bison).
    Scanner léxico en scanner.l y Parser LALR(1) en parser.y generando el AST.
  * Semana 13: Análisis Semántico y Tabla de Símbolos.
    Gestión de ámbitos local/global, verificación de tipos y restricción física
    biomecánica en rango [0°, 90°].
  * Semana 14: Generación de Código Intermedio.
    Traducción del AST a Código de Tres Direcciones (TAC), matriz de Cuádruplas
    y Triplas con temporales y etiquetas.
  * Semana 15: Optimización y Código Objeto.
    Particionado en bloques básicos, plegado de constantes, propagación de copias,
    DCE cinemático (reducción >= 15%) y emisión de trayectoria JSON para Three.js.
================================================================================
)";
}

std::string normalizeRobScript(const std::string& input) {
    std::string result = input;

    // 1. Omitir directiva MOTOR <identificador>;
    std::regex motor_re(R"(\bMOTOR\s+[a-zA-Z0-9_]+\s*;)", std::regex_constants::icase);
    result = std::regex_replace(result, motor_re, "");

    // 2. Normalizar sintaxis DEDO <dedo> FALANGE <falange> ROTAR <expr>;
    std::regex dedo_re(R"(\bDEDO\s+([a-zA-Z0-9_]+)\s+FALANGE\s+([a-zA-Z0-9_]+)\s+ROTAR\s+([^;]+);)", std::regex_constants::icase);

    std::string transformed;
    std::smatch m;
    std::string::const_iterator searchStart(result.cbegin());
    while (std::regex_search(searchStart, result.cend(), m, dedo_re)) {
        transformed.append(searchStart, searchStart + m.position());
        std::string finger = m[1].str();
        std::string phalanx = m[2].str();
        std::string expr = m[3].str();

        std::string p_lower = phalanx;
        for (char& c : p_lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (p_lower == "proximal") p_lower = "falange_1";
        else if (p_lower == "medial") p_lower = "falange_2";
        else if (p_lower == "distal") p_lower = "falange_3";

        transformed += "rotar " + finger + " " + p_lower + " " + expr + ";";
        searchStart += m.position() + m.length();
    }
    transformed.append(searchStart, result.cend());
    return transformed;
}

std::string readFileContents(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[ERROR]: No se pudo abrir el archivo fuente: " << filepath << "\n";
        return {};
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int runLL1Analysis() {
    std::cout << "\n================================================================================\n"
              << " [SEMANA 10] ANALIZADOR SINTÁCTICO PREDICTIVO LL(1)\n"
              << "================================================================================\n\n";

    auto grammar = cyberhand::syntax::ll1::Grammar::createKinematicGrammar();
    cyberhand::syntax::ll1::LL1Table analyzer(grammar);

    std::cout << ">> 1. CONJUNTOS PRIMERO (FIRST) Y SIGUIENTE (FOLLOW):\n";
    analyzer.printFirstFollow();

    std::cout << "\n>> 2. TABLA PREDICTIVA LL(1):\n";
    analyzer.printTable();

    std::cout << "\n>> 3. VERIFICACIÓN FORMAL DE CONFLICTOS:\n";
    std::cout << "   Total de conflictos detectados: " << analyzer.conflictCount() << "\n";
    std::cout << "   Gramática LL(1) válida: " << (analyzer.isLL1() ? "SÍ (Libre de ambigüedad)" : "NO") << "\n";

    std::cout << "\n>> 4. TRAZA DE SIMULACIÓN LL(1) ('rotar pulgar falange_1 45'):\n";
    std::vector<std::string> valid_tokens = {"rotar", "pulgar", "falange_1", "45"};
    auto result = analyzer.simulate(valid_tokens);
    for (const auto& step : result.trace) {
        std::cout << "   " << step << "\n";
    }

    if (result.success) {
        std::cout << "\n>> [ÉXITO]: Análisis sintáctico predictivo LL(1) completado con éxito.\n";
        return 0;
    } else {
        std::cerr << "\n>> [ERROR]: Fallo en simulación LL(1): " << result.error_message << "\n";
        return 1;
    }
}

int runLR0Analysis() {
    std::cout << "\n================================================================================\n"
              << " [SEMANA 11] ANALIZADOR SINTÁCTICO ASCENDENTE LR(0)\n"
              << "================================================================================\n\n";

    auto grammar = cyberhand::syntax::lr0::Grammar::createKinematicGrammar();
    cyberhand::syntax::lr0::LR0Automaton automaton(grammar);

    std::cout << ">> 1. COLECCIÓN CANÓNICA DE ESTADOS LR(0) (" << automaton.getStates().size() << " ESTADOS):\n";
    automaton.printStates();

    std::cout << "\n>> 2. TABLAS ACTION Y GOTO:\n";
    automaton.printTable();

    std::cout << "\n>> 3. VERIFICACIÓN DE CONFLICTOS (SHIFT/REDUCE O REDUCE/REDUCE):\n";
    std::cout << "   Conflictos detectados: " << automaton.getConflicts().size() << "\n";
    std::cout << "   Gramática LR(0) pura: " << (!automaton.hasConflicts() ? "SÍ (Autómata determinista)" : "NO") << "\n";

    std::cout << "\n>> 4. TRAZA DE SIMULACIÓN ASCENDENTE LR(0) (Shift/Reduce):\n";
    std::vector<std::string> valid_tokens = {"rotar", "pulgar", "numero"};
    auto parse_res = automaton.simulate(valid_tokens);
    automaton.printTrace(parse_res);

    if (parse_res.success) {
        std::cout << "\n>> [ÉXITO]: Análisis sintáctico ascendente LR(0) completado con éxito.\n";
        return 0;
    } else {
        std::cerr << "\n>> [ERROR]: Secuencia rechazada: " << parse_res.error_message << "\n";
        return 1;
    }
}

int runSymbolTable(const std::string& filepath) {
    std::cout << "\n================================================================================\n"
              << " [SEMANA 13] TABLA DE SÍMBOLOS Y GESTIÓN DE ÁMBITOS\n"
              << " Archivo: " << filepath << "\n"
              << "================================================================================\n\n";

    std::string raw = readFileContents(filepath);
    if (raw.empty()) return 1;

    std::string normalized = normalizeRobScript(raw);
    cyberhand::grammar::ParserDriver driver;
    auto ast = driver.parseString(normalized);
    if (!ast) {
        std::cerr << "[ERROR SINTÁCTICO] L" << driver.errorLine() << ":C" << driver.errorColumn()
                  << " - " << driver.errorMessage() << "\n";
        return 1;
    }

    cyberhand::semantics::SymbolTable sym_table;
    cyberhand::semantics::SemanticAnalyzer analyzer(sym_table);
    bool ok = analyzer.analyze(*ast);

    std::cout << ">> TABLA DE SÍMBOLOS GENERADA:\n";
    sym_table.printTable(std::cout);

    if (!ok) {
        std::cerr << "\n>> [ERRORES SEMÁNTICOS DETECTADOS]:\n";
        for (const auto& err : analyzer.errors()) {
            std::cerr << "  - " << err << "\n";
        }
        return 1;
    }

    std::cout << "\n>> [ÉXITO]: Tabla de símbolos validada correctamente sin errores.\n";
    return 0;
}

int runQuadruples(const std::string& filepath) {
    std::cout << "\n================================================================================\n"
              << " [SEMANA 14] GENERACIÓN DE CÓDIGO INTERMEDIO (TAC, CUÁDRUPLAS Y TRIPLAS)\n"
              << " Archivo: " << filepath << "\n"
              << "================================================================================\n\n";

    std::string raw = readFileContents(filepath);
    if (raw.empty()) return 1;

    std::string normalized = normalizeRobScript(raw);
    cyberhand::grammar::ParserDriver driver;
    auto ast = driver.parseString(normalized);
    if (!ast) {
        std::cerr << "[ERROR SINTÁCTICO] L" << driver.errorLine() << ":C" << driver.errorColumn()
                  << " - " << driver.errorMessage() << "\n";
        return 1;
    }

    cyberhand::semantics::SymbolTable sym_table;
    cyberhand::semantics::SemanticAnalyzer analyzer(sym_table);
    if (!analyzer.analyze(*ast)) {
        std::cerr << "[ERROR SEMÁNTICO] No se puede generar código intermedio por errores semánticos:\n";
        for (const auto& err : analyzer.errors()) {
            std::cerr << "  - " << err << "\n";
        }
        return 1;
    }

    cyberhand::intermediate::CodeGenerator codegen;
    codegen.generate(*ast);

    std::cout << ">> 1. TABLA DE CUÁDRUPLAS (OP, ARG1, ARG2, RES):\n"
              << codegen.dumpQuadruples() << "\n";

    std::cout << ">> 2. TABLA DE TRIPLAS (COMPARATIVA TEÓRICA SEMANA 14):\n"
              << codegen.dumpTriples() << "\n";

    std::cout << ">> [ÉXITO]: " << codegen.quadruples().size() << " cuádruplas generadas exitosamente.\n";
    return 0;
}

int runOptimize(const std::string& filepath) {
    std::cout << "\n================================================================================\n"
              << " [SEMANA 15] OPTIMIZACIÓN DE BLOQUES BÁSICOS Y CÓDIGO INTERMEDIO\n"
              << " Archivo: " << filepath << "\n"
              << "================================================================================\n\n";

    std::string raw = readFileContents(filepath);
    if (raw.empty()) return 1;

    std::string normalized = normalizeRobScript(raw);
    cyberhand::grammar::ParserDriver driver;
    auto ast = driver.parseString(normalized);
    if (!ast) {
        std::cerr << "[ERROR SINTÁCTICO] L" << driver.errorLine() << ":C" << driver.errorColumn()
                  << " - " << driver.errorMessage() << "\n";
        return 1;
    }

    cyberhand::semantics::SymbolTable sym_table;
    cyberhand::semantics::SemanticAnalyzer analyzer(sym_table);
    if (!analyzer.analyze(*ast)) {
        std::cerr << "[ERROR SEMÁNTICO] Fallo en verificación semántica:\n";
        for (const auto& err : analyzer.errors()) {
            std::cerr << "  - " << err << "\n";
        }
        return 1;
    }

    cyberhand::intermediate::CodeGenerator codegen;
    codegen.generate(*ast);

    cyberhand::optimizer::Optimizer optimizer;
    optimizer.optimize(codegen.quadruples());
    optimizer.printOptimizationReport(std::cout);

    std::cout << "\n>> [ÉXITO]: Optimización en bloques básicos completada.\n";
    return 0;
}

int runCompile(const std::string& filepath, const std::string& out_path) {
    std::string raw = readFileContents(filepath);
    if (raw.empty()) return 1;

    std::string normalized = normalizeRobScript(raw);
    cyberhand::grammar::ParserDriver driver;
    auto ast = driver.parseString(normalized);
    if (!ast) {
        std::cerr << "[ERROR SINTÁCTICO] L" << driver.errorLine() << ":C" << driver.errorColumn()
                  << " - " << driver.errorMessage() << "\n";
        return 1;
    }

    cyberhand::semantics::SymbolTable sym_table;
    cyberhand::semantics::SemanticAnalyzer analyzer(sym_table);
    if (!analyzer.analyze(*ast)) {
        std::cerr << "[ERROR SEMÁNTICO] Se encontraron violaciones en el programa cinemático:\n";
        for (const auto& err : analyzer.errors()) {
            std::cerr << "  - " << err << "\n";
        }
        return 1;
    }

    cyberhand::intermediate::CodeGenerator codegen;
    codegen.generate(*ast);

    cyberhand::optimizer::Optimizer optimizer;
    auto opt_quads = optimizer.optimize(codegen.quadruples());

    std::filesystem::path out_file_path(out_path);
    if (out_file_path.has_parent_path()) {
        std::filesystem::create_directories(out_file_path.parent_path());
    }

    if (!optimizer.exportTrajectoryJson(opt_quads, out_path)) {
        std::cerr << "[ERROR]: No se pudo escribir la trayectoria en el archivo: " << out_path << "\n";
        return 1;
    }

    std::cout << "\n================================================================================\n"
              << " COMPILACIÓN CINEMÁTICA EXITOSA (SEMANA 10 A 15)\n"
              << "================================================================================\n"
              << " Archivo fuente:           " << filepath << "\n"
              << " Archivo de trayectoria:   " << out_path << "\n"
              << " Instrucciones TAC orig:   " << codegen.quadruples().size() << "\n"
              << " Instrucciones optimizadas:" << opt_quads.size() << "\n"
              << " Porcentaje de reducción:  " << optimizer.reductionPercentage() << "%\n"
              << " Grados de libertad (GDL): 18 articulaciones (dedos + muñeca)\n"
              << " Estado para visor web 3D: LISTO PARA REPRODUCCIÓN\n"
              << "================================================================================\n\n";

    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string flag = argv[1];

    if (flag == "-h" || flag == "--help") {
        printHelp();
        return 0;
    }

    if (flag == "--analisis-ll1") {
        return runLL1Analysis();
    }

    if (flag == "--analisis-lr0") {
        return runLR0Analysis();
    }

    if (flag == "--tabla-simbolos") {
        if (argc < 3) {
            std::cerr << "[ERROR]: Debe especificar el archivo fuente .rob\n";
            std::cerr << "Uso: " << argv[0] << " --tabla-simbolos <archivo.rob>\n";
            return 1;
        }
        return runSymbolTable(argv[2]);
    }

    if (flag == "--cuadruplas") {
        if (argc < 3) {
            std::cerr << "[ERROR]: Debe especificar el archivo fuente .rob\n";
            std::cerr << "Uso: " << argv[0] << " --cuadruplas <archivo.rob>\n";
            return 1;
        }
        return runQuadruples(argv[2]);
    }

    if (flag == "--optimizar") {
        if (argc < 3) {
            std::cerr << "[ERROR]: Debe especificar el archivo fuente .rob\n";
            std::cerr << "Uso: " << argv[0] << " --optimizar <archivo.rob>\n";
            return 1;
        }
        return runOptimize(argv[2]);
    }

    if (flag == "--compilar") {
        if (argc < 3) {
            std::cerr << "[ERROR]: Debe especificar el archivo fuente .rob\n";
            std::cerr << "Uso: " << argv[0] << " --compilar <archivo.rob> [-o salida.json]\n";
            return 1;
        }

        std::string src_path;
        std::string out_path = "web/trayectoria.json";

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o") {
                if (i + 1 < argc) {
                    out_path = argv[++i];
                } else {
                    std::cerr << "[ERROR]: La opción -o requiere un archivo de destino.\n";
                    return 1;
                }
            } else {
                src_path = arg;
            }
        }

        if (src_path.empty()) {
            std::cerr << "[ERROR]: Archivo fuente no especificado.\n";
            return 1;
        }

        return runCompile(src_path, out_path);
    }

    std::cerr << "[ERROR]: Opción desconocida '" << flag << "'.\n";
    std::cerr << "Ejecute '" << argv[0] << " --help' para ver las opciones disponibles.\n";
    return 1;
}
