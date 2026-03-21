%{
    #include "ast.hpp"
    #include "lexer.hpp"
    #include "parser.hpp"
    #include "symbol.hpp"
    #include <vector>

    #define T_eof 0
    std::vector<char *> ids, stringLits, operators;
    std::vector<char> constChars;
    std::vector<int> constInts;
    std::vector<double> constReals;
%}

D [0-9]
L [a-zA-Z]
W [ \t\n\r]

%option noyywrap

%%

"and" { operators.push_back(strdup(yytext)); return T_and; }
"array" { return T_array; }
"begin" { return T_begin; }
"boolean" { return T_boolean; }
"char" { return T_char; }
"dispose" { return T_dispose; }
"div" { operators.push_back(strdup(yytext)); return T_div; }
"do" { return T_do; }
"else" { return T_else; }
"end" { return T_end; }
"false" { return T_false; }
"forward" { return T_forward; }
"function" { return T_function; }
"goto" { return T_goto; }
"if" { return T_if; }
"integer" { return T_integer; }
"label" { return T_label; }
"mod" { operators.push_back(strdup(yytext)); return T_mod; }
"new" { return T_new; }
"nil" { return T_nil; }
"not" { operators.push_back(strdup(yytext)); return T_not; }
"of" { return T_of; }
"or" { operators.push_back(strdup(yytext)); return T_or; }
"procedure" { return T_procedure; }
"program" { return T_program; }
"real" { return T_real; }
"result" { return T_result; }
"return" { return T_return; }
"then" { return T_then; }
"true" { return T_true; }
"var" { return T_var; }
"while" { return T_while; }

"<>" { operators.push_back(strdup(yytext)); return T_noteq; }
">=" { operators.push_back(strdup(yytext)); return T_geq; }
"<=" { operators.push_back(strdup(yytext)); return T_leq; }
":=" { operators.push_back(strdup(yytext)); return T_set; }

"[" { operators.push_back(strdup(yytext)); return T_left_br; }
"]" { operators.push_back(strdup(yytext)); return T_right_br; }
"+" { operators.push_back(strdup(yytext)); return T_op_plus; }
"-" { operators.push_back(strdup(yytext)); return T_op_minus; }
"*" { operators.push_back(strdup(yytext)); return T_op_mult; }
"/" { operators.push_back(strdup(yytext)); return T_op_div; }
"=" { operators.push_back(strdup(yytext)); return T_eq; }
"<" { operators.push_back(strdup(yytext)); return T_lt; }
">" { operators.push_back(strdup(yytext)); return T_gt; }
"^" { return T_deref; }
"@" { return T_mem; }
";" { return T_semCol; }
"(" { return T_leftPar; }
")" { return T_rightPar; }
"." { return T_period; }
":" { return T_col; }
"," { return T_comma; }



{L}({L}|{D}|_)*                             { ids.push_back(strdup(yytext)); return T_id; /* variable or function/procedure name */ }
{D}+\.{D}+(e[\+\-]?{D}+)?                   { constReals.push_back(stod(yytext)); return real_const; }
{D}+                                        { constInts.push_back(atoi(yytext)); return integer_const; }

'.' | '\n'                                  { constChars.push_back(strdup(yytext)[0]); return char_const; /* Any character, including '\n' */ }
\".*\"                                      { stringLits.push_back(strdup(yytext)); return string_lit; /* Strings, might have to change this so I can tell it's a char array */ }

{W}+ { /* Whitespace, do nothing */ }

\(\*.*\*\) { /* This one is for comments */ }

. { fprintf(stderr, "Illegal character %c\n", yytext[0]); }

%%

// int main(void) {
//     int token;
//     do {
//         token = yylex();
//         printf("token = %d, lexeme = %s\n", token, yytext);
//     } while (token != T_eof);
// }
