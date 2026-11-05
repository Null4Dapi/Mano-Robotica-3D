#ifndef CYBERHAND_INTERMEDIATE_CODE_GENERATOR_HPP
#define CYBERHAND_INTERMEDIATE_CODE_GENERATOR_HPP

#include "core/AST.hpp"
#include "modules/intermediate/Quadruple.hpp"
#include <vector>
#include <string>
#include <iostream>

namespace cyberhand::intermediate {

class CodeGenerator : public cyberhand::core::ASTVisitor {
public:
    CodeGenerator() = default;

    void generate(cyberhand::core::ProgramNode& root);
    void reset();

    [[nodiscard]] const std::vector<Quadruple>& quadruples() const noexcept { return quadruples_; }
    [[nodiscard]] std::string newTemp();
    [[nodiscard]] std::string newLabel();

    [[nodiscard]] std::string dumpQuadruples() const;
    [[nodiscard]] std::string dumpTriples() const;
    void printQuadruples(std::ostream& os = std::cout) const;
    void printTriples(std::ostream& os = std::cout) const;

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
    std::vector<Quadruple> quadruples_;
    int temp_counter_{0};
    int label_counter_{0};
    std::string last_result_;

    void emit(std::string op, std::string arg1, std::string arg2, std::string result);
    std::string emitExpr(cyberhand::core::ExprNode* expr);
};

} // namespace cyberhand::intermediate

#endif // CYBERHAND_INTERMEDIATE_CODE_GENERATOR_HPP
