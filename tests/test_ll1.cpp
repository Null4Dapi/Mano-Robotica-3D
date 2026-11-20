#include "modules/ll1_analyzer/LL1Table.hpp"
#include <iostream>
#include <cassert>

int main() {
    using namespace cyberhand::syntax::ll1;

    std::cout << "=== Test LL(1) Analyzer ===\n";
    Grammar grammar = Grammar::createKinematicGrammar();
    LL1Table analyzer(grammar);

    // 1. Verificar PRIMERO (FIRST)
    const auto& first = analyzer.getFirstSets();
    assert(first.at("Grados").count("numero") == 1);
    assert(first.at("Falange").count("falange_1") == 1);
    assert(first.at("Falange").count("falange_2") == 1);
    assert(first.at("Dedo").count("pulgar") == 1);
    assert(first.at("Rotar").count("rotar") == 1);
    assert(first.at("Esperar").count("esperar") == 1);
    assert(first.at("MasDeclaraciones").count(std::string(EPSILON)) == 1);

    // 2. Verificar SIGUIENTE (FOLLOW)
    const auto& follow = analyzer.getFollowSets();
    assert(follow.at("Programa").count(std::string(END_MARKER)) == 1);
    assert(follow.at("MasDeclaraciones").count(std::string(END_MARKER)) == 1);
    assert(follow.at("Dedo").count("falange_1") == 1);
    assert(follow.at("Falange").count("numero") == 1);

    // 3. Ausencia de conflictos (LL(1) puro)
    assert(analyzer.isLL1());
    assert(analyzer.conflictCount() == 0);

    // 4. Pretty-print demo para el docente
    analyzer.printFirstFollow();
    analyzer.printTable();

    // 5. Simulación de sentencia válida: "rotar pulgar falange_1 45"
    std::vector<std::string> valid_tokens = {"rotar", "pulgar", "falange_1", "45"};
    auto result_valid = analyzer.simulate(valid_tokens);
    std::cout << "\n--- Traza de analisis sintactico ---\n";
    for (const auto& step : result_valid.trace) {
        std::cout << "  " << step << "\n";
    }
    assert(result_valid.success);

    // 6. Simulación de sentencia inválida: "rotar 45 pulgar"
    std::vector<std::string> invalid_tokens = {"rotar", "45", "pulgar"};
    auto result_invalid = analyzer.simulate(invalid_tokens);
    assert(!result_invalid.success);
    std::cout << "Sentencia invalida rechazada correctamente: " << result_invalid.error_message << "\n";

    std::cout << "\n>> TODAS LAS PRUEBAS LL(1) PASARON CON EXITO <<\n";
    return 0;
}
