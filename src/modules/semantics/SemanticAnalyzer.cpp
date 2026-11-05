#include "modules/semantics/SemanticAnalyzer.hpp"
#include <sstream>
#include <cmath>
#include <unordered_set>

namespace cyberhand::semantics {

static std::string toLower(std::string_view s) {
    std::string res;
    res.reserve(s.size());
    for (char c : s) res.push_back(static_cast<char>(std::tolower(c)));
    return res;
}

SemanticAnalyzer::SemanticAnalyzer(SymbolTable& table) : owned_table_(), table_(table) {}
SemanticAnalyzer::SemanticAnalyzer() : owned_table_(), table_(owned_table_) {}

bool SemanticAnalyzer::analyze(cyberhand::core::ProgramNode& root) {
    errors_.clear();
    root.accept(*this);
    return errors_.empty();
}

bool SemanticAnalyzer::analyze(cyberhand::core::ProgramNode* root) {
    return root ? analyze(*root) : false;
}

void SemanticAnalyzer::addError(const std::string& msg, int line, int col) {
    errors_.push_back("L" + std::to_string(line) + ":C" + std::to_string(col) + " - " + msg);
}

std::optional<double> SemanticAnalyzer::evalConst(cyberhand::core::ExprNode* expr) {
    if (!expr) return std::nullopt;
    if (auto* n = dynamic_cast<cyberhand::core::NumberExprNode*>(expr)) return n->value();
    if (auto* v = dynamic_cast<cyberhand::core::VariableExprNode*>(expr)) {
        auto* s = table_.lookup(v->name());
        return (s && s->value.has_value()) ? s->value : std::nullopt;
    }
    if (auto* b = dynamic_cast<cyberhand::core::BinaryExprNode*>(expr)) {
        auto l = evalConst(b->left()), r = evalConst(b->right());
        if (l && r) {
            if (b->op() == "+") return *l + *r;
            if (b->op() == "-") return *l - *r;
            if (b->op() == "*") return *l * *r;
            if (b->op() == "/" && *r != 0.0) return *l / *r;
        }
    }
    return std::nullopt;
}

DataType SemanticAnalyzer::inferType(cyberhand::core::ExprNode* expr) {
    if (!expr) return DataType::UNKNOWN;
    if (auto* v = dynamic_cast<cyberhand::core::VariableExprNode*>(expr)) {
        auto* s = table_.lookup(v->name());
        return s ? s->type : DataType::UNKNOWN;
    }
    if (auto* b = dynamic_cast<cyberhand::core::BinaryExprNode*>(expr)) {
        auto lt = inferType(b->left()), rt = inferType(b->right());
        if (lt == DataType::TIME || rt == DataType::TIME) return DataType::TIME;
        if (lt == DataType::ANGLE || rt == DataType::ANGLE) return DataType::ANGLE;
        if (lt == DataType::FLOAT || rt == DataType::FLOAT) return DataType::FLOAT;
        return DataType::INT;
    }
    if (auto* n = dynamic_cast<cyberhand::core::NumberExprNode*>(expr)) {
        return (n->value() == std::floor(n->value())) ? DataType::INT : DataType::FLOAT;
    }
    return DataType::UNKNOWN;
}

void SemanticAnalyzer::visit(cyberhand::core::ProgramNode& node) {
    for (const auto& r : node.routines()) {
        Symbol sym{r->name(), DataType::ROUTINE, Category::ROUTINE, 0, r->line(), r->column(), std::nullopt};
        if (!table_.insert(sym)) {
            addError("Rutina duplicada '" + r->name() + "' ya declarada en ambito global", r->line(), r->column());
        }
        r->accept(*this);
    }
    for (const auto& s : node.main_statements()) s->accept(*this);
}

void SemanticAnalyzer::visit(cyberhand::core::RoutineDefNode& node) {
    table_.enterScope();
    for (const auto& s : node.statements()) s->accept(*this);
    table_.exitScope();
}

void SemanticAnalyzer::visit(cyberhand::core::RotateCmdNode& node) {
    static const std::unordered_set<std::string> fingers = {"pulgar", "indice", "medio", "anular", "menique", "muneca"};
    static const std::unordered_set<std::string> phalanges = {"falange_1", "falange_2", "falange_3", "eje_x", "eje_y", "eje_z"};

    if (!fingers.contains(toLower(node.finger()))) {
        addError("Actuador/dedo desconocido '" + node.finger() + "'", node.line(), node.column());
    }
    if (!phalanges.contains(toLower(node.phalanx()))) {
        addError("Articulacion/falange desconocida '" + node.phalanx() + "'", node.line(), node.column());
    }

    if (node.angle_expr()) {
        node.angle_expr()->accept(*this);
        if (inferType(node.angle_expr()) == DataType::TIME) {
            addError("Error de tipo: se esperaba tipo ANGLE pero se recibio TIME", node.line(), node.column());
        }
        auto val = evalConst(node.angle_expr());
        if (val.has_value() && (*val < 0.0 || *val > 90.0)) {
            std::ostringstream oss;
            oss << "Violacion biomecanica: angulo " << *val << " fuera del rango seguro [0, 90]";
            addError(oss.str(), node.line(), node.column());
        }
    }
}

void SemanticAnalyzer::visit(cyberhand::core::WaitCmdNode& node) {
    if (node.duration_expr()) {
        node.duration_expr()->accept(*this);
        if (inferType(node.duration_expr()) == DataType::ANGLE) {
            addError("Error de tipo: se esperaba tipo TIME pero se recibio ANGLE", node.line(), node.column());
        }
        auto val = evalConst(node.duration_expr());
        if (val.has_value() && *val < 0.0) {
            addError("Tiempo de espera no puede ser negativo", node.line(), node.column());
        }
    }
}

void SemanticAnalyzer::visit(cyberhand::core::RepeatCmdNode& node) {
    if (node.count_expr()) {
        node.count_expr()->accept(*this);
        auto val = evalConst(node.count_expr());
        if (val.has_value() && *val <= 0.0) {
            addError("Numero de repeticiones debe ser positivo", node.line(), node.column());
        }
    }
    for (const auto& s : node.body()) s->accept(*this);
}

void SemanticAnalyzer::visit(cyberhand::core::AssignStmtNode& node) {
    if (node.expr()) node.expr()->accept(*this);
    if (table_.lookupCurrentScope(node.var_name())) {
        addError("Declaracion duplicada de variable '" + node.var_name() + "' en el mismo ambito", node.line(), node.column());
        return;
    }
    auto* existing = table_.lookup(node.var_name());
    if (existing && (existing->category == Category::ACTUATOR || existing->category == Category::ROUTINE)) {
        addError("No se puede redefinir el identificador reservado '" + node.var_name() + "' como variable", node.line(), node.column());
        return;
    }
    DataType t = inferType(node.expr());
    std::string lower = toLower(node.var_name());
    if (lower.find("angulo") != std::string::npos || lower.find("angle") != std::string::npos) t = DataType::ANGLE;
    else if (lower.find("tiempo") != std::string::npos || lower.find("time") != std::string::npos || lower.find("ms") != std::string::npos) t = DataType::TIME;

    Symbol s{node.var_name(), t, Category::VARIABLE, table_.currentScopeLevel(), node.line(), node.column(), evalConst(node.expr())};
    table_.insert(s);
}

void SemanticAnalyzer::visit(cyberhand::core::BinaryExprNode& node) {
    if (node.left()) node.left()->accept(*this);
    if (node.right()) node.right()->accept(*this);
}

void SemanticAnalyzer::visit([[maybe_unused]] cyberhand::core::NumberExprNode& node) {}

void SemanticAnalyzer::visit(cyberhand::core::VariableExprNode& node) {
    if (!table_.lookup(node.name())) {
        addError("Variable no declarada '" + node.name() + "'", node.line(), node.column());
    }
}

} // namespace cyberhand::semantics
