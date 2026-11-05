#include "grammar/ParserDriver.hpp"
#include "modules/intermediate/CodeGenerator.hpp"
#include <iostream>
#include <cassert>

int main() {
    using namespace cyberhand::grammar;
    using namespace cyberhand::intermediate;

    std::cout << "=== Test Intermediate Code Generator (Semana 14) ===\n";
    ParserDriver driver;
    CodeGenerator codegen;

    // Test 1: Arithmetic TAC decomposition with temporaries
    {
        std::cout << "[Test 1] Arithmetic expressions to TAC... ";
        auto ast = driver.parseString("INICIO\n x = 10 + 20 * 3;\nFIN\n");
        assert(ast != nullptr);
        codegen.generate(*ast);
        const auto& quads = codegen.quadruples();
        assert(quads.size() == 3);

        // quad 0: (*, 20, 3, t1)
        assert(quads[0].op == "*");
        assert(quads[0].arg1 == "20" && quads[0].arg2 == "3" && quads[0].result == "t1");
        // quad 1: (+, 10, t1, t2)
        assert(quads[1].op == "+");
        assert(quads[1].arg1 == "10" && quads[1].arg2 == "t1" && quads[1].result == "t2");
        // quad 2: (=, t2, _, x)
        assert(quads[2].op == "=");
        assert(quads[2].arg1 == "t2" && quads[2].result == "x");
        std::cout << "PASSED\n";
    }

    // Test 2: Kinematic commands (ROTAR, ESPERAR)
    {
        std::cout << "[Test 2] Rotations and wait commands... ";
        auto ast = driver.parseString("INICIO\n rotar pulgar falange_1 45;\n esperar 200 ms;\nFIN\n");
        assert(ast != nullptr);
        codegen.generate(*ast);
        const auto& quads = codegen.quadruples();
        assert(quads.size() == 2);
        assert(quads[0].op == "ROTAR" && quads[0].arg1 == "pulgar:falange_1" && quads[0].arg2 == "45");
        assert(quads[1].op == "ESPERAR" && quads[1].arg1 == "200" && quads[1].arg2 == "ms");
        std::cout << "PASSED\n";
    }

    // Test 3: Repeat loop with counter, labels, JUMP_IF_ZERO, decrement, JUMP
    {
        std::cout << "[Test 3] Repeat loop control flow... ";
        auto ast = driver.parseString("INICIO\n repetir 3 veces {\n rotar indice falange_2 30;\n }\nFIN\n");
        assert(ast != nullptr);
        codegen.generate(*ast);
        const auto& quads = codegen.quadruples();

        // Expect: (=, 3, _, t1), (LABEL, L1, _, _), (JUMP_IF_ZERO, t1, _, L2),
        // (ROTAR, ...), (-, t1, 1, t2), (=, t2, _, t1), (JUMP, L1, _, _), (LABEL, L2, _, _)
        assert(quads.size() == 8);
        assert(quads[0].op == "=" && quads[0].arg1 == "3" && quads[0].result == "t1");
        assert(quads[1].op == "LABEL" && quads[1].arg1 == "L1");
        assert(quads[2].op == "JUMP_IF_ZERO" && quads[2].arg1 == "t1" && quads[2].result == "L2");
        assert(quads[3].op == "ROTAR" && quads[3].arg1 == "indice:falange_2" && quads[3].arg2 == "30");
        assert(quads[4].op == "-" && quads[4].arg1 == "t1" && quads[4].arg2 == "1" && quads[4].result == "t2");
        assert(quads[5].op == "=" && quads[5].arg1 == "t2" && quads[5].result == "t1");
        assert(quads[6].op == "JUMP" && quads[6].arg1 == "L1");
        assert(quads[7].op == "LABEL" && quads[7].arg1 == "L2");
        std::cout << "PASSED\n";
    }

    // Test 4: Routine boundary marks (LABEL, RETURN)
    {
        std::cout << "[Test 4] Routine definitions... ";
        auto ast = driver.parseString("rutina CerrarPinza() {\n rotar menique falange_1 60;\n}\nINICIO\n esperar 100 ms;\nFIN\n");
        assert(ast != nullptr);
        codegen.generate(*ast);
        const auto& quads = codegen.quadruples();
        assert(quads.size() == 4);
        assert(quads[0].op == "LABEL" && quads[0].arg1 == "CerrarPinza");
        assert(quads[1].op == "ROTAR" && quads[1].arg1 == "menique:falange_1" && quads[1].arg2 == "60");
        assert(quads[2].op == "RETURN");
        assert(quads[3].op == "ESPERAR" && quads[3].arg1 == "100" && quads[3].arg2 == "ms");
        std::cout << "PASSED\n";
    }

    // Test 5: Table rendering (Quadruples and Triples)
    {
        std::cout << "[Test 5] ASCII table rendering and Triples index resolution... ";
        auto ast = driver.parseString(R"(
            INICIO
                angulo = 15 + 5 * 2;
                repetir 2 veces {
                    rotar pulgar falange_1 angulo;
                    esperar 50 ms;
                }
            FIN
        )");
        assert(ast != nullptr);
        codegen.generate(*ast);
        std::string quad_str = codegen.dumpQuadruples();
        std::string trip_str = codegen.dumpTriples();

        assert(quad_str.find("Index") != std::string::npos);
        assert(quad_str.find("Operador") != std::string::npos);
        assert(quad_str.find("Resultado") != std::string::npos);

        assert(trip_str.find("Index") != std::string::npos);
        assert(trip_str.find("Operador") != std::string::npos);
        assert(trip_str.find("(0)") != std::string::npos); // Triples references (0)

        std::cout << "PASSED\n";
        std::cout << "\n--- Tabla de Cuádruplas ---\n" << quad_str;
        std::cout << "\n--- Tabla de Triplas (Comparativa Teórica Semana 14) ---\n" << trip_str;
    }

    std::cout << "\nTodas las pruebas de Generación de Código Intermedio pasaron exitosamente.\n";
    return 0;
}
