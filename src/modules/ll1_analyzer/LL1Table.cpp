#include "modules/ll1_analyzer/LL1Table.hpp"
#include <algorithm>
#include <cctype>

namespace cyberhand::syntax::ll1 {

Grammar Grammar::createKinematicGrammar() {
    return {
        .start_symbol = "Programa",
        .non_terminals = {"Programa", "MasDeclaraciones", "Declaracion", "Rutina",
                          "Comando", "Rotar", "Esperar", "Dedo", "Falange", "Grados"},
        .terminals = {"rutina", "id", "rotar", "esperar", "numero", "ms",
                      "pulgar", "indice", "falange_1", "falange_2"},
        .productions = {
            {1,  "Programa",         {"Declaracion", "MasDeclaraciones"}},
            {2,  "MasDeclaraciones", {"Declaracion", "MasDeclaraciones"}},
            {3,  "MasDeclaraciones", {std::string(EPSILON)}},
            {4,  "Declaracion",      {"Rutina"}},
            {5,  "Declaracion",      {"Comando"}},
            {6,  "Rutina",           {"rutina", "id"}},
            {7,  "Comando",          {"Rotar"}},
            {8,  "Comando",          {"Esperar"}},
            {9,  "Rotar",            {"rotar", "Dedo", "Falange", "Grados"}},
            {10, "Esperar",          {"esperar", "numero", "ms"}},
            {11, "Dedo",             {"pulgar"}},
            {12, "Dedo",             {"indice"}},
            {13, "Falange",          {"falange_1"}},
            {14, "Falange",          {"falange_2"}},
            {15, "Grados",           {"numero"}}
        }
    };
}

LL1Table::LL1Table(Grammar grammar) : grammar_(std::move(grammar)) {
    computeFirstSets();
    computeFollowSets();
    buildTable();
}

bool LL1Table::isTerminal(const std::string& s) const {
    return s == END_MARKER || s == EPSILON ||
           std::find(grammar_.terminals.begin(), grammar_.terminals.end(), s) != grammar_.terminals.end();
}

bool LL1Table::isNonTerminal(const std::string& s) const {
    return std::find(grammar_.non_terminals.begin(), grammar_.non_terminals.end(), s) != grammar_.non_terminals.end();
}

std::string LL1Table::tokenToTerminal(const std::string& token) const {
    if (isTerminal(token)) return token;
    if (!token.empty() && std::all_of(token.begin(), token.end(), [](char c) {
        return std::isdigit(static_cast<unsigned char>(c));
    })) return "numero";
    return token;
}

std::set<std::string> LL1Table::firstOfSequence(const std::vector<std::string>& seq) const {
    if (seq.empty() || (seq.size() == 1 && seq[0] == EPSILON)) return {std::string(EPSILON)};
    std::set<std::string> result;
    bool all_eps = true;
    for (const auto& sym : seq) {
        const auto& sym_first = (first_sets_.contains(sym)) ? first_sets_.at(sym) : std::set<std::string>{sym};
        bool has_eps = false;
        for (const auto& s : sym_first) {
            if (s == EPSILON) has_eps = true;
            else result.insert(s);
        }
        if (!has_eps) { all_eps = false; break; }
    }
    if (all_eps) result.insert(std::string(EPSILON));
    return result;
}

void LL1Table::computeFirstSets() {
    first_sets_.clear();
    for (const auto& t : grammar_.terminals) first_sets_[t] = {t};
    first_sets_[std::string(EPSILON)] = {std::string(EPSILON)};
    for (const auto& nt : grammar_.non_terminals) first_sets_[nt] = {};

    for (bool changed = true; changed; ) {
        changed = false;
        for (const auto& prod : grammar_.productions) {
            for (const auto& sym : firstOfSequence(prod.rhs)) {
                if (first_sets_[prod.lhs].insert(sym).second) changed = true;
            }
        }
    }
}

void LL1Table::computeFollowSets() {
    follow_sets_.clear();
    for (const auto& nt : grammar_.non_terminals) follow_sets_[nt] = {};
    follow_sets_[grammar_.start_symbol].insert(std::string(END_MARKER));

    for (bool changed = true; changed; ) {
        changed = false;
        for (const auto& prod : grammar_.productions) {
            for (size_t i = 0; i < prod.rhs.size(); ++i) {
                const auto& B = prod.rhs[i];
                if (!isNonTerminal(B)) continue;
                std::vector<std::string> beta(prod.rhs.begin() + i + 1, prod.rhs.end());
                auto beta_first = firstOfSequence(beta);
                for (const auto& sym : beta_first) {
                    if (sym != EPSILON && follow_sets_[B].insert(sym).second) changed = true;
                }
                if (beta_first.contains(std::string(EPSILON))) {
                    for (const auto& sym : follow_sets_[prod.lhs]) {
                        if (follow_sets_[B].insert(sym).second) changed = true;
                    }
                }
            }
        }
    }
}

void LL1Table::buildTable() {
    table_.clear();
    conflict_count_ = 0;
    for (const auto& prod : grammar_.productions) {
        auto rhs_first = firstOfSequence(prod.rhs);
        for (const auto& a : rhs_first) {
            if (a != EPSILON) {
                auto& cell = table_[{prod.lhs, a}];
                cell.push_back(prod);
                if (cell.size() > 1) conflict_count_++;
            }
        }
        if (rhs_first.contains(std::string(EPSILON))) {
            for (const auto& b : follow_sets_[prod.lhs]) {
                auto& cell = table_[{prod.lhs, b}];
                cell.push_back(prod);
                if (cell.size() > 1) conflict_count_++;
            }
        }
    }
}

ParseResult LL1Table::simulate(const std::vector<std::string>& input_tokens) const {
    ParseResult result;
    std::vector<std::string> stack{std::string(END_MARKER), grammar_.start_symbol};
    std::vector<std::string> tokens = input_tokens;
    tokens.push_back(std::string(END_MARKER));
    size_t ip = 0;

    while (!stack.empty()) {
        std::string top = stack.back();
        std::string cur_tok = (ip < tokens.size()) ? tokens[ip] : std::string(END_MARKER);
        std::string cur_term = tokenToTerminal(cur_tok);

        if (top == END_MARKER) {
            if (cur_term == END_MARKER) {
                stack.pop_back();
                result.trace.push_back("Aceptar: pila vacia y fin de entrada emparejado ($).");
                result.success = true;
                return result;
            }
            result.error_message = "Tokens sobrantes al final de la entrada: " + cur_tok;
            return result;
        }

        if (isTerminal(top)) {
            if (top == cur_term || top == cur_tok) {
                stack.pop_back();
                result.trace.push_back("Emparejar terminal: '" + top + "' [token: '" + cur_tok + "']");
                ip++;
            } else {
                result.error_message = "Error sintactico: se esperaba '" + top + "', pero llego '" + cur_tok + "'";
                return result;
            }
        } else {
            auto it = table_.find({top, cur_term});
            if (it == table_.end() || it->second.empty()) {
                result.error_message = "Error de sintaxis: celda vacia en M[" + top + ", " + cur_term + "]";
                return result;
            }
            if (it->second.size() > 1) {
                result.error_message = "Conflicto LL(1) no determinista en M[" + top + ", " + cur_term + "]";
                return result;
            }
            const auto& prod = it->second.front();
            stack.pop_back();

            std::string rhs_repr;
            for (size_t k = 0; k < prod.rhs.size(); ++k) {
                if (k > 0) rhs_repr += " ";
                rhs_repr += prod.rhs[k];
            }
            result.trace.push_back("Derivar [" + std::to_string(prod.id) + "]: " + prod.lhs + " -> " + rhs_repr);

            if (!prod.rhs.empty() && !(prod.rhs.size() == 1 && prod.rhs[0] == EPSILON)) {
                for (auto rit = prod.rhs.rbegin(); rit != prod.rhs.rend(); ++rit) {
                    stack.push_back(*rit);
                }
            }
        }
    }
    result.success = (ip >= tokens.size() - 1);
    return result;
}

void LL1Table::printFirstFollow(std::ostream& os) const {
    os << "\n+-------------------+------------------------------------+--------------------------------+\n";
    os << "| No Terminal       | PRIMERO                            | SIGUIENTE                      |\n";
    os << "+-------------------+------------------------------------+--------------------------------+\n";
    for (const auto& nt : grammar_.non_terminals) {
        std::string first_s = "{ ", follow_s = "{ ";
        if (first_sets_.contains(nt)) {
            size_t i = 0;
            for (const auto& s : first_sets_.at(nt)) { if (i++ > 0) first_s += ", "; first_s += s; }
        }
        first_s += " }";
        if (follow_sets_.contains(nt)) {
            size_t i = 0;
            for (const auto& s : follow_sets_.at(nt)) { if (i++ > 0) follow_s += ", "; follow_s += s; }
        }
        follow_s += " }";

        os << "| " << nt << (nt.size() < 18 ? std::string(18 - nt.size(), ' ') : "")
           << "| " << first_s << (first_s.size() < 35 ? std::string(35 - first_s.size(), ' ') : "")
           << "| " << follow_s << (follow_s.size() < 31 ? std::string(31 - follow_s.size(), ' ') : "")
           << "|\n";
    }
    os << "+-------------------+------------------------------------+--------------------------------+\n";
}

void LL1Table::printTable(std::ostream& os) const {
    os << "\n+-----------------------------------------------------------------------------------------+\n";
    os << "| TABLA DE ANALISIS SINTACTICO PREDICTIVO LL(1)                                          |\n";
    os << "+-------------------+-------------------+-------------------------------------------------+\n";
    os << "| No Terminal       | Terminal          | Produccion M[A, a]                              |\n";
    os << "+-------------------+-------------------+-------------------------------------------------+\n";
    for (const auto& [key, prods] : table_) {
        const auto& [nt, term] = key;
        for (const auto& p : prods) {
            std::string rhs;
            for (size_t k = 0; k < p.rhs.size(); ++k) { if (k > 0) rhs += " "; rhs += p.rhs[k]; }
            std::string rule_str = "[" + std::to_string(p.id) + "] " + p.lhs + " -> " + rhs;
            os << "| " << nt << (nt.size() < 18 ? std::string(18 - nt.size(), ' ') : "")
               << "| " << term << (term.size() < 18 ? std::string(18 - term.size(), ' ') : "")
               << "| " << rule_str << (rule_str.size() < 48 ? std::string(48 - rule_str.size(), ' ') : "")
               << "|\n";
        }
    }
    os << "+-------------------+-------------------+-------------------------------------------------+\n";
    os << "| Estado: " << (isLL1() ? "GRAMATICA LL(1) VALIDA (0 Conflictos)" : "CONFLICTOS DETECTADOS") << "\n";
    os << "+-----------------------------------------------------------------------------------------+\n";
}

} // namespace cyberhand::syntax::ll1
