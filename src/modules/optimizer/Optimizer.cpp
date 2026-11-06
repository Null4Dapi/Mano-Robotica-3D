#include "modules/optimizer/Optimizer.hpp"
#include "core/TUI.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <unordered_map>
#include <map>

namespace cyberhand::optimizer {

static bool isNumber(const std::string& s) {
    if (s.empty()) return false;
    char* end = nullptr;
    std::strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0';
}

static std::string formatNumber(double v) {
    if (v == std::floor(v) && !std::isinf(v)) return std::to_string(static_cast<long long>(v));
    std::ostringstream oss; oss << v; return oss.str();
}

std::vector<BasicBlock> Optimizer::partitionIntoBasicBlocks(const std::vector<intermediate::Quadruple>& quads) {
    if (quads.empty()) return {};
    int n = static_cast<int>(quads.size());
    std::unordered_map<std::string, int> label_to_idx;
    for (int i = 0; i < n; ++i)
        if (quads[i].op == "LABEL" && !quads[i].arg1.empty()) label_to_idx[quads[i].arg1] = i;

    std::vector<bool> is_leader(n, false);
    is_leader[0] = true;
    for (int i = 0; i < n; ++i) {
        const auto& q = quads[i];
        std::string tgt = (q.op == "JUMP") ? q.arg1 : ((q.op == "JUMP_IF_ZERO") ? q.result : "");
        bool jump = (q.op == "JUMP" || q.op == "JUMP_IF_ZERO" || q.op == "RETURN");
        if (jump) {
            if (!tgt.empty() && label_to_idx.contains(tgt)) is_leader[label_to_idx[tgt]] = true;
            if (i + 1 < n) is_leader[i + 1] = true;
        }
    }

    std::vector<BasicBlock> blocks;
    std::unordered_map<int, int> inst_to_block;
    for (int i = 0; i < n; ++i) {
        if (is_leader[i]) blocks.push_back(BasicBlock{static_cast<int>(blocks.size()), {}, {}, {}});
        blocks.back().instructions.push_back(quads[i]);
        inst_to_block[i] = blocks.back().id;
    }

    for (size_t b = 0; b < blocks.size(); ++b) {
        const auto& last = blocks[b].instructions.back();
        if (last.op == "JUMP" && label_to_idx.contains(last.arg1)) {
            blocks[b].successors.push_back(inst_to_block[label_to_idx[last.arg1]]);
        } else if (last.op == "JUMP_IF_ZERO") {
            if (label_to_idx.contains(last.result)) blocks[b].successors.push_back(inst_to_block[label_to_idx[last.result]]);
            if (b + 1 < blocks.size()) blocks[b].successors.push_back(static_cast<int>(b + 1));
        } else if (last.op != "RETURN" && b + 1 < blocks.size()) {
            blocks[b].successors.push_back(static_cast<int>(b + 1));
        }
    }
    for (const auto& bb : blocks)
        for (int succ : bb.successors) blocks[succ].predecessors.push_back(bb.id);

    return blocks;
}

void Optimizer::optimizeBlock(BasicBlock& block) {
    std::unordered_map<std::string, std::string> const_map;
    // Pass 1: Constant Propagation and Folding
    for (auto& q : block.instructions) {
        if (const_map.contains(q.arg1)) q.arg1 = const_map[q.arg1];
        if (const_map.contains(q.arg2)) q.arg2 = const_map[q.arg2];
        if ((q.op == "+" || q.op == "-" || q.op == "*" || q.op == "/") && isNumber(q.arg1) && isNumber(q.arg2)) {
            double a = std::stod(q.arg1), b = std::stod(q.arg2), res = 0.0;
            if (q.op == "+") res = a + b;
            else if (q.op == "-") res = a - b;
            else if (q.op == "*") res = a * b;
            else if (q.op == "/" && b != 0.0) res = a / b;
            q.op = "="; q.arg1 = formatNumber(res); q.arg2 = "_";
            const_map[q.result] = q.arg1;
        } else if (q.op == "=" && isNumber(q.arg1)) {
            const_map[q.result] = q.arg1;
        } else if (!q.result.empty() && q.result != "_") {
            const_map.erase(q.result);
        }
    }
    // Pass 2: Kinematic Dead Code Elimination (redundant consecutive rotations, 0-ms waits)
    std::vector<intermediate::Quadruple> filtered;
    for (size_t i = 0; i < block.instructions.size(); ++i) {
        const auto& q = block.instructions[i];
        if (q.op == "ROTAR" && i + 1 < block.instructions.size() &&
            block.instructions[i + 1].op == "ROTAR" && q.arg1 == block.instructions[i + 1].arg1) continue;
        if (q.op == "ESPERAR" && (q.arg1 == "0" || q.arg1 == "0.0")) continue;
        filtered.push_back(q);
    }
    block.instructions = std::move(filtered);

    // Pass 3: Dead Code Elimination for unused intermediate temporaries (t*)
    bool changed = true;
    while (changed) {
        changed = false;
        std::unordered_map<std::string, int> uses;
        for (const auto& q : block.instructions) {
            if (!q.arg1.empty() && q.arg1 != "_") uses[q.arg1]++;
            if (!q.arg2.empty() && q.arg2 != "_") uses[q.arg2]++;
        }
        std::vector<intermediate::Quadruple> next_instrs;
        for (const auto& q : block.instructions) {
            bool is_temp = !q.result.empty() && q.result[0] == 't';
            bool is_def = (q.op == "=" || q.op == "+" || q.op == "-" || q.op == "*" || q.op == "/");
            if (is_temp && is_def && uses[q.result] == 0) { changed = true; continue; }
            next_instrs.push_back(q);
        }
        block.instructions = std::move(next_instrs);
    }
}

std::vector<intermediate::Quadruple> Optimizer::optimize(const std::vector<intermediate::Quadruple>& quads) {
    original_quads_ = quads;
    original_count_ = quads.size();
    basic_blocks_ = partitionIntoBasicBlocks(quads);
    optimized_quads_.clear();
    for (auto& block : basic_blocks_) {
        optimizeBlock(block);
        for (auto& q : block.instructions) optimized_quads_.push_back(q);
    }
    for (size_t i = 0; i < optimized_quads_.size(); ++i) optimized_quads_[i].index = static_cast<int>(i);
    optimized_count_ = optimized_quads_.size();
    reduction_percentage_ = (original_count_ > 0)
        ? (100.0 * static_cast<double>(original_count_ - optimized_count_) / static_cast<double>(original_count_)) : 0.0;
    return optimized_quads_;
}

bool Optimizer::exportTrajectoryJson(const std::vector<intermediate::Quadruple>& quads, const std::string& filepath) const {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;
    std::map<std::string, double> joints;
    for (const auto* f : {"ANULAR", "INDICE", "MEDIO", "MENIQUE", "PULGAR"})
        for (const auto* p : {"DISTAL", "MEDIAL", "PROXIMAL"}) joints[std::string(f) + "_" + p] = 0.0;
    joints["MUNICA_PITCH"] = joints["MUNICA_ROLL"] = joints["MUNICA_YAW"] = 0.0;

    auto normalize = [](const std::string& s) {
        std::string up; for (char c : s) up.push_back(static_cast<char>(std::toupper(c)));
        size_t sep = up.find_first_of(":_");
        if (sep == std::string::npos) return up;
        std::string f = up.substr(0, sep), p = up.substr(sep + 1);
        if (f == "MUNECA") f = "MUNICA";
        if (p == "FALANGE_1") p = "PROXIMAL"; else if (p == "FALANGE_2") p = "MEDIAL";
        else if (p == "FALANGE_3") p = "DISTAL"; else if (p == "EJE_X") p = "ROLL";
        else if (p == "EJE_Y") p = "PITCH"; else if (p == "EJE_Z") p = "YAW";
        return f + "_" + p;
    };

    std::map<std::string, double> mem;
    std::vector<std::pair<int, std::map<std::string, double>>> frames = {{0, joints}};
    int t = 0;
    for (const auto& q : quads) {
        if (q.op == "=" || q.op == "ASIGNAR") {
            mem[q.result] = isNumber(q.arg1) ? std::stod(q.arg1) : (mem.contains(q.arg1) ? mem[q.arg1] : 0.0);
        } else if (q.op == "ROTAR") {
            std::string k = normalize(q.arg1);
            if (joints.contains(k)) joints[k] = isNumber(q.arg2) ? std::stod(q.arg2) : (mem.contains(q.arg2) ? mem[q.arg2] : 0.0);
            t += 100; frames.push_back({t, joints});
        } else if (q.op == "ESPERAR") {
            int dur = isNumber(q.arg1) ? std::stoi(q.arg1) : (mem.contains(q.arg1) ? static_cast<int>(mem[q.arg1]) : 0);
            if (dur > 0) { t += dur; frames.push_back({t, joints}); }
        }
    }

    out << "{\n  \"programa\": \"ManoBionica_V1\",\n  \"total_gdl\": 18,\n  \"duracion_total_ms\": " << t << ",\n  \"frames\": [\n";
    for (size_t i = 0; i < frames.size(); ++i) {
        out << "    {\n      \"tiempo_ms\": " << frames[i].first << ",\n      \"articulaciones\": {\n";
        size_t j = 0;
        for (const auto& [name, val] : frames[i].second) {
            out << "        \"" << name << "\": " << std::fixed << std::setprecision(1) << val << (++j < frames[i].second.size() ? ",\n" : "\n");
        }
        out << "      }\n    }" << (i + 1 < frames.size() ? ",\n" : "\n");
    }
    out << "  ]\n}\n";
    return true;
}

void Optimizer::printOptimizationReport(std::ostream& os) const {
    using namespace cyberhand::ui;
    os << "\n" << GRAY << TOP_L << hRule(66) << TOP_R << RESET << "\n";
    os << GRAY << V_LINE << BOLD << WHITE << " REPORTE DE OPTIMIZACION DE CUADRUPLAS (SEMANA 15)                 " << GRAY << V_LINE << RESET << "\n";
    os << GRAY << T_RIGHT << hRule(66) << T_LEFT << RESET << "\n";
    os << GRAY << V_LINE << RESET << "  Original:   " << WHITE << std::setw(4) << original_count_ << RESET << " instrucciones" << std::string(33, ' ') << GRAY << V_LINE << RESET << "\n";
    os << GRAY << V_LINE << RESET << "  Optimizado: " << GREEN << std::setw(4) << optimized_count_ << RESET << " instrucciones" << std::string(33, ' ') << GRAY << V_LINE << RESET << "\n";
    os << GRAY << V_LINE << RESET << "  Reduccion:  " << YELLOW << BOLD << std::fixed << std::setprecision(1) << reduction_percentage_ << "%" << RESET << std::string(45, ' ') << GRAY << V_LINE << RESET << "\n";
    os << GRAY << T_RIGHT << hRule(6) << T_DOWN << hRule(14) << T_DOWN << hRule(20) << T_DOWN << hRule(12) << T_DOWN << hRule(10) << T_LEFT << RESET << "\n";
    os << GRAY << V_LINE << BOLD << " ID   " << GRAY << V_LINE << BOLD << " OPERADOR     " << GRAY << V_LINE << BOLD << " ARG1               " << GRAY << V_LINE << BOLD << " ARG2       " << GRAY << V_LINE << BOLD << " RESULTADO" << GRAY << V_LINE << RESET << "\n";
    os << GRAY << T_RIGHT << hRule(6) << CROSS << hRule(14) << CROSS << hRule(20) << CROSS << hRule(12) << CROSS << hRule(10) << T_LEFT << RESET << "\n";
    for (const auto& q : optimized_quads_) {
        auto fmt = [](const std::string& s) { return s.empty() ? "_" : s; };
        os << GRAY << V_LINE << DIM << " " << std::left << std::setw(4) << q.index << " "
           << GRAY << V_LINE << CYAN << " " << std::setw(12) << fmt(q.op) << " "
           << GRAY << V_LINE << WHITE << " " << std::setw(18) << fmt(q.arg1) << " "
           << GRAY << V_LINE << WHITE << " " << std::setw(10) << fmt(q.arg2) << " "
           << GRAY << V_LINE << GREEN << " " << std::setw(8)  << fmt(q.result) << " "
           << GRAY << V_LINE << RESET << "\n";
    }
    os << GRAY << BOT_L << hRule(6) << T_UP << hRule(14) << T_UP << hRule(20) << T_UP << hRule(12) << T_UP << hRule(10) << BOT_R << RESET << "\n";
}

} // namespace cyberhand::optimizer
