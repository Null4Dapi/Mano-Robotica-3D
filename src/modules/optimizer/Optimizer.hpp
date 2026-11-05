#ifndef CYBERHAND_OPTIMIZER_OPTIMIZER_HPP
#define CYBERHAND_OPTIMIZER_OPTIMIZER_HPP

#include "modules/optimizer/BasicBlock.hpp"
#include <vector>
#include <string>
#include <iostream>

namespace cyberhand::optimizer {

class Optimizer {
public:
    Optimizer() = default;
    explicit Optimizer(std::vector<cyberhand::intermediate::Quadruple> quads) : original_quads_(std::move(quads)) {}

    std::vector<BasicBlock> partitionIntoBasicBlocks(const std::vector<cyberhand::intermediate::Quadruple>& quads);
    std::vector<BasicBlock> partitionIntoBasicBlocks() { return partitionIntoBasicBlocks(original_quads_); }

    std::vector<cyberhand::intermediate::Quadruple> optimize(const std::vector<cyberhand::intermediate::Quadruple>& quads);
    std::vector<cyberhand::intermediate::Quadruple> optimize() { return optimize(original_quads_); }

    bool exportTrajectoryJson(const std::vector<cyberhand::intermediate::Quadruple>& quads, const std::string& filepath) const;
    bool exportTrajectoryJson(const std::string& filepath) const { return exportTrajectoryJson(optimized_quads_, filepath); }

    void printOptimizationReport(std::ostream& os = std::cout) const;

    [[nodiscard]] size_t originalInstructionCount() const noexcept { return original_count_; }
    [[nodiscard]] size_t optimizedInstructionCount() const noexcept { return optimized_count_; }
    [[nodiscard]] double reductionPercentage() const noexcept { return reduction_percentage_; }
    [[nodiscard]] const std::vector<cyberhand::intermediate::Quadruple>& originalQuadruples() const noexcept { return original_quads_; }
    [[nodiscard]] const std::vector<cyberhand::intermediate::Quadruple>& optimizedQuadruples() const noexcept { return optimized_quads_; }
    [[nodiscard]] const std::vector<BasicBlock>& basicBlocks() const noexcept { return basic_blocks_; }

private:
    std::vector<cyberhand::intermediate::Quadruple> original_quads_, optimized_quads_;
    std::vector<BasicBlock> basic_blocks_;
    size_t original_count_{0}, optimized_count_{0};
    double reduction_percentage_{0.0};
    void optimizeBlock(BasicBlock& block);
};

} // namespace cyberhand::optimizer
#endif
