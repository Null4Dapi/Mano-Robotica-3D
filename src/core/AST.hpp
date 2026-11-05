#ifndef CYBERHAND_AST_HPP
#define CYBERHAND_AST_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility>

namespace cyberhand::core {

// Forward declarations
class ProgramNode;
class RoutineDefNode;
class RotateCmdNode;
class WaitCmdNode;
class RepeatCmdNode;
class AssignStmtNode;
class BinaryExprNode;
class NumberExprNode;
class VariableExprNode;

// Interfaz Visitor para recorrido polimórfico limpio del AST (Patrón Visitor)
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(ProgramNode& node) = 0;
    virtual void visit(RoutineDefNode& node) = 0;
    virtual void visit(RotateCmdNode& node) = 0;
    virtual void visit(WaitCmdNode& node) = 0;
    virtual void visit(RepeatCmdNode& node) = 0;
    virtual void visit(AssignStmtNode& node) = 0;
    virtual void visit(BinaryExprNode& node) = 0;
    virtual void visit(NumberExprNode& node) = 0;
    virtual void visit(VariableExprNode& node) = 0;
};

// Clase base abstracta de todos los nodos del AST
class ASTNode {
protected:
    int line_{1};
    int column_{1};

public:
    ASTNode(int line, int col) : line_(line), column_(col) {}
    virtual ~ASTNode() = default;

    [[nodiscard]] int line() const noexcept { return line_; }
    [[nodiscard]] int column() const noexcept { return column_; }

    virtual void accept(ASTVisitor& visitor) = 0;
};

// Clase base para expresiones (evaluables a un valor numérico/temporal)
class ExprNode : public ASTNode {
public:
    using ASTNode::ASTNode;
};

// Nodo: Número literal
class NumberExprNode : public ExprNode {
private:
    double value_{0.0};

public:
    NumberExprNode(double val, int line, int col)
        : ExprNode(line, col), value_(val) {}

    [[nodiscard]] double value() const noexcept { return value_; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Nodo: Referencia a variable
class VariableExprNode : public ExprNode {
private:
    std::string name_;

public:
    VariableExprNode(std::string name, int line, int col)
        : ExprNode(line, col), name_(std::move(name)) {}

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Nodo: Expresión Binaria (ej. a + b, 30 * 2)
class BinaryExprNode : public ExprNode {
private:
    std::string op_;
    std::unique_ptr<ExprNode> left_;
    std::unique_ptr<ExprNode> right_;

public:
    BinaryExprNode(std::string op, std::unique_ptr<ExprNode> left,
                   std::unique_ptr<ExprNode> right, int line, int col)
        : ExprNode(line, col), op_(std::move(op)), left_(std::move(left)), right_(std::move(right)) {}

    [[nodiscard]] const std::string& op() const noexcept { return op_; }
    [[nodiscard]] ExprNode* left() const noexcept { return left_.get(); }
    [[nodiscard]] ExprNode* right() const noexcept { return right_.get(); }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Clase base para instrucciones/sentencias
class StmtNode : public ASTNode {
public:
    using ASTNode::ASTNode;
};

// Nodo: Asignación de variable (ej. angulo = 45;)
class AssignStmtNode : public StmtNode {
private:
    std::string var_name_;
    std::unique_ptr<ExprNode> expr_;

public:
    AssignStmtNode(std::string var_name, std::unique_ptr<ExprNode> expr, int line, int col)
        : StmtNode(line, col), var_name_(std::move(var_name)), expr_(std::move(expr)) {}

    [[nodiscard]] const std::string& var_name() const noexcept { return var_name_; }
    [[nodiscard]] ExprNode* expr() const noexcept { return expr_.get(); }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Nodo: Comando ROTAR (ej. rotar pulgar falange_1 45;)
class RotateCmdNode : public StmtNode {
private:
    std::string finger_;
    std::string phalanx_;
    std::unique_ptr<ExprNode> angle_expr_;

public:
    RotateCmdNode(std::string finger, std::string phalanx,
                  std::unique_ptr<ExprNode> angle_expr, int line, int col)
        : StmtNode(line, col), finger_(std::move(finger)), phalanx_(std::move(phalanx)),
          angle_expr_(std::move(angle_expr)) {}

    [[nodiscard]] const std::string& finger() const noexcept { return finger_; }
    [[nodiscard]] const std::string& phalanx() const noexcept { return phalanx_; }
    [[nodiscard]] ExprNode* angle_expr() const noexcept { return angle_expr_.get(); }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Nodo: Comando ESPERAR (ej. esperar 500 ms;)
class WaitCmdNode : public StmtNode {
private:
    std::unique_ptr<ExprNode> duration_expr_;

public:
    WaitCmdNode(std::unique_ptr<ExprNode> duration, int line, int col)
        : StmtNode(line, col), duration_expr_(std::move(duration)) {}

    [[nodiscard]] ExprNode* duration_expr() const noexcept { return duration_expr_.get(); }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Nodo: Ciclo REPETIR (ej. repetir 3 veces { ... })
class RepeatCmdNode : public StmtNode {
private:
    std::unique_ptr<ExprNode> count_expr_;
    std::vector<std::unique_ptr<StmtNode>> body_;

public:
    RepeatCmdNode(std::unique_ptr<ExprNode> count,
                  std::vector<std::unique_ptr<StmtNode>> body, int line, int col)
        : StmtNode(line, col), count_expr_(std::move(count)), body_(std::move(body)) {}

    [[nodiscard]] ExprNode* count_expr() const noexcept { return count_expr_.get(); }
    [[nodiscard]] const std::vector<std::unique_ptr<StmtNode>>& body() const noexcept { return body_; }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Nodo: Definición de Rutina (ej. rutina CerrarPuno() { ... })
class RoutineDefNode : public ASTNode {
private:
    std::string name_;
    std::vector<std::unique_ptr<StmtNode>> statements_;

public:
    RoutineDefNode(std::string name, std::vector<std::unique_ptr<StmtNode>> stmts, int line, int col)
        : ASTNode(line, col), name_(std::move(name)), statements_(std::move(stmts)) {}

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::vector<std::unique_ptr<StmtNode>>& statements() const noexcept { return statements_; }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Nodo Raíz: Programa Completo
class ProgramNode : public ASTNode {
private:
    std::vector<std::unique_ptr<RoutineDefNode>> routines_;
    std::vector<std::unique_ptr<StmtNode>> main_statements_;

public:
    ProgramNode(std::vector<std::unique_ptr<RoutineDefNode>> routines,
                std::vector<std::unique_ptr<StmtNode>> main_stmts, int line, int col)
        : ASTNode(line, col), routines_(std::move(routines)), main_statements_(std::move(main_stmts)) {}

    [[nodiscard]] const std::vector<std::unique_ptr<RoutineDefNode>>& routines() const noexcept { return routines_; }
    [[nodiscard]] const std::vector<std::unique_ptr<StmtNode>>& main_statements() const noexcept { return main_statements_; }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

} // namespace cyberhand::core

#endif // CYBERHAND_AST_HPP
