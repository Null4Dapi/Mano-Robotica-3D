#include "modules/lr0_analyzer/LR0Automaton.hpp"
#include "core/TUI.hpp"
#include <algorithm>
#include <iomanip>

namespace cyberhand::syntax::lr0 {

Grammar Grammar::createKinematicGrammar() {
    return Grammar{
        .start_symbol = "Instruccion",
        .non_terminals = {"Instruccion", "Dedo", "Angulo"},
        .terminals = {"rotar", "esperar", "pulgar", "indice", "numero"},
        .productions = {
            {1, "Instruccion", {"rotar", "Dedo", "Angulo"}}, {2, "Instruccion", {"esperar", "Angulo"}},
            {3, "Dedo", {"pulgar"}}, {4, "Dedo", {"indice"}}, {5, "Angulo", {"numero"}}
        }
    };
}

LR0Automaton::LR0Automaton(Grammar grammar) : grammar_(std::move(grammar)) {
    augmentGrammar();
    buildCanonicalCollection();
    buildTables();
}

void LR0Automaton::augmentGrammar() {
    if (grammar_.productions.empty()) return;
    std::string prime = grammar_.start_symbol + "'";
    if (grammar_.productions[0].lhs == prime) return;
    grammar_.productions.insert(grammar_.productions.begin(), {0, prime, {grammar_.start_symbol}});
    for (size_t i = 0; i < grammar_.productions.size(); ++i) grammar_.productions[i].id = static_cast<int>(i);
    grammar_.non_terminals.insert(grammar_.non_terminals.begin(), prime);
    grammar_.start_symbol = prime;
}

bool LR0Automaton::isNonTerminal(const std::string& s) const {
    return std::find(grammar_.non_terminals.begin(), grammar_.non_terminals.end(), s) != grammar_.non_terminals.end();
}

bool LR0Automaton::isTerminal(const std::string& s) const {
    return s != END_MARKER && std::find(grammar_.terminals.begin(), grammar_.terminals.end(), s) != grammar_.terminals.end();
}

std::set<Item> LR0Automaton::closure(const std::set<Item>& items) const {
    std::set<Item> res = items;
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<Item> current(res.begin(), res.end());
        for (const auto& item : current) {
            const auto& prod = grammar_.productions[item.prod_id];
            if (item.dot < prod.rhs.size() && isNonTerminal(prod.rhs[item.dot])) {
                for (const auto& p : grammar_.productions) {
                    if (p.lhs == prod.rhs[item.dot] && res.insert(Item{p.id, 0}).second) changed = true;
                }
            }
        }
    }
    return res;
}

std::set<Item> LR0Automaton::goTo(const std::set<Item>& items, const std::string& symbol) const {
    std::set<Item> moved;
    for (const auto& item : items) {
        const auto& prod = grammar_.productions[item.prod_id];
        if (item.dot < prod.rhs.size() && prod.rhs[item.dot] == symbol) moved.insert(Item{item.prod_id, item.dot + 1});
    }
    return moved.empty() ? std::set<Item>{} : closure(moved);
}

void LR0Automaton::buildCanonicalCollection() {
    states_.clear(); goto_table_.clear(); transitions_.clear();
    std::set<Item> i0 = closure({{0, 0}});
    states_.push_back(State{0, i0});
    std::map<std::set<Item>, int> state_map{{i0, 0}};

    std::vector<std::string> symbols;
    for (const auto& nt : grammar_.non_terminals) {
        if (nt != grammar_.start_symbol) symbols.push_back(nt);
    }
    for (const auto& t : grammar_.terminals) symbols.push_back(t);

    size_t processed = 0;
    while (processed < states_.size()) {
        int cur_id = states_[processed].id;
        const auto cur_items = states_[processed++].items;
        for (const auto& sym : symbols) {
            auto next_items = goTo(cur_items, sym);
            if (next_items.empty()) continue;
            auto [it, inserted] = state_map.try_emplace(next_items, static_cast<int>(states_.size()));
            if (inserted) states_.push_back(State{it->second, next_items});
            transitions_[{cur_id, sym}] = it->second;
            if (isNonTerminal(sym)) goto_table_[{cur_id, sym}] = it->second;
        }
    }
}

