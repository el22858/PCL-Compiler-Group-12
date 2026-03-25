%top{
    #include "ast.hpp"
    #include "basic.hpp"
    #include "parser.hpp"
    #include "symbol.hpp"

    #define YY_DECL yy::parser::symbol_type ylex()
}

%{
    #include <iostream>
    #include <memory>
    #include <vector>
    
    static yy::location loc;

    #define T_eof 0
    std::vector<char *> ids, stringLits, operators;
    std::vector<char> constChars;
    std::vector<int> constInts;
    std::vector<double> constReals;

    #define YY_USER_ACTION loc.columns(yyleng);
%}

D [0-9]
L [a-zA-Z]
W [ \t\n\r]

%option warn noyywrap nodefault nounput yylineno
%option header-file="lexer.hpp"
%option outfile="lexer.cpp"

%%

%{
    loc.step();
%}

"and" { operators.push_back(strdup(yytext)); /* return T_and; */ return yy::parser::make_T_and(yytext, loc);  }
"array" { /* return T_array; */ return yy::parser::make_T_array(loc); }
"begin" { /* return T_begin; */ return yy::parser::make_T_begin(loc); }
"boolean" { /* return T_boolean; */ return yy::parser::make_T_boolean(loc); }
"char" { /* return T_char; */ return yy::parser::make_T_char(loc); }
"dispose" { /* return T_dispose; */ return yy::parser::make_T_dispose(loc); }
"div" { operators.push_back((yytext)); /* return T_div; */ return yy::parser::make_T_div(yytext, loc); }
"do" { /* return T_do; */ return yy::parser::make_T_do(loc); }
"else" { /* return T_else; */ return yy::parser::make_T_else(loc); }
"end" { /* return T_end; */ return yy::parser::make_T_end(loc); }
"false" { /* return T_false; */ return yy::parser::make_T_false(loc); }
"forward" { /* return T_forward; */ return yy::parser::make_T_forward(loc); }
"function" { /* return T_function; */ return yy::parser::make_T_function(loc); }
"goto" { /* return T_goto; */ return yy::parser::make_T_goto(loc); }
"if" { /* return T_if; */ return yy::parser::make_T_if(loc); }
"integer" { /* return T_integer; */ return yy::parser::make_T_integer(loc); }
"label" { /* return T_label; */ return yy::parser::make_T_label(loc); }
"mod" { operators.push_back((yytext)); /* return T_mod; */ return yy::parser::make_T_mod(yytext, loc); }
"new" { /* return T_new; */ return yy::parser::make_T_new(loc); }
"nil" { /* return T_nil; */ return yy::parser::make_T_nil(loc); }
"not" { operators.push_back((yytext)); /* return T_not; */ return yy::parser::make_T_not(yytext, loc); }
"of" { /* return T_of; */ return yy::parser::make_T_of(loc); }
"or" { operators.push_back((yytext)); /* return T_or; */ return yy::parser::make_T_or(yytext, loc); }
"procedure" { /* return T_procedure; */ return yy::parser::make_T_procedure(loc); }
"program" { /* return T_program; */ return yy::parser::make_T_program(loc); }
"real" { /* return T_real; */ return yy::parser::make_T_real(loc); }
"result" { /* return T_result; */ return yy::parser::make_T_result(loc); }
"return" { /* return T_return; */ return yy::parser::make_T_return(loc); }
"then" { /* return T_then; */ return yy::parser::make_T_then(loc); }
"true" { /* return T_true; */ return yy::parser::make_T_true(loc); }
"var" { /* return T_var; */ return yy::parser::make_T_var(loc); }
"while" { /* return T_while; */ return yy::parser::make_T_while(loc); }

"<>" { operators.push_back(strdup(yytext)); /* return T_noteq; */ return yy::parser::make_T_noteq(yytext, loc); }
">=" { operators.push_back(strdup(yytext)); /* return T_geq; */ return yy::parser::make_T_geq(yytext, loc); }
"<=" { operators.push_back(strdup(yytext)); /* return T_leq; */ return yy::parser::make_T_leq(yytext, loc); }
":=" { operators.push_back(strdup(yytext)); /* return T_set; */ return yy::parser::make_T_set(loc); }

"[" { operators.push_back(strdup(yytext)); /* return T_left_br; */ return yy::parser::make_T_left_br(yytext, loc); }
"]" { operators.push_back(strdup(yytext)); /* return T_right_br; */ return yy::parser::make_T_right_br(yytext, loc); }
"+" { operators.push_back(strdup(yytext)); /* return T_op_plus; */ return yy::parser::make_T_op_plus(yytext, loc); }
"-" { operators.push_back(strdup(yytext)); /* return T_op_minus; */ return yy::parser::make_T_op_minus(yytext, loc); }
"*" { operators.push_back(strdup(yytext)); /* return T_op_mult; */ return yy::parser::make_T_op_mult(yytext, loc); }
"/" { operators.push_back(strdup(yytext)); /* return T_op_div; */ return yy::parser::make_T_op_div(yytext, loc); }
"=" { operators.push_back(strdup(yytext)); /* return T_eq; */ return yy::parser::make_T_eq(yytext, loc); }
"<" { operators.push_back(strdup(yytext)); /* return T_lt; */ return yy::parser::make_T_lt(yytext, loc); }
">" { operators.push_back(strdup(yytext)); /* return T_gt; */ return yy::parser::make_T_gt(yytext, loc); }
"^" { /* return T_deref; */ return yy::parser::make_T_deref(yytext, loc); }
"@" { /* return T_mem; */ return yy::parser::make_T_mem(yytext, loc); }
";" { /* return T_semCol; */ return yy::parser::make_T_semCol(loc); }
"(" { /* return T_leftPar; */ return yy::parser::make_T_leftPar(yytext, loc); }
")" { /* return T_rightPar; */ return yy::parser::make_T_rightPar(yytext, loc); }
"." { /* return T_period; */ return yy::parser::make_T_period(loc); }
":" { /* return T_col; */ return yy::parser::make_T_col(loc); }
"," { /* return T_comma; */ return yy::parser::make_T_comma(loc); }



{L}({L}|{D}|_)*                             { ids.push_back(strdup(yytext)); /* return T_id; */ return yy::parser::make_T_id(strdup(yytext), loc); /* variable or function/procedure name */ }
{D}+\.{D}+(e[\+\-]?{D}+)?                   { constReals.push_back(stod(yytext)); /* return real_const; */ return yy::parser::make_real_const(stod(yytext),loc); }
{D}+                                        { constInts.push_back(atoi(yytext)); /* return integer_const; */ return yy::parser::make_integer_const(atoi(yytext), loc); }

'.' | '\n'                                  { constChars.push_back(strdup(yytext)[0]); /* return char_const; */ return yy::parser::make_char_const(strdup(yytext)[0], loc); /* Any character, including '\n' */ }
\".*\"                                      { stringLits.push_back(strdup(yytext)); /* return string_lit; */ return yy::parser::make_string_lit(strdup(yytext), loc); /* Strings, might have to change this so I can tell it's a char array */ }

{W}+ { /* Whitespace, do nothing */ }

\(\*.*\*\) { /* This one is for comments */ }

. { std::cerr << "Illegal character '" << yytext[0] << "' found at line " /* << loc.begin.lin */ << "." << std::endl; exit(1); }

<<EOF>> { return yy::parser::make_YYEOF(loc); }

%%