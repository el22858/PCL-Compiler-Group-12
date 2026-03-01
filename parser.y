%{
    #include <cstdio>
    #include "lexer.hpp"
%}

%token T_dispose       "dispose"
%token T_forward       "forward"
%token T_nil           "nil"

%token T_procedure     "procedure"
%token T_begin         "begin"
%token T_end           "end"
%token T_program       "program"
%token T_result        "result"
%token T_return        "return"
%token T_goto          "goto"
%token T_label         "label"

%token T_while         "while"
%token T_do            "do"
    
%token T_if            "if"
%token T_then          "then"
%token T_else          "else"
    
%token T_var           "var"
%token T_function      "function"
%token T_set           ":="

%token T_integer       "integer"
%token T_real          "real"
%token T_array         "array"
%token T_boolean       "boolean"
%token T_char          "char"
%token T_of            "of"
%token T_new           "new"

%token T_and           "and"
%token T_or            "or"
%token T_not           "not"
%token T_mod           "mod"
%token T_div           "div"
%token T_noteq         "<>"
%token T_leq           "<="
%token T_geq           ">="

%token T_true          "true"
%token T_false         "false"

%token T_id
%token T_const
%token T_escape

%left '+' '-'
%left '*' '/' "div" "mod"
%left "and" "or"
%left "not"

%%

program : "program" T_id ';' body '.';

body : "begin" stmt_list "end";

stmt_list : | stmt_list stmt;

stmt : T_id '(' T_const ')' | call;

expr : lVal | rVal;
exprList : expr | exprList ',' expr;

lVal : T_id;
rVal : T_const | unop expr | expr binop expr;

call : T_id '(' exprList ')'

unop : "not" | '+' | '-';
binop : '+' | '-' | '*' | '/' | "div" | "mod" | "or" | "and" | '=' | "<>" | '<' | "<=" | '>' | ">=";


%%

void yyerror(const char *msg) {
    printf("Syntax error: %s\n", msg);
    exit(42);
}

int main() {
    int result = yyparse();
    if (result == 0) printf("Success.\n");
    return result;
}