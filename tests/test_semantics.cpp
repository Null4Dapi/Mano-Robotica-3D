#include "grammar/ParserDriver.hpp"
#include "modules/semantics/SymbolTable.hpp"
#include "modules/semantics/SemanticAnalyzer.hpp"
#include <iostream>
#include <sstream>
#include <cassert>

int main() {
    using namespace cyberhand::grammar;
    using namespace cyberhand::semantics;

    std::cout << "=== Test Semantic Analyzer & Symbol Table ===\n";

    ParserDriver driver;

    // Test a: Valid kinematic script passes with 0 errors
    {
        std::cout << "[Test a] Valid kinematic script... ";
        std::string valid_script = R"(
            rutina CerrarPinza() {
                angulo_base = 45;
                rotar pulgar falange_1 angulo_base grados;
                rotar indice falange_1 (angulo_base + 15) grados;
                esperar 250 ms;
            }
            INICIO
                velocidad = 2 * 10;
                rotar medio falange_1 velocidad;
                esperar 100 ms;
                repetir 3 veces {
                    rotar anular falange_1 30;
                    rotar menique falange_2 (velocidad - 5);
                }
            FIN
        )";
        auto ast = driver.parseString(valid_script);
        assert(ast != nullptr);
        assert(!driver.hasError());

        SymbolTable sym_table;
        SemanticAnalyzer analyzer(sym_table);
        bool ok = analyzer.analyze(*ast);
        if (!ok) {
            for (const auto& err : analyzer.errors()) {
                std::cerr << "  Error inesperado: " << err << "\n";
            }
        }
        assert(ok);
        assert(analyzer.errors().empty());
        std::cout << "PASSED (0 errores)\n";
    }

    // Test b: Script with duplicate variable declaration fails with duplicate error
    {
        std::cout << "[Test b] Duplicate variable declaration... ";
        std::string dup_script = R"(
            INICIO
                pos = 30;
                pos = 45;
                rotar pulgar falange_1 pos;
            FIN
        )";
        auto ast = driver.parseString(dup_script);
        assert(ast != nullptr);

        SemanticAnalyzer analyzer;
        bool ok = analyzer.analyze(*ast);
        assert(!ok);
        assert(analyzer.hasErrors());
        bool found_dup_msg = false;
        for (const auto& err : analyzer.errors()) {
            if (err.find("duplicada") != std::string::npos || err.find("Duplicate") != std::string::npos) {
                found_dup_msg = true;
            }
        }
        assert(found_dup_msg);
        std::cout << "PASSED (Detectado error de duplicado)\n";
    }

    // Test c: Script with angle > 90° (e.g., 120°) fails with biomechanical violation error
    {
        std::cout << "[Test c] Biomechanical range violation (> 90 deg)... ";
        std::string over_script = R"(
            INICIO
                rotar pulgar falange_1 120 grados;
            FIN
        )";
        auto ast = driver.parseString(over_script);
        assert(ast != nullptr);

        SemanticAnalyzer analyzer;
        bool ok = analyzer.analyze(*ast);
        assert(!ok);
        assert(analyzer.hasErrors());
        bool found_bio_msg = false;
        for (const auto& err : analyzer.errors()) {
            if (err.find("Violacion biomecanica") != std::string::npos) {
                found_bio_msg = true;
            }
        }
        assert(found_bio_msg);

        // Also test constant expression exceeding 90
        std::string expr_over_script = R"(
            INICIO
                base = 50;
                rotar indice falange_2 (base + 45);
            FIN
        )";
        auto ast2 = driver.parseString(expr_over_script);
        SemanticAnalyzer analyzer2;
        bool ok2 = analyzer2.analyze(*ast2);
        assert(!ok2);
        std::cout << "PASSED (Detectada violacion biomecanica [0, 90])\n";
    }

    // Test d: Script using undeclared variable fails with undeclared variable error
    {
        std::cout << "[Test d] Undeclared variable... ";
        std::string undef_script = R"(
            INICIO
                rotar pulgar falange_1 variable_fantasma;
            FIN
        )";
        auto ast = driver.parseString(undef_script);
        assert(ast != nullptr);

        SemanticAnalyzer analyzer;
        bool ok = analyzer.analyze(*ast);
        assert(!ok);
        assert(analyzer.hasErrors());
        bool found_undef = false;
        for (const auto& err : analyzer.errors()) {
            if (err.find("no declarada") != std::string::npos) {
                found_undef = true;
            }
        }
        assert(found_undef);
        std::cout << "PASSED (Detectada variable no declarada)\n";
    }

    // Test e: Verify table display output
    {
        std::cout << "[Test e] Formatted ASCII symbol table display... \n";
        std::string sample_script = R"(
            rutina MoverIndice() {
                angulo_paso = 30;
                rotar indice falange_1 angulo_paso;
            }
            INICIO
                total_movimientos = 5;
                esperar 200 ms;
            FIN
        )";
        auto ast = driver.parseString(sample_script);
        assert(ast != nullptr);

        SymbolTable sym_table;
        SemanticAnalyzer analyzer(sym_table);
        bool ok = analyzer.analyze(*ast);
        assert(ok);

        std::ostringstream oss;
        sym_table.printTable(oss);
        std::string output = oss.str();
        assert(!output.empty());
        assert(output.find("Identifier") != std::string::npos);
        assert(output.find("MoverIndice") != std::string::npos);
        assert(output.find("angulo_paso") != std::string::npos);
        assert(output.find("total_movimientos") != std::string::npos);
        std::cout << output;
        std::cout << "PASSED (Tabla ASCII validada)\n";
    }

    // Test f: Actuator validation and type mismatch
    {
        std::cout << "[Test f] Actuator validation & unit mismatch... ";
        std::string bad_actuator = "INICIO\n rotar brazo falange_1 45;\nFIN";
        auto ast = driver.parseString(bad_actuator);
        SemanticAnalyzer a1;
        assert(!a1.analyze(*ast));

        std::string type_mismatch = R"(
            INICIO
                tiempo_espera = 500;
                rotar pulgar falange_1 tiempo_espera;
            FIN
        )";
        auto ast2 = driver.parseString(type_mismatch);
        SemanticAnalyzer a2;
        assert(!a2.analyze(*ast2));
        std::cout << "PASSED (Actuadores invalidos y tipos incompatibles rechazados)\n";
    }

    std::cout << "\n>> TODAS LAS PRUEBAS SEMANTICAS PASARON CON EXITO <<\n";
    return 0;
}
