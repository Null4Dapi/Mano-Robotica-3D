#include "modules/optimizer/BasicBlock.hpp"
#include "modules/optimizer/Optimizer.hpp"
#include <iostream>
#include <cassert>
#include <fstream>
#include <filesystem>

int main() {
    using namespace cyberhand::intermediate;
    using namespace cyberhand::optimizer;

    std::cout << "=== Test Basic Block Optimizer & Trajectory Exporter (Semana 15) ===\n";
    Optimizer optimizer;

    // Test a: Basic block partitioning & leader identification
    {
        std::cout << "[Test a] Basic block partitioning and leader detection... ";
        std::vector<Quadruple> quads = {
            {0, "=", "3", "_", "t1"}, {1, "LABEL", "L1", "_", "_"},
            {2, "JUMP_IF_ZERO", "t1", "_", "L2"}, {3, "ROTAR", "indice:falange_1", "30", "_"},
            {4, "-", "t1", "1", "t2"}, {5, "=", "t2", "_", "t1"},
            {6, "JUMP", "L1", "_", "_"}, {7, "LABEL", "L2", "_", "_"}
        };
        auto blocks = optimizer.partitionIntoBasicBlocks(quads);
        assert(blocks.size() == 4);
        assert(blocks[0].id == 0 && blocks[0].instructions[0].index == 0 && blocks[0].instructions.size() == 1);
        assert(blocks[1].id == 1 && blocks[1].instructions[0].index == 1 && blocks[1].instructions.size() == 2);
        assert(blocks[2].id == 2 && blocks[2].instructions[0].index == 3 && blocks[2].instructions.size() == 4);
        assert(blocks[3].id == 3 && blocks[3].instructions[0].index == 7 && blocks[3].instructions.size() == 1);
        assert(blocks[0].successors == std::vector<int>{1});
        std::cout << "PASSED\n";
    }

    // Test b, c, d, e: Folding (5 * 2 + 15 -> 25), propagation, dead temporaries DCE, >= 15% reduction
    {
        std::cout << "[Test b, c, d, e] Folding, propagation, DCE, and reduction >= 15%... ";
        std::vector<Quadruple> quads = {
            {0, "*", "5", "2", "t1"}, {1, "+", "t1", "15", "t2"},
            {2, "=", "t2", "_", "angulo"}, {3, "ROTAR", "pulgar:falange_1", "angulo", "_"},
            {4, "ROTAR", "pulgar:falange_1", "angulo", "_"}, {5, "ESPERAR", "0", "ms", "_"}
        };
        auto opt_quads = optimizer.optimize(quads);
        bool found_rotation_with_25 = false, found_t1 = false, found_t2 = false;
        for (const auto& q : opt_quads) {
            if (q.op == "ROTAR" && q.arg2 == "25") found_rotation_with_25 = true;
            if (q.result == "t1" || q.arg1 == "t1" || q.arg2 == "t1") found_t1 = true;
            if (q.result == "t2" || q.arg1 == "t2" || q.arg2 == "t2") found_t2 = true;
        }
        assert(found_rotation_with_25 && !found_t1 && !found_t2);
        assert(optimizer.reductionPercentage() >= 15.0);
        assert(optimizer.optimizedInstructionCount() < optimizer.originalInstructionCount());
        for (size_t i = 0; i < opt_quads.size(); ++i) assert(opt_quads[i].index == static_cast<int>(i));
        std::cout << "PASSED (Reduction: " << optimizer.reductionPercentage() << "%)\n";
        optimizer.printOptimizationReport();
    }

    // Test f: JSON trajectory export & validation
    {
        std::cout << "[Test f] Trajectory JSON export and structure validation... ";
        std::string out_path = "bin/test_trayectoria.json";
        std::vector<Quadruple> quads = {
            {0, "=", "45", "_", "angulo"},
            {1, "ROTAR", "pulgar:falange_1", "angulo", "_"},
            {2, "ESPERAR", "200", "ms", "_"}
        };
        auto opt_quads = optimizer.optimize(quads);
        assert(optimizer.exportTrajectoryJson(opt_quads, out_path));
        assert(std::filesystem::exists(out_path));
        std::ifstream f(out_path);
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();
        std::filesystem::remove(out_path);
        assert(content.find("\"programa\":") != std::string::npos);
        assert(content.find("\"total_gdl\": 18") != std::string::npos);
        assert(content.find("\"duracion_total_ms\": 300") != std::string::npos);
        assert(content.find("\"frames\":") != std::string::npos);
        assert(content.find("\"PULGAR_PROXIMAL\": 45.0") != std::string::npos);
        std::cout << "PASSED\n";
    }

    std::cout << "\nTodas las pruebas del Optimizador y Exportador pasaron exitosamente.\n";
    return 0;
}
