%{
#include "parser.tab.hpp"
#include <cstdlib>
#include <cstring>

int yycolumn = 1;

#define YY_USER_ACTION \
    yylloc.first_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_line = yylineno; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

%option noyywrap nounput noinput case-insensitive yylineno

%x COMMENT

%%

"//"[^\r\n]*            { /* ignore single-line comment */ }
"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>\n             { yycolumn = 1; }
<COMMENT>.              { /* ignore comment body */ }
<COMMENT><<EOF>>        { BEGIN(INITIAL); return 0; }

"inicio"                { return TOKEN_INICIO; }
"fin"                   { return TOKEN_FIN; }
"rutina"                { return TOKEN_RUTINA; }
"rotar"                 { return TOKEN_ROTAR; }
"esperar"               { return TOKEN_ESPERAR; }
"ms"                    { return TOKEN_MS; }
"repetir"               { return TOKEN_REPETIR; }
"veces"                 { return TOKEN_VECES; }
"grados"                { return TOKEN_GRADOS; }

[0-9]+(\.[0-9]+)?       { yylval.dval = std::atof(yytext); return TOKEN_NUMERO; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.sval = strdup(yytext); return TOKEN_IDENTIFICADOR; }

[+\-*/=;,:{}\(\)]       { return yytext[0]; }

[ \t\r]+                { /* ignore whitespace */ }
\n                      { yycolumn = 1; }

.                       { return yytext[0]; }

%%
