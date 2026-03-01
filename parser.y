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
%token integer_const
%token real_const
%token char_const
%token string_lit
%token T_escape

%left '+' '-'
%left '*' '/' "div" "mod"
%left "and" "or"
%left "not"

%%

program : "program" T_id ';' body '.';

body : localList block;
localList : | localList local;
local : "var" decList | "label" idList ';' | header ';' body ';' | "forward" header ';';
idList : T_id | idList ',' T_id; 
decList : declaration | decList declaration; 
declaration : idList ':' type ';'

header : "procedure" T_id '(' optFormalList ')' | "function" T_id '(' optFormalList ')' ':' type;
optFormalList : | formalList;
formalList : formal | formalList ';' formal;

formal : optVar idList ':' type;
optVar :  | "var";

type : "integer" | "real" | "boolean" | "char" | "array" optLength "of" type | "^" type;
optLength : | '[' integer_const ']';

block : "begin" stmt_list "end";
stmt_list : stmt | stmt_list 'e' stmt;

stmt :  | lVal ":=" expr | block | call | "if" expr "then" stmt optElse | "while" expr "do" stmt | T_id ':' stmt | "goto" T_id | "return" | "new" optObj lVal | "dispose" optArr lVal;
optElse :  | "else" stmt;
optObj : | '[' expr ']';
optArr : | '[' ']';

expr : lVal | rVal;
exprList : expr | exprList ',' expr;

lVal : T_id | "result" | string_lit | lVal '[' expr ']' | expr '^' | '(' lVal ')';
rVal : integer_const | "true" | "false" | real_const | char_const | '(' rVal ')' | "nil" | call | '@' lVal | unop expr | expr binop expr;

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