#include "grammar/ParserDriver.hpp"
#include <iostream>
#include <cassert>
#include <chrono>
#include <fstream>
#include <filesystem>

int main() {
    using namespace cyberhand::grammar;
    using namespace cyberhand::core;

    std::cout << "=== Test Flex & Bison Automated Parser (Semana 12) ===\n";

    ParserDriver driver;

    // 1. Script cinemático completo con rutinas, asignaciones, rotaciones, expresiones y repetir
    std::string script = R"(
        // Rutina para cerrar pinza biomecánica
        rutina CerrarPinza() {
            angulo_base = 45;
            rotar pulgar falange_1 angulo_base grados;
            rotar indice falange_1 (angulo_base + 15) grados;
            esperar 250 ms;
        }

        /* Bloque principal de ejecución */
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

    // 2. Medición de rendimiento cinemático (< 50ms)
    auto t_start = std::chrono::high_resolution_clock::now();
    auto ast = driver.parseString(script);
    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

    std::cout << "Tiempo de parseo: " << elapsed_ms << " ms\n";
    assert(elapsed_ms < 50.0);

    // 3. Verificación de integridad del AST
    assert(ast != nullptr);
    assert(!driver.hasError());
    assert(ast->routines().size() == 1);
    assert(ast->routines()[0]->name() == "CerrarPinza");
    assert(ast->routines()[0]->statements().size() == 4);

    assert(ast->main_statements().size() == 4);
    auto* repeat_node = dynamic_cast<RepeatCmdNode*>(ast->main_statements()[3].get());
    assert(repeat_node != nullptr);
    assert(repeat_node->body().size() == 2);
    std::cout << "AST verificado con exito: 1 rutina, 4 sentencias principales, 2 en bucle.\n";

    // 4. Verificación de captura de errores sintácticos con línea y columna
    std::string invalid_script = "inicio\n  rotar 45 pulgar;\nfin";
    auto bad_ast = driver.parseString(invalid_script);
    assert(bad_ast == nullptr);
    assert(driver.hasError());
    assert(driver.errorLine() == 2);
    std::cout << "Error sintactico detectado correctamente en L:" << driver.errorLine()
              << " C:" << driver.errorColumn() << " -> " << driver.errorMessage() << "\n";

    // 5. Verificación de lectura de archivo físico (parseFile)
    std::string tmp_file = (std::filesystem::temp_directory_path() / "cyberhand_test.rob").string();
    {
        std::ofstream ofs(tmp_file);
        ofs << "inicio\n rotar pulgar falange_1 90;\nfin\n";
    }
    auto file_ast = driver.parseFile(tmp_file);
    std::filesystem::remove(tmp_file);
    assert(file_ast != nullptr);
    assert(file_ast->main_statements().size() == 1);

    // Verificación de archivo inexistente
    auto missing_ast = driver.parseFile("/tmp/archivo_inexistente_12345.rob");
    assert(missing_ast == nullptr);
    assert(driver.hasError());

    std::cout << ">> TODAS LAS PRUEBAS BISON/FLEX PASARON CON EXITO <<\n";
    return 0;
}
