#include "modules/semantics/SymbolTable.hpp"
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
    os << "+----------------------+------------+------------+-------+------+\n"
       << "| Identifier           | Type       | Category   | Scope | Line |\n"
       << "+----------------------+------------+------------+-------+------+\n";
    for (const auto& s : all_symbols_) {
        os << "| " << std::left << std::setw(20) << s.name
           << " | " << std::setw(10) << dataTypeToString(s.type)
           << " | " << std::setw(10) << categoryToString(s.category)
           << " | " << std::setw(5)  << s.scope_level
           << " | " << std::setw(4)  << s.line << " |\n";
    }
    os << "+----------------------+------------+------------+-------+------+\n";
}

} // namespace cyberhand::semantics
