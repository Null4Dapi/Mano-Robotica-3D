#ifndef CYBERHAND_OPTIMIZER_BASIC_BLOCK_HPP
#define CYBERHAND_OPTIMIZER_BASIC_BLOCK_HPP

#include "modules/intermediate/Quadruple.hpp"
#include <vector>

namespace cyberhand::optimizer {

struct BasicBlock {
    int id{0};
    std::vector<cyberhand::intermediate::Quadruple> instructions;
    std::vector<int> successors;
    std::vector<int> predecessors;
};

} // namespace cyberhand::optimizer

#endif // CYBERHAND_OPTIMIZER_BASIC_BLOCK_HPP
