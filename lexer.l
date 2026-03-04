%{
    #include "lexer.hpp"
    #include "parser.hpp"
    #include "ast.hpp"
    #include <string>
    using namespace std;

    #define T_eof 0
%}

D [0-9]
L [a-zA-Z]
W [ \t\n\r]

%option noyywrap

%%

"and" { return T_and; }
"array" { return T_array; }
"begin" { return T_begin; }
"boolean" { return T_boolean; }
"char" { return T_char; }
"dispose" { return T_dispose; }
"div" { return T_div; }
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
"mod" { return T_mod; }
"new" { return T_new; }
"nil" { return T_nil; }
"not" { return T_not; }
"of" { return T_of; }
"or" { return T_or; }
"procedure" { return T_procedure; }
"program" { return T_program; }
"real" { return T_real; }
"result" { return T_result; }
"return" { return T_return; }
"then" { return T_then; }
"true" { return T_true; }
"var" { return T_var; }
"while" { return T_while; }

"<>" { yylval.var = "<>"; return T_noteq; }
">=" { yylval.var = ">="; return T_geq; }
"<=" { yylval.var = "<="; return T_leq; }
":=" { yylval.var = ":="; return T_set; }



{L}({L}|{D}|_)* { yylval.var = yytext; return T_id; /* variable or function/procedure name */ }
{D}+\.{D}+(e[\+\-]?{D}+)? { yylval.real = std::stof(yytext); return real_const; }
{D}+ { yylval.num = atoi(yytext); return integer_const; }

'.' | '\n' { yylval.c = yytext[0]; return char_const; /* Any character, including '\n' */ }
\".*\" { return string_lit; /* Strings, might have to change this so I can tell it's a char array */ }

{W}+ { /* Whitespace, do nothing */ }

[\=\+\-\*\\/<\^\>\@\[\]\;\.\(\)\:\,] { yylval.c = yytext[0]; return yytext[0]; /* Any single character we don't want in ' '  */ }

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