void LR0Automaton::buildTables() {
    action_table_.clear(); conflicts_.clear();
    std::vector<std::string> all_terms = grammar_.terminals;
    all_terms.push_back(std::string(END_MARKER));

    for (const auto& st : states_) {
        std::map<std::string, std::vector<Action>> candidates;
        for (const auto& t : grammar_.terminals) {
            auto it = transitions_.find({st.id, t});
            if (it != transitions_.end()) candidates[t].push_back(Action{ActionType::SHIFT, it->second});
        }
        for (const auto& item : st.items) {
            const auto& prod = grammar_.productions[item.prod_id];
            if (item.dot == prod.rhs.size()) {
                if (prod.id == 0) {
                    candidates[std::string(END_MARKER)].push_back(Action{ActionType::ACCEPT, 0});
                } else {
                    for (const auto& t : all_terms) {
                        Action act{ActionType::REDUCE, prod.id};
                        auto& vec = candidates[t];
                        if (std::find(vec.begin(), vec.end(), act) == vec.end()) vec.push_back(act);
                    }
                }
            }
        }
        for (const auto& [sym, acts] : candidates) {
            if (acts.empty()) continue;
            if (acts.size() > 1) {
                bool has_s = false, has_r = false;
                for (const auto& a : acts) {
                    if (a.type == ActionType::SHIFT) has_s = true;
                    if (a.type == ActionType::REDUCE) has_r = true;
                }
                std::string desc = (has_s && has_r) ? "Shift-Reduce conflict on symbol '" + sym + "'"
                                                    : "Reduce-Reduce conflict on symbol '" + sym + "'";
                conflicts_.push_back(Conflict{st.id, sym, desc});
            }
            action_table_[{st.id, sym}] = acts[0];
        }
    }
}

ParseResult LR0Automaton::simulate(const std::vector<std::string>& tokens) const {
    ParseResult result;
    if (hasConflicts()) { result.error_message = "Cannot simulate: LR(0) conflicts detected"; return result; }
    std::vector<int> state_stack{0};
    std::vector<std::string> symbol_stack, input = tokens;
    input.push_back(std::string(END_MARKER));
    size_t ip = 0, step_count = 0;

    auto join = [](const auto& v, size_t start = 0) {
        std::string s;
        for (size_t i = start; i < v.size(); ++i) {
            if constexpr (std::is_same_v<std::decay_t<decltype(v[i])>, int>) s += (i == start ? "" : " ") + std::to_string(v[i]);
            else s += (i == start ? "" : " ") + v[i];
        }
        return s.empty() ? "-" : s;
    };

    while (step_count < 1000) {
        step_count++;
        int s = state_stack.back();
        std::string a = (ip < input.size()) ? input[ip] : std::string(END_MARKER);
        auto it = action_table_.find({s, a});
        if (it == action_table_.end()) {
            result.error_message = "Syntax error at state " + std::to_string(s) + " with token '" + a + "'";
            result.trace.push_back({step_count, join(state_stack), join(symbol_stack), join(input, ip), "ERROR"});
            return result;
        }
        const Action& act = it->second;
        if (act.type == ActionType::SHIFT) {
            result.trace.push_back({step_count, join(state_stack), join(symbol_stack), join(input, ip), "SHIFT " + std::to_string(act.target)});
            symbol_stack.push_back(a); state_stack.push_back(act.target); ip++;
        } else if (act.type == ActionType::REDUCE) {
            const auto& prod = grammar_.productions[act.target];
            std::string desc = "REDUCE " + std::to_string(prod.id) + " (" + prod.lhs + " ->";
            for (const auto& r : prod.rhs) desc += " " + r;
            desc += ")";
            result.trace.push_back({step_count, join(state_stack), join(symbol_stack), join(input, ip), desc});
            for (size_t i = 0; i < prod.rhs.size(); ++i) { state_stack.pop_back(); symbol_stack.pop_back(); }
            int top = state_stack.back();
            auto git = goto_table_.find({top, prod.lhs});
            if (git == goto_table_.end()) { result.error_message = "GOTO missing for " + prod.lhs; return result; }
            symbol_stack.push_back(prod.lhs); state_stack.push_back(git->second);
        } else if (act.type == ActionType::ACCEPT) {
            result.trace.push_back({step_count, join(state_stack), join(symbol_stack), join(input, ip), "ACCEPT"});
            result.success = true;
            return result;
        }
    }
    result.error_message = "Simulation step limit exceeded";
    return result;
}

std::string LR0Automaton::formatItem(const Item& item) const {
    const auto& p = grammar_.productions[item.prod_id];
    std::string s = p.lhs + " ->";
    for (size_t i = 0; i < p.rhs.size(); ++i) s += (i == item.dot ? " ." : "") + (" " + p.rhs[i]);
    if (item.dot == p.rhs.size()) s += " .";
    return s;
}

void LR0Automaton::printStates(std::ostream& os) const {
    using namespace cyberhand::ui;
    os << "\n" << BOLD << WHITE << "=== Coleccion Canonica de Estados LR(0) (" << states_.size() << " Estados) ===" << RESET << "\n";
    for (const auto& st : states_) {
        os << CYAN << "  I" << st.id << ":" << RESET << "\n";
        for (const auto& item : st.items) {
            os << GRAY << "    • " << RESET << formatItem(item) << "\n";
        }
    }
}

void LR0Automaton::printTable(std::ostream& os) const {
    using namespace cyberhand::ui;
    os << "\n" << BOLD << WHITE << "=== Tablas ACTION y GOTO LR(0) ===" << RESET << "\n";
    std::vector<std::string> terms = grammar_.terminals, non_terms;
    terms.push_back(std::string(END_MARKER));
    for (const auto& nt : grammar_.non_terminals) if (nt != grammar_.start_symbol) non_terms.push_back(nt);

    os << GRAY << TOP_L << hRule(8) << T_DOWN << hRule(49) << T_DOWN << hRule(38) << TOP_R << RESET << "\n";
    os << GRAY << V_LINE << BOLD << WHITE << " State  " << RESET
       << GRAY << V_LINE << BOLD << CYAN << "  ACTION (Shift/Reduce)                         " << RESET
       << GRAY << V_LINE << BOLD << GREEN << "  GOTO (Transiciones)                 " << RESET
       << GRAY << V_LINE << RESET << "\n";
    os << GRAY << T_RIGHT << hRule(8) << CROSS << hRule(49) << CROSS << hRule(38) << T_LEFT << RESET << "\n";

    os << GRAY << V_LINE << "        " << V_LINE;
    for (const auto& t : terms) os << std::setw(8) << t;
    os << " " << V_LINE;
    for (const auto& nt : non_terms) os << std::setw(12) << nt;
    os << "  " << V_LINE << RESET << "\n";
    os << GRAY << T_RIGHT << hRule(8) << CROSS << hRule(49) << CROSS << hRule(38) << T_LEFT << RESET << "\n";

    for (const auto& st : states_) {
        os << GRAY << V_LINE << WHITE << std::setw(6) << st.id << "  " << GRAY << V_LINE;
        for (const auto& t : terms) {
            auto it = action_table_.find({st.id, t});
            std::string c = ".";
            if (it != action_table_.end()) {
                c = (it->second.type == ActionType::SHIFT) ? "s" + std::to_string(it->second.target)
                  : (it->second.type == ActionType::REDUCE) ? "r" + std::to_string(it->second.target) : "acc";
            }
            if (c.starts_with("s")) os << CYAN << std::setw(8) << c << RESET;
            else if (c.starts_with("r")) os << YELLOW << std::setw(8) << c << RESET;
            else if (c == "acc") os << GREEN << std::setw(8) << c << RESET;
            else os << GRAY << std::setw(8) << c << RESET;
        }
        os << " " << GRAY << V_LINE;
        for (const auto& nt : non_terms) {
            auto git = goto_table_.find({st.id, nt});
            std::string val = (git != goto_table_.end() ? std::to_string(git->second) : ".");
            if (val != ".") os << WHITE << std::setw(12) << val << RESET;
            else os << GRAY << std::setw(12) << val << RESET;
        }
        os << "  " << GRAY << V_LINE << RESET << "\n";
    }
    os << GRAY << BOT_L << hRule(8) << T_UP << hRule(49) << T_UP << hRule(38) << BOT_R << RESET << "\n";
}

void LR0Automaton::printTrace(const ParseResult& res, std::ostream& os) const {
    using namespace cyberhand::ui;
    os << "\n" << GRAY << TOP_L << hRule(6) << T_DOWN << hRule(22) << T_DOWN << hRule(25) << T_DOWN << hRule(32) << T_DOWN << hRule(18) << TOP_R << RESET << "\n";
    os << GRAY << V_LINE << BOLD << WHITE << " Paso "
       << GRAY << V_LINE << " Pila Estados        "
       << GRAY << V_LINE << " Pila Simbolos           "
       << GRAY << V_LINE << " Entrada                        "
       << GRAY << V_LINE << " Accion           " << GRAY << V_LINE << RESET << "\n";
    os << GRAY << T_RIGHT << hRule(6) << CROSS << hRule(22) << CROSS << hRule(25) << CROSS << hRule(32) << CROSS << hRule(18) << T_LEFT << RESET << "\n";

    for (const auto& s : res.trace) {
        std::string act_color = (s.action_str == "ACCEPT") ? std::string(GREEN)
                              : (s.action_str.starts_with("SHIFT")) ? std::string(CYAN)
                              : (s.action_str.starts_with("REDUCE")) ? std::string(YELLOW) : std::string(RED);
        os << GRAY << V_LINE << WHITE << std::right << std::setw(5) << s.step << " "
           << GRAY << V_LINE << CYAN << " " << std::setw(20) << std::left << s.stack_str << " "
           << GRAY << V_LINE << WHITE << " " << std::setw(23) << std::left << s.symbols_str << " "
           << GRAY << V_LINE << DIM << " " << std::setw(30) << std::left << s.input_str << " "
           << GRAY << V_LINE << act_color << " " << std::setw(16) << std::left << s.action_str << " "
           << GRAY << V_LINE << RESET << "\n";
    }
    os << GRAY << BOT_L << hRule(6) << T_UP << hRule(22) << T_UP << hRule(25) << T_UP << hRule(32) << T_UP << hRule(18) << BOT_R << RESET << "\n";
}

} // namespace cyberhand::syntax::lr0
