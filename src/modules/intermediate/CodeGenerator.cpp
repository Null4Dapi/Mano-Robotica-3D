#include "modules/intermediate/CodeGenerator.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <unordered_map>

namespace cyberhand::intermediate {

void CodeGenerator::reset() {
    quadruples_.clear();
    temp_counter_ = 0;
    label_counter_ = 0;
    last_result_.clear();
}

void CodeGenerator::generate(cyberhand::core::ProgramNode& root) {
    reset();
    root.accept(*this);
}

std::string CodeGenerator::newTemp() { return "t" + std::to_string(++temp_counter_); }
std::string CodeGenerator::newLabel() { return "L" + std::to_string(++label_counter_); }

void CodeGenerator::emit(std::string op, std::string a1, std::string a2, std::string res) {
    int idx = static_cast<int>(quadruples_.size());
    quadruples_.emplace_back(idx, std::move(op), std::move(a1), std::move(a2), std::move(res));
}

std::string CodeGenerator::emitExpr(cyberhand::core::ExprNode* expr) {
    if (!expr) return "_";
    expr->accept(*this);
    return last_result_;
}

void CodeGenerator::visit(cyberhand::core::ProgramNode& n) {
    for (const auto& r : n.routines()) if (r) r->accept(*this);
    for (const auto& s : n.main_statements()) if (s) s->accept(*this);
}

void CodeGenerator::visit(cyberhand::core::RoutineDefNode& n) {
    emit("LABEL", n.name(), "_", "_");
    for (const auto& s : n.statements()) if (s) s->accept(*this);
    emit("RETURN", "_", "_", "_");
}

void CodeGenerator::visit(cyberhand::core::RotateCmdNode& n) {
    emit("ROTAR", n.finger() + ":" + n.phalanx(), emitExpr(n.angle_expr()), "_");
}

void CodeGenerator::visit(cyberhand::core::WaitCmdNode& n) {
    emit("ESPERAR", emitExpr(n.duration_expr()), "ms", "_");
}

void CodeGenerator::visit(cyberhand::core::RepeatCmdNode& n) {
    std::string count = emitExpr(n.count_expr()), counter = newTemp();
    emit("=", std::move(count), "_", counter);
    std::string start_lbl = newLabel(), end_lbl = newLabel();
    emit("LABEL", start_lbl, "_", "_");
    emit("JUMP_IF_ZERO", counter, "_", end_lbl);
    for (const auto& s : n.body()) if (s) s->accept(*this);
    std::string dec = newTemp();
    emit("-", counter, "1", dec);
    emit("=", std::move(dec), "_", counter);
    emit("JUMP", std::move(start_lbl), "_", "_");
    emit("LABEL", std::move(end_lbl), "_", "_");
}

void CodeGenerator::visit(cyberhand::core::AssignStmtNode& n) {
    emit("=", emitExpr(n.expr()), "_", n.var_name());
}

void CodeGenerator::visit(cyberhand::core::BinaryExprNode& n) {
    std::string l = emitExpr(n.left()), r = emitExpr(n.right()), t = newTemp();
    emit(n.op(), std::move(l), std::move(r), t);
    last_result_ = t;
}

void CodeGenerator::visit(cyberhand::core::NumberExprNode& n) {
    double v = n.value();
    if (v == std::floor(v)) last_result_ = std::to_string(static_cast<long long>(v));
    else { std::ostringstream oss; oss << v; last_result_ = oss.str(); }
}

void CodeGenerator::visit(cyberhand::core::VariableExprNode& n) {
    last_result_ = n.name();
}

static std::string renderTable(const std::vector<std::string>& hdrs,
                               const std::vector<std::vector<std::string>>& rows) {
    std::vector<size_t> w(hdrs.size());
    for (size_t i = 0; i < hdrs.size(); ++i) w[i] = hdrs[i].size();
    for (const auto& row : rows)
        for (size_t i = 0; i < row.size(); ++i) w[i] = std::max(w[i], row[i].size());

    auto sep = [&]() {
        std::string s = "+";
        for (size_t x : w) s += std::string(x + 2, '-') + "+";
        return s + "\n";
    };
    std::ostringstream out;
    out << sep() << "|";
    for (size_t i = 0; i < hdrs.size(); ++i)
        out << " " << std::left << std::setw(w[i]) << hdrs[i] << " |";
    out << "\n" << sep();
    for (const auto& row : rows) {
        out << "|";
        for (size_t i = 0; i < row.size(); ++i)
            out << " " << std::left << std::setw(w[i]) << row[i] << " |";
        out << "\n";
    }
    out << sep();
    return out.str();
}

std::string CodeGenerator::dumpQuadruples() const {
    std::vector<std::string> hdrs = {"Index", "Operador", "Arg1", "Arg2", "Resultado"};
    std::vector<std::vector<std::string>> rows;
    for (const auto& q : quadruples_) {
        auto f = [](const std::string& s) { return s.empty() ? "_" : s; };
        rows.push_back({std::to_string(q.index), f(q.op), f(q.arg1), f(q.arg2), f(q.result)});
    }
    return renderTable(hdrs, rows);
}

std::string CodeGenerator::dumpTriples() const {
    std::unordered_map<std::string, int> temp_to_idx;
    auto resolve = [&](const std::string& arg) -> std::string {
        if (arg.empty()) return "_";
        auto it = temp_to_idx.find(arg);
        return (it != temp_to_idx.end()) ? ("(" + std::to_string(it->second) + ")") : arg;
    };
    std::vector<std::string> hdrs = {"Index", "Operador", "Arg1", "Arg2"};
    std::vector<std::vector<std::string>> rows;
    for (const auto& q : quadruples_) {
        std::string a1 = resolve(q.arg1), a2 = resolve(q.arg2);
        if (q.op == "=") { a1 = q.result; a2 = resolve(q.arg1); }
        else if (q.op == "JUMP_IF_ZERO") { a2 = q.result; }
        rows.push_back({std::to_string(q.index), q.op, std::move(a1), std::move(a2)});
        if (!q.result.empty() && q.result[0] == 't') temp_to_idx[q.result] = q.index;
    }
    return renderTable(hdrs, rows);
}

void CodeGenerator::printQuadruples(std::ostream& os) const { os << dumpQuadruples(); }
void CodeGenerator::printTriples(std::ostream& os) const { os << dumpTriples(); }

} // namespace cyberhand::intermediate
