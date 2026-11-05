%code requires {
    #include "core/AST.hpp"
    namespace cyberhand::grammar { class ParserDriver; }
}

%{
#include "core/AST.hpp"
#include "grammar/ParserDriver.hpp"
#include <vector>
#include <memory>
#include <cstdlib>

int yylex();
void yyerror(cyberhand::grammar::ParserDriver* driver, const char* s);
%}

%parse-param { cyberhand::grammar::ParserDriver* driver }
%locations

%union {
    double dval;
    char* sval;
    cyberhand::core::ExprNode* expr;
    cyberhand::core::StmtNode* stmt;
    cyberhand::core::RoutineDefNode* routine;
    std::vector<std::unique_ptr<cyberhand::core::StmtNode>>* stmts;
    std::vector<std::unique_ptr<cyberhand::core::RoutineDefNode>>* routines;
}

%token TOKEN_INICIO TOKEN_FIN TOKEN_RUTINA TOKEN_ROTAR TOKEN_ESPERAR
%token TOKEN_MS TOKEN_REPETIR TOKEN_VECES TOKEN_GRADOS
%token <dval> TOKEN_NUMERO
%token <sval> TOKEN_IDENTIFICADOR

%left '+' '-'
%left '*' '/'
%precedence UMINUS

%type <routines> routine_list
%type <routine> routine_def
%type <stmts> stmt_list main_block
%type <stmt> stmt rotate_cmd wait_cmd repeat_cmd assign_stmt
%type <expr> expr

%%

program
    : routine_list main_block {
          auto prog = std::make_unique<cyberhand::core::ProgramNode>(
              std::move(*$1), std::move(*$2), @$.first_line, @$.first_column);
          delete $1; delete $2;
          driver->setAST(std::move(prog));
      }
    ;

routine_list
    : /* empty */ { $$ = new std::vector<std::unique_ptr<cyberhand::core::RoutineDefNode>>(); }
    | routine_list routine_def { $1->push_back(std::unique_ptr<cyberhand::core::RoutineDefNode>($2)); $$ = $1; }
    ;

routine_def
    : TOKEN_RUTINA TOKEN_IDENTIFICADOR opt_params '{' stmt_list '}' {
          $$ = new cyberhand::core::RoutineDefNode($2, std::move(*$5), @1.first_line, @1.first_column);
          free($2); delete $5;
      }
    ;

opt_params: /* empty */ | '(' ')' ;
main_block: TOKEN_INICIO stmt_list TOKEN_FIN { $$ = $2; } | stmt_list { $$ = $1; } ;

stmt_list
    : /* empty */ { $$ = new std::vector<std::unique_ptr<cyberhand::core::StmtNode>>(); }
    | stmt_list stmt {
          if ($2) $1->push_back(std::unique_ptr<cyberhand::core::StmtNode>($2));
          $$ = $1;
      }
    ;

stmt: rotate_cmd | wait_cmd | repeat_cmd | assign_stmt | ';' { $$ = nullptr; } ;

rotate_cmd
    : TOKEN_ROTAR TOKEN_IDENTIFICADOR TOKEN_IDENTIFICADOR expr opt_grados ';' {
          $$ = new cyberhand::core::RotateCmdNode($2, $3, std::unique_ptr<cyberhand::core::ExprNode>($4), @1.first_line, @1.first_column);
          free($2); free($3);
      }
    ;

opt_grados: /* empty */ | TOKEN_GRADOS ;

wait_cmd
    : TOKEN_ESPERAR expr opt_ms ';' {
          $$ = new cyberhand::core::WaitCmdNode(std::unique_ptr<cyberhand::core::ExprNode>($2), @1.first_line, @1.first_column);
      }
    ;

opt_ms: /* empty */ | TOKEN_MS ;

repeat_cmd
    : TOKEN_REPETIR expr opt_veces '{' stmt_list '}' {
          $$ = new cyberhand::core::RepeatCmdNode(std::unique_ptr<cyberhand::core::ExprNode>($2), std::move(*$5), @1.first_line, @1.first_column);
          delete $5;
      }
    ;

opt_veces: /* empty */ | TOKEN_VECES ;

assign_stmt
    : TOKEN_IDENTIFICADOR '=' expr ';' {
          $$ = new cyberhand::core::AssignStmtNode($1, std::unique_ptr<cyberhand::core::ExprNode>($3), @1.first_line, @1.first_column);
          free($1);
      }
    ;

expr
    : expr '+' expr { $$ = new cyberhand::core::BinaryExprNode("+", std::unique_ptr<cyberhand::core::ExprNode>($1), std::unique_ptr<cyberhand::core::ExprNode>($3), @$.first_line, @$.first_column); }
    | expr '-' expr { $$ = new cyberhand::core::BinaryExprNode("-", std::unique_ptr<cyberhand::core::ExprNode>($1), std::unique_ptr<cyberhand::core::ExprNode>($3), @$.first_line, @$.first_column); }
    | expr '*' expr { $$ = new cyberhand::core::BinaryExprNode("*", std::unique_ptr<cyberhand::core::ExprNode>($1), std::unique_ptr<cyberhand::core::ExprNode>($3), @$.first_line, @$.first_column); }
    | expr '/' expr { $$ = new cyberhand::core::BinaryExprNode("/", std::unique_ptr<cyberhand::core::ExprNode>($1), std::unique_ptr<cyberhand::core::ExprNode>($3), @$.first_line, @$.first_column); }
    | '-' expr %prec UMINUS {
          auto z = std::make_unique<cyberhand::core::NumberExprNode>(0.0, @1.first_line, @1.first_column);
          $$ = new cyberhand::core::BinaryExprNode("-", std::move(z), std::unique_ptr<cyberhand::core::ExprNode>($2), @$.first_line, @$.first_column);
      }
    | '(' expr ')' { $$ = $2; }
    | TOKEN_NUMERO { $$ = new cyberhand::core::NumberExprNode($1, @1.first_line, @1.first_column); }
    | TOKEN_IDENTIFICADOR { $$ = new cyberhand::core::VariableExprNode($1, @1.first_line, @1.first_column); free($1); }
    ;

%%

void yyerror(cyberhand::grammar::ParserDriver* driver, const char* s) {
    driver->setError(s, yylloc.first_line, yylloc.first_column);
}
