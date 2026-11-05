#ifndef CYBERHAND_SEMANTICS_SYMBOL_TABLE_HPP
#define CYBERHAND_SEMANTICS_SYMBOL_TABLE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <iostream>

namespace cyberhand::semantics {

enum class DataType { INT, FLOAT, ANGLE, TIME, ROUTINE, UNKNOWN };
enum class Category { VARIABLE, CONSTANT, ROUTINE, ACTUATOR };

const char* dataTypeToString(DataType t);
const char* categoryToString(Category c);

struct Symbol {
    std::string name;
    DataType type{DataType::UNKNOWN};
    Category category{Category::VARIABLE};
    int scope_level{0};
    int line{0};
    int column{0};
    std::optional<double> value{std::nullopt};
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();
    [[nodiscard]] int currentScopeLevel() const noexcept;

    bool insert(const Symbol& sym);
    [[nodiscard]] const Symbol* lookup(const std::string& name) const;
    [[nodiscard]] const Symbol* lookupCurrentScope(const std::string& name) const;

    [[nodiscard]] const std::vector<Symbol>& allSymbols() const noexcept { return all_symbols_; }
    void printTable(std::ostream& os = std::cout) const;

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
    std::vector<Symbol> all_symbols_;
    void initActuators();
};

} // namespace cyberhand::semantics

#endif // CYBERHAND_SEMANTICS_SYMBOL_TABLE_HPP
