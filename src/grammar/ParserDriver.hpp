#ifndef CYBERHAND_PARSER_DRIVER_HPP
#define CYBERHAND_PARSER_DRIVER_HPP

#include "core/AST.hpp"
#include <memory>
#include <string>

namespace cyberhand::grammar {

class ParserDriver {
public:
    ParserDriver() = default;

    std::unique_ptr<cyberhand::core::ProgramNode> parseString(const std::string& input);
    std::unique_ptr<cyberhand::core::ProgramNode> parseFile(const std::string& filepath);

    [[nodiscard]] bool hasError() const noexcept { return has_error_; }
    [[nodiscard]] const std::string& errorMessage() const noexcept { return error_msg_; }
    [[nodiscard]] int errorLine() const noexcept { return error_line_; }
    [[nodiscard]] int errorColumn() const noexcept { return error_col_; }

    void setError(std::string msg, int line, int col) {
        has_error_ = true;
        error_msg_ = std::move(msg);
        error_line_ = line;
        error_col_ = col;
    }

    void setAST(std::unique_ptr<cyberhand::core::ProgramNode> ast) { ast_ = std::move(ast); }

private:
    std::unique_ptr<cyberhand::core::ProgramNode> ast_{nullptr};
    bool has_error_{false};
    std::string error_msg_{};
    int error_line_{0};
    int error_col_{0};
};

} // namespace cyberhand::grammar

#endif // CYBERHAND_PARSER_DRIVER_HPP
