#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <cctype>

#include "grammar/ParserDriver.hpp"
#include "core/TUI.hpp"
#include "modules/ll1_analyzer/LL1Table.hpp"
#include "modules/lr0_analyzer/LR0Automaton.hpp"
#include "modules/semantics/SemanticAnalyzer.hpp"
#include "modules/semantics/SymbolTable.hpp"
#include "modules/intermediate/CodeGenerator.hpp"
#include "modules/optimizer/Optimizer.hpp"

namespace {

void printHelp() {
    using namespace cyberhand::ui;
    std::cout << "\n" << GRAY << TOP_L << hRule(78) << TOP_R << RESET << "\n"
              << GRAY << V_LINE << " " << BOLD << WHITE << "CYBERHAND COMPILER & KINEMATIC VIRTUAL MACHINE (C++20)" << RESET
              << std::string(24, ' ') << GRAY << V_LINE << RESET << "\n"
              << GRAY << V_LINE << " " << DIM << "Sistema Integral de Compilación y Control Cinemático · 18 GDL" << RESET
              << std::string(13, ' ') << GRAY << V_LINE << RESET << "\n"
              << GRAY << BOT_L << hRule(78) << BOT_R << RESET << "\n\n"
              << BOLD << WHITE << "USO:\n" << RESET
              << "  bin/cyberhand_compiler " << CYAN << "[OPCIÓN] [PARÁMETROS]" << RESET << "\n\n"
              << BOLD << WHITE << "OPCIONES DISPONIBLES:\n" << RESET
              << "  " << CYAN << "-h, --help" << RESET << "                     Muestra este menú de ayuda de la arquitectura.\n"
              << "  " << CYAN << "--analisis-ll1" << RESET << "                 Analizador predictivo LL(1): FIRST, FOLLOW y tabla.\n"
              << "  " << CYAN << "--analisis-lr0" << RESET << "                 Analizador ascendente LR(0): colección canónica y ACTION/GOTO.\n"
              << "  " << CYAN << "--tabla-simbolos <archivo.rob>" << RESET << " Tabla de Símbolos con ámbitos y actuadores.\n"
              << "  " << CYAN << "--cuadruplas <archivo.rob>" << RESET << "     Código Intermedio (TAC), Cuádruplas y Triplas.\n"
              << "  " << CYAN << "--optimizar <archivo.rob>" << RESET << "      Optimización en bloques básicos (folding, DCE).\n"
              << "  " << CYAN << "--compilar <archivo.rob> [-o salida.json]" << RESET << "\n"
              << "                                 Pipeline completo: parsing, semántica [0°, 90°], TAC,\n"
              << "                                 optimización y exportación de trayectoria JSON.\n"
              << "                                 (Salida por defecto: web/trayectoria.json)\n\n"
              << BOLD << WHITE << "SUBSISTEMAS DEL PIPELINE CINEMÁTICO:\n" << RESET
              << "  " << GRAY << "•" << RESET << " " << BOLD << "Frontend LL(1):" << RESET << " Análisis Sintáctico Predictivo (FIRST, FOLLOW, Tabla M[A,a]).\n"
              << "  " << GRAY << "•" << RESET << " " << BOLD << "Frontend LR(0):" << RESET << " Análisis Sintáctico Ascendente (Colección canónica, Shift/Reduce).\n"
              << "  " << GRAY << "•" << RESET << " " << BOLD << "Parser & AST:" << RESET << " Generadores Flex & Bison generando el AST tipado en C++.\n"
              << "  " << GRAY << "•" << RESET << " " << BOLD << "Motor Semántico:" << RESET << " Tabla de Símbolos y Validación Biomecánica (Rangos [0°, 90°]).\n"
              << "  " << GRAY << "•" << RESET << " " << BOLD << "Representación IR:" << RESET << " Generación de Código Intermedio (TAC en Cuádruplas y Triplas).\n"
              << "  " << GRAY << "•" << RESET << " " << BOLD << "Optimizador:" << RESET << " Bloques Básicos, DCE y Emisión de Trayectoria 3D (18 GDL).\n";
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

void printHeader(std::string_view title, std::string_view subtitle = "") {
    using namespace cyberhand::ui;
    std::cout << "\n" << GRAY << TOP_L << hRule(78) << TOP_R << "\n"
              << V_LINE << " " << BOLD << CYAN << title << RESET;
    size_t title_len = title.size() + 1;
    if (title_len < 78) std::cout << std::string(78 - title_len, ' ');
    std::cout << GRAY << V_LINE << "\n";
    if (!subtitle.empty()) {
        std::cout << V_LINE << " " << DIM << subtitle << RESET;
        size_t sub_len = subtitle.size() + 1;
        if (sub_len < 78) std::cout << std::string(78 - sub_len, ' ');
        std::cout << GRAY << V_LINE << "\n";
    }
    std::cout << BOT_L << hRule(78) << BOT_R << RESET << "\n\n";
}

int runLL1Analysis() {
    printHeader("ANALIZADOR SINTÁCTICO PREDICTIVO LL(1)");

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
    printHeader("ANALIZADOR SINTÁCTICO ASCENDENTE LR(0)");

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
    printHeader("TABLA DE SÍMBOLOS Y GESTIÓN DE ÁMBITOS", "Archivo: " + filepath);

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
    printHeader("GENERACIÓN DE CÓDIGO INTERMEDIO (TAC, CUÁDRUPLAS Y TRIPLAS)", "Archivo: " + filepath);

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

    std::cout << ">> 2. TABLA DE TRIPLAS (REPRESENTACIÓN COMPACTA EN MEMORIA):\n"
              << codegen.dumpTriples() << "\n";

    std::cout << ">> [ÉXITO]: " << codegen.quadruples().size() << " cuádruplas generadas exitosamente.\n";
    return 0;
}

int runOptimize(const std::string& filepath) {
    printHeader("OPTIMIZACIÓN DE BLOQUES BÁSICOS Y CÓDIGO INTERMEDIO", "Archivo: " + filepath);

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

    std::cout << "\n" << cyberhand::ui::GRAY << cyberhand::ui::TOP_L << cyberhand::ui::hRule(78) << cyberhand::ui::TOP_R << "\n"
              << cyberhand::ui::V_LINE << " " << cyberhand::ui::BOLD << cyberhand::ui::GREEN << "COMPILACIÓN CINEMÁTICA EXITOSA (18 GDL)" << cyberhand::ui::RESET
              << std::string(38, ' ') << cyberhand::ui::GRAY << cyberhand::ui::V_LINE << "\n"
              << cyberhand::ui::T_RIGHT << cyberhand::ui::hRule(78) << cyberhand::ui::T_LEFT << "\n"
              << cyberhand::ui::V_LINE << "  " << cyberhand::ui::WHITE << "Archivo fuente:          " << cyberhand::ui::RESET << std::setw(49) << std::left << filepath << cyberhand::ui::GRAY << cyberhand::ui::V_LINE << "\n"
              << cyberhand::ui::V_LINE << "  " << cyberhand::ui::WHITE << "Archivo de trayectoria:  " << cyberhand::ui::RESET << std::setw(49) << std::left << out_path << cyberhand::ui::GRAY << cyberhand::ui::V_LINE << "\n"
              << cyberhand::ui::V_LINE << "  " << cyberhand::ui::WHITE << "Instrucciones TAC orig:  " << cyberhand::ui::CYAN << std::setw(49) << std::left << codegen.quadruples().size() << cyberhand::ui::GRAY << cyberhand::ui::V_LINE << "\n"
              << cyberhand::ui::V_LINE << "  " << cyberhand::ui::WHITE << "Instrucciones optimiz:   " << cyberhand::ui::GREEN << std::setw(49) << std::left << opt_quads.size() << cyberhand::ui::GRAY << cyberhand::ui::V_LINE << "\n"
              << cyberhand::ui::V_LINE << "  " << cyberhand::ui::WHITE << "Porcentaje de reducción: " << cyberhand::ui::YELLOW << std::setw(49) << std::left << (std::to_string(optimizer.reductionPercentage()) + "%") << cyberhand::ui::GRAY << cyberhand::ui::V_LINE << "\n"
              << cyberhand::ui::V_LINE << "  " << cyberhand::ui::WHITE << "Grados de libertad (GDL):" << cyberhand::ui::RESET << std::setw(49) << std::left << "18 articulaciones (dedos + muñeca)" << cyberhand::ui::GRAY << cyberhand::ui::V_LINE << "\n"
              << cyberhand::ui::V_LINE << "  " << cyberhand::ui::WHITE << "Estado para visor web 3D:" << cyberhand::ui::GREEN << std::setw(49) << std::left << "LISTO PARA REPRODUCCIÓN" << cyberhand::ui::GRAY << cyberhand::ui::V_LINE << "\n"
              << cyberhand::ui::BOT_L << cyberhand::ui::hRule(78) << cyberhand::ui::BOT_R << cyberhand::ui::RESET << "\n\n";

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
