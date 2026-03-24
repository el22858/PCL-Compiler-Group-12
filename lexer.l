%top{
    #include "ast.hpp"
    #include "lexer.hpp"
    #include "parser.hpp"
    #include "symbol.hpp"

    /* #define YY_DECL yy::parser::symbol_type ylex() */
}

%{
    #include <iostream>
    #include <memory>
    #include <vector>
    
    /* static yy::location loc; */

    #define T_eof 0
    std::vector<char *> ids, stringLits, operators;
    std::vector<char> constChars;
    std::vector<int> constInts;
    std::vector<double> constReals;

    /* #define YY_USER_ACTION loc.columns(yyleng); */
%}

D [0-9]
L [a-zA-Z]
W [ \t\n\r]

%option warn noyywrap nodefault nounput yylineno
%option outfile="lexer.cpp"

%%

%{
    /* loc.step(); */
%}

"and" { operators.push_back(strdup(yytext)); return T_and; /* return yy::parser::make_and(loc); */ }
"array" { return T_array; /* return yy::parser::make_array(loc); */ }
"begin" { return T_begin; /* return yy::parser::make_begin(loc); */ }
"boolean" { return T_boolean; /* return yy::parser::make_boolean(loc); */ }
"char" { return T_char; /* return yy::parser::make_char(loc); */ }
"dispose" { return T_dispose; /* return yy::parser::make_dispose(loc); */ }
"div" { operators.push_back(strdup(yytext)); return T_div; /* return yy::parser::make_div(loc); */ }
"do" { return T_do; /* return yy::parser::make_do(loc); */ }
"else" { return T_else; /* return yy::parser::make_else(loc); */ }
"end" { return T_end; /* return yy::parser::make_end(loc); */ }
"false" { return T_false; /* return yy::parser::make_false(loc); */ }
"forward" { return T_forward; /* return yy::parser::make_forward(loc); */ }
"function" { return T_function; /* return yy::parser::make_function(loc); */ }
"goto" { return T_goto; /* return yy::parser::make_goto(loc); */ }
"if" { return T_if; /* return yy::parser::make_if(loc); */ }
"integer" { return T_integer; /* return yy::parser::make_integer(loc); */ }
"label" { return T_label; /* return yy::parser::make_label(loc); */ }
"mod" { operators.push_back(strdup(yytext)); return T_mod; /* return yy::parser::make_mod(loc); */ }
"new" { return T_new; /* return yy::parser::make_new(loc); */ }
"nil" { return T_nil; /* return yy::parser::make_nil(loc); */ }
"not" { operators.push_back(strdup(yytext)); return T_not; /* return yy::parser::make_not(loc); */ }
"of" { return T_of; /* return yy::parser::make_of(loc); */ }
"or" { operators.push_back(strdup(yytext)); return T_or; /* return yy::parser::make_or(loc); */ }
"procedure" { return T_procedure; /* return yy::parser::make_procedure(loc); */ }
"program" { return T_program; /* return yy::parser::make_program(loc); */ }
"real" { return T_real; /* return yy::parser::make_real(loc); */ }
"result" { return T_result; /* return yy::parser::make_result(loc); */ }
"return" { return T_return; /* return yy::parser::make_return(loc); */ }
"then" { return T_then; /* return yy::parser::make_then(loc); */ }
"true" { return T_true; /* return yy::parser::make_true(loc); */ }
"var" { return T_var; /* return yy::parser::make_var(loc); */ }
"while" { return T_while; /* return yy::parser::make_while(loc); */ }

"<>" { operators.push_back(strdup(yytext)); return T_noteq; /* return yy::parser::make_noteq(loc); */ }
">=" { operators.push_back(strdup(yytext)); return T_geq; /* return yy::parser::make_geq(loc); */ }
"<=" { operators.push_back(strdup(yytext)); return T_leq; /* return yy::parser::make_leq(loc); */ }
":=" { operators.push_back(strdup(yytext)); return T_set; /* return yy::parser::make_set(loc); */ }

"[" { operators.push_back(strdup(yytext)); return T_left_br; /* return yy::parser::make_left_br(loc); */ }
"]" { operators.push_back(strdup(yytext)); return T_right_br; /* return yy::parser::make_right_br(loc); */ }
"+" { operators.push_back(strdup(yytext)); return T_op_plus; /* return yy::parser::make_op_plus(loc); */ }
"-" { operators.push_back(strdup(yytext)); return T_op_minus; /* return yy::parser::make_op_minus(loc); */ }
"*" { operators.push_back(strdup(yytext)); return T_op_mult; /* return yy::parser::make_op_mult(loc); */ }
"/" { operators.push_back(strdup(yytext)); return T_op_div; /* return yy::parser::make_op_div(loc); */ }
"=" { operators.push_back(strdup(yytext)); return T_eq; /* return yy::parser::make_eq(loc); */ }
"<" { operators.push_back(strdup(yytext)); return T_lt; /* return yy::parser::make_lt(loc); */ }
">" { operators.push_back(strdup(yytext)); return T_gt; /* return yy::parser::make_gt(loc); */ }
"^" { return T_deref; /* return yy::parser::make_deref(loc); */ }
"@" { return T_mem; /* return yy::parser::make_mem(loc); */ }
";" { return T_semCol; /* return yy::parser::make_semCol(loc); */ }
"(" { return T_leftPar; /* return yy::parser::make_leftPar(loc); */ }
")" { return T_rightPar; /* return yy::parser::make_rightPar(loc); */ }
"." { return T_period; /* return yy::parser::make_period(loc); */ }
":" { return T_col; /* return yy::parser::make_col(loc); */ }
"," { return T_comma; /* return yy::parser::make_comma(loc); */ }



{L}({L}|{D}|_)*                             { ids.push_back(strdup(yytext)); return T_id; /* return yy::parser::make_id(loc); */ /* variable or function/procedure name */ }
{D}+\.{D}+(e[\+\-]?{D}+)?                   { constReals.push_back(stod(yytext)); return real_const; /* return yy::parser::make_real_const(loc); */ }
{D}+                                        { constInts.push_back(atoi(yytext)); return integer_const; /* return yy::parser::make_integer_const(loc); */ }

'.' | '\n'                                  { constChars.push_back(strdup(yytext)[0]); return char_const; /* return yy::parser::make_char_const(loc); */ /* Any character, including '\n' */ }
\".*\"                                      { stringLits.push_back(strdup(yytext)); return string_lit; /* return yy::parser::make_string_lit(loc); */ /* Strings, might have to change this so I can tell it's a char array */ }

{W}+ { /* Whitespace, do nothing */ }

\(\*.*\*\) { /* This one is for comments */ }

. { std::cerr << "Illegal character '" << yytext[0] << "' found at line " /* << loc.begin.lin */ << "." << std::endl; exit(1); }

%%