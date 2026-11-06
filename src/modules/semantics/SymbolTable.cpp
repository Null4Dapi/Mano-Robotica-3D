#include "modules/semantics/SymbolTable.hpp"
#include "core/TUI.hpp"
#include <iomanip>

namespace cyberhand::semantics {

const char* dataTypeToString(DataType t) {
    switch (t) {
        case DataType::INT:     return "INT";
        case DataType::FLOAT:   return "FLOAT";
        case DataType::ANGLE:   return "ANGLE";
        case DataType::TIME:    return "TIME";
        case DataType::ROUTINE: return "ROUTINE";
        default:                return "UNKNOWN";
    }
}

const char* categoryToString(Category c) {
    switch (c) {
        case Category::VARIABLE: return "VARIABLE";
        case Category::CONSTANT: return "CONSTANT";
        case Category::ROUTINE:  return "ROUTINE";
        case Category::ACTUATOR: return "ACTUATOR";
        default:                 return "UNKNOWN";
    }
}

SymbolTable::SymbolTable() {
    enterScope();
    initActuators();
}

void SymbolTable::initActuators() {
    for (const auto* f : {"pulgar", "indice", "medio", "anular", "menique", "muneca"}) {
        insert(Symbol{f, DataType::UNKNOWN, Category::ACTUATOR, 0, 0, 0, std::nullopt});
    }
    for (const auto* p : {"falange_1", "falange_2", "falange_3", "eje_x", "eje_y", "eje_z"}) {
        insert(Symbol{p, DataType::UNKNOWN, Category::ACTUATOR, 0, 0, 0, std::nullopt});
    }
}

void SymbolTable::enterScope() {
    scopes_.emplace_back();
}

void SymbolTable::exitScope() {
    if (scopes_.size() > 1) scopes_.pop_back();
}

int SymbolTable::currentScopeLevel() const noexcept {
    return static_cast<int>(scopes_.size()) - 1;
}

bool SymbolTable::insert(const Symbol& sym) {
    if (scopes_.empty()) scopes_.emplace_back();
    auto& curr = scopes_.back();
    if (curr.contains(sym.name)) return false;
    curr[sym.name] = sym;
    all_symbols_.push_back(sym);
    return true;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return &found->second;
    }
    return nullptr;
}

const Symbol* SymbolTable::lookupCurrentScope(const std::string& name) const {
    if (scopes_.empty()) return nullptr;
    auto found = scopes_.back().find(name);
    return (found != scopes_.back().end()) ? &found->second : nullptr;
}

void SymbolTable::printTable(std::ostream& os) const {
    using namespace cyberhand::ui;
    os << "\n" << GRAY << TOP_L << hRule(22) << T_DOWN << hRule(12) << T_DOWN << hRule(12) << T_DOWN << hRule(7) << T_DOWN << hRule(6) << TOP_R << RESET << "\n";
    os << GRAY << V_LINE << BOLD << WHITE << " Identifier           "
       << GRAY << V_LINE << " Type       "
       << GRAY << V_LINE << " Category   "
       << GRAY << V_LINE << " Scope "
       << GRAY << V_LINE << " Line " << GRAY << V_LINE << RESET << "\n";
    os << GRAY << T_RIGHT << hRule(22) << CROSS << hRule(12) << CROSS << hRule(12) << CROSS << hRule(7) << CROSS << hRule(6) << T_LEFT << RESET << "\n";

    for (const auto& s : all_symbols_) {
        std::string cat_str(categoryToString(s.category));
        std::string cat_color = (cat_str == "ACTUATOR") ? std::string(CYAN)
                              : (cat_str == "VARIABLE") ? std::string(WHITE) : std::string(YELLOW);
        std::string scope_str = (s.scope_level == 0) ? "global" : "local";

        os << GRAY << V_LINE << WHITE << " " << std::left << std::setw(20) << s.name << " "
           << GRAY << V_LINE << GREEN << " " << std::setw(10) << dataTypeToString(s.type) << " "
           << GRAY << V_LINE << cat_color << " " << std::setw(10) << cat_str << " "
           << GRAY << V_LINE << DIM << " " << std::setw(5)  << s.scope_level << " "
           << GRAY << V_LINE << GRAY << " " << std::setw(4)  << s.line << " "
           << GRAY << V_LINE << RESET << "\n";
    }
    os << GRAY << BOT_L << hRule(22) << T_UP << hRule(12) << T_UP << hRule(12) << T_UP << hRule(7) << T_UP << hRule(6) << BOT_R << RESET << "\n";
}

} // namespace cyberhand::semantics
