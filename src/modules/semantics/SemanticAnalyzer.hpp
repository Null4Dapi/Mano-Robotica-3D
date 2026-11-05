#ifndef CYBERHAND_SEMANTICS_SEMANTIC_ANALYZER_HPP
#define CYBERHAND_SEMANTICS_SEMANTIC_ANALYZER_HPP

#include "core/AST.hpp"
#include "modules/semantics/SymbolTable.hpp"
#include <vector>
#include <string>
#include <optional>

namespace cyberhand::semantics {

class SemanticAnalyzer : public cyberhand::core::ASTVisitor {
public:
    explicit SemanticAnalyzer(SymbolTable& table);
    SemanticAnalyzer();

    bool analyze(cyberhand::core::ProgramNode& root);
    bool analyze(cyberhand::core::ProgramNode* root);

    [[nodiscard]] const std::vector<std::string>& errors() const noexcept { return errors_; }
    [[nodiscard]] bool hasErrors() const noexcept { return !errors_.empty(); }
    [[nodiscard]] SymbolTable& symbolTable() noexcept { return table_; }
    [[nodiscard]] const SymbolTable& symbolTable() const noexcept { return table_; }

    void visit(cyberhand::core::ProgramNode& node) override;
    void visit(cyberhand::core::RoutineDefNode& node) override;
    void visit(cyberhand::core::RotateCmdNode& node) override;
    void visit(cyberhand::core::WaitCmdNode& node) override;
    void visit(cyberhand::core::RepeatCmdNode& node) override;
    void visit(cyberhand::core::AssignStmtNode& node) override;
    void visit(cyberhand::core::BinaryExprNode& node) override;
    void visit(cyberhand::core::NumberExprNode& node) override;
    void visit(cyberhand::core::VariableExprNode& node) override;

private:
    SymbolTable owned_table_;
    SymbolTable& table_;
    std::vector<std::string> errors_;

    void addError(const std::string& msg, int line, int col);
    std::optional<double> evalConst(cyberhand::core::ExprNode* expr);
    DataType inferType(cyberhand::core::ExprNode* expr);
};

} // namespace cyberhand::semantics

#endif // CYBERHAND_SEMANTICS_SEMANTIC_ANALYZER_HPP
