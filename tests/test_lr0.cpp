#include "modules/lr0_analyzer/LR0Automaton.hpp"
#include <iostream>
#include <cassert>

int main() {
    using namespace cyberhand::syntax::lr0;
    std::cout << "=== Test LR(0) Bottom-Up Syntax Analyzer ===\n";

    // 1. Gramática Cinemática LR(0) y Autómata
    Grammar kin_grammar = Grammar::createKinematicGrammar();
    LR0Automaton analyzer(kin_grammar);

    // 2. Verificar Estados Canónicos
    const auto& states = analyzer.getStates();
    std::cout << "Estados generados: " << states.size() << "\n";
    assert(states.size() == 10);
    assert(!analyzer.hasConflicts());
    assert(analyzer.getConflicts().empty());

    // 3. Tablas ACTION y GOTO pobladas y válidas
    const auto& action_tbl = analyzer.getActionTable();
    const auto& goto_tbl = analyzer.getGotoTable();
    assert(!action_tbl.empty() && !goto_tbl.empty());

    // I0 con "rotar" debe ser SHIFT a I2, I1 con "$" debe ser ACCEPT
    auto it_shift = action_tbl.find({0, "rotar"});
    assert(it_shift != action_tbl.end() && it_shift->second.type == ActionType::SHIFT && it_shift->second.target == 2);
    auto it_acc = action_tbl.find({1, "$"});
    assert(it_acc != action_tbl.end() && it_acc->second.type == ActionType::ACCEPT);

    // 4. Formatted ASCII outputs
    analyzer.printStates();
    analyzer.printTable();

    // 5. Simulación de traza ascendente: "rotar pulgar numero"
    std::vector<std::string> valid_tokens = {"rotar", "pulgar", "numero"};
    auto parse_res = analyzer.simulate(valid_tokens);
    std::cout << "\n--- Traza de Analisis Ascendente LR(0) ---\n";
    analyzer.printTrace(parse_res);
    assert(parse_res.success && !parse_res.trace.empty());

    // 6. Rechazo de secuencia sintácticamente inválida: "rotar numero pulgar"
    std::vector<std::string> invalid_tokens = {"rotar", "numero", "pulgar"};
    auto invalid_res = analyzer.simulate(invalid_tokens);
    assert(!invalid_res.success);
    std::cout << "Secuencia invalida rechazada correctamente: " << invalid_res.error_message << "\n";

    // 7. Detección de Conflictos (Gramática no-LR(0): E -> E + E | id)
    Grammar conflict_grammar{
        .start_symbol = "E", .non_terminals = {"E"}, .terminals = {"+", "id"},
        .productions = {{1, "E", {"E", "+", "E"}}, {2, "E", {"id"}}}
    };
    LR0Automaton conflict_analyzer(conflict_grammar);
    assert(conflict_analyzer.hasConflicts());
    std::cout << "Conflictos detectados en gramatica ambigua: " << conflict_analyzer.getConflicts().size() << "\n";
    for (const auto& c : conflict_analyzer.getConflicts()) {
        std::cout << "  [Estado " << c.state_id << "] Simbolo '" << c.symbol << "': " << c.description << "\n";
    }

    std::cout << "\n>> TODAS LAS PRUEBAS LR(0) PASARON CON EXITO <<\n";
    return 0;
}
