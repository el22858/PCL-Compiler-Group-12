%{
    #include <cstdio>
    #include <string>
    #include "lexer.hpp"
    #include "ast.hpp"
    using namespace std;
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

%token<var> T_id
%token<num> integer_const
%token<real> real_const
%token<c> char_const
%token<var> string_lit
%token T_escape

%union{
    string var;
    int num;
    float real;
    char c;
    Stmt *stmt;
    Expr *expr;
    Block *blk;
    ExprList *eList;
}

%left '+' '-'
%left '*' '/' "div" "mod"
%left "and" "or"
%left "not"

%type<stmt> stmt;
%type<expr> expr lVal rVal call;
%type<blk> stmt_list;
%type<var> unop stringBinop;
%type<c> charBinop;
%type<eList> exprList

%%

program : "program" T_id ';' body '.'   { cout << $2 <<"\n"; }
    ;

body : localList block
    ;
localList :
    | localList local
    ;
local : "var" decList
    | "label" idList ';'
    | header ';' body ';'
    | "forward" header ';'
    ;
idList : T_id
    | idList ',' T_id
    ; 
decList : declaration
    | decList declaration
    ; 
declaration : idList ':' type ';'
    ;

header : "procedure" T_id '(' optFormalList ')'
    | "function" T_id '(' optFormalList ')' ':' type
    ;
optFormalList :
    | formalList
    ;
formalList : formal
    | formalList ';' formal
    ;

formal : optVar idList ':' type
    ;
optVar : 
    | "var"
    ;

type : "integer"
    | "real"
    | "boolean"
    | "char"
    | "array" optLength "of" type
    | "^" type
    ;
optLength :
    | '[' integer_const ']'
    ;

block : "begin" stmt_list "end"
    ;
stmt_list : stmt                        { $$ = new Block(); $$->append($1); }
    | stmt_list ';' stmt                { $1->append($3); $$ = $1; }
    ;

stmt : 
    | lVal ":=" expr
    | block
    | call
    | "if" expr "then" stmt             { $$ = new ITE($2, $4, nullptr); }
    | "if" expr "then" stmt "else" stmt { $$ = new ITE($2, $4, $6); }
    | "while" expr "do" stmt            { $$ = new While($2, $4); }
    | T_id ':' stmt
    | "goto" T_id
    | "return"
    | "new" optObj lVal
    | "dispose" optArr lVal
    ;
optObj :
    | '[' expr ']'
    ;
optArr :
    | '[' ']'
    ;

expr : lVal
    | rVal
    ;
exprList : expr                         { $$ = new ExprList(); $$->append($1); }
    | exprList ',' expr                 { $1->append($3); $$ = $1; }
    ;

lVal : T_id                             { $$ = new Id($1); }
    | "result"
    | string_lit                        { $$ = new StringLit($1); }
    | lVal '[' expr ']'
    | expr '^'
    | '(' lVal ')';
rVal : integer_const                    { $$ = new IntConst($1); }
    | "true"                            { $$ = new BoolConst(true); }
    | "false"                           { $$ = new BoolConst(false); }
    | real_const                        { $$ = new RealConst($1); }
    | char_const                        { $$ = new CharConst($1); }
    | '(' rVal ')'
    | "nil"                             { $$ = new Nil(); }
    | call
    | '@' lVal                          { $$ = new UnOp("@", $2); }
    | unop expr                         { $$ = new UnOp($1, $2); }
    | expr charBinop expr               { $$ = new CharBinOp($1, $2, $3); }
    | expr stringBinop expr             { $$ = new StringBinOp($1, $2, $3); }
    ;

call : T_id '(' exprList ')'    { $$ = new Call($1, $3); }
    ;

unop : "not"
    | '+'
    | '-'
    ;

charBinop : '+'
    | '-'
    | '*'
    | '/'
    | '='
    | '<'
    | '>'
    ;

stringBinop : "div"
    | "mod"
    | "or"
    | "and"
    | "<>"
    | "<="
    | ">="
    ;


%%

void yyerror(const char *msg) {
    printf("Syntax error: %s\n", msg);
    exit(42);
}

int main() {
    int result = yyparse();
    //if (result == 0) printf("Success.\n");
    return result;
}