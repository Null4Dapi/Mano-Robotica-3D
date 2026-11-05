#include "grammar/ParserDriver.hpp"
#include "parser.tab.hpp"
#include <cstdio>

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE b);
extern FILE *yyin;
extern int yylineno;
extern int yycolumn;
extern void yyrestart(FILE *input_file);
int yyparse(cyberhand::grammar::ParserDriver* driver);

namespace cyberhand::grammar {

std::unique_ptr<cyberhand::core::ProgramNode> ParserDriver::parseString(const std::string& input) {
    has_error_ = false;
    error_msg_.clear();
    error_line_ = 0;
    error_col_ = 0;
    ast_.reset();

    yylineno = 1;
    yycolumn = 1;
    YY_BUFFER_STATE buffer = yy_scan_string(input.c_str());
    int res = yyparse(this);
    yy_delete_buffer(buffer);

    return (res == 0 && !has_error_) ? std::move(ast_) : nullptr;
}

std::unique_ptr<cyberhand::core::ProgramNode> ParserDriver::parseFile(const std::string& filepath) {
    has_error_ = false;
    error_msg_.clear();
    error_line_ = 0;
    error_col_ = 0;
    ast_.reset();

    FILE* fp = std::fopen(filepath.c_str(), "r");
    if (!fp) {
        setError("Error opening file: " + filepath, 1, 1);
        return nullptr;
    }

    yylineno = 1;
    yycolumn = 1;
    yyrestart(fp);
    int res = yyparse(this);
    std::fclose(fp);

    return (res == 0 && !has_error_) ? std::move(ast_) : nullptr;
}

} // namespace cyberhand::grammar
