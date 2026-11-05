#ifndef CYBERHAND_INTERMEDIATE_QUADRUPLE_HPP
#define CYBERHAND_INTERMEDIATE_QUADRUPLE_HPP

#include <string>
#include <utility>

namespace cyberhand::intermediate {

struct Quadruple {
    int index{0};
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;

    Quadruple() = default;
    Quadruple(int idx, std::string op_, std::string a1, std::string a2, std::string res)
        : index(idx), op(std::move(op_)), arg1(std::move(a1)), arg2(std::move(a2)), result(std::move(res)) {}

    [[nodiscard]] std::string toString() const {
        auto fmt = [](const std::string& s) { return s.empty() ? "_" : s; };
        return "(" + fmt(op) + ", " + fmt(arg1) + ", " + fmt(arg2) + ", " + fmt(result) + ")";
    }
};

} // namespace cyberhand::intermediate

#endif // CYBERHAND_INTERMEDIATE_QUADRUPLE_HPP
