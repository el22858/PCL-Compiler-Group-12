%{
    #include <cstdio>
    #include <string>
    #include "ast.hpp"
    #include "lexer.hpp"
    using namespace std;

    #define YYERROR_VERBOSE 1
    #define YYDEBUG 1
%}


%define parse.error verbose
%verbose
%define parse.trace


%token T_dispose        "dispose"
%token T_forward        "forward"
%token T_nil            "nil"

%token T_procedure      "procedure"
%token T_begin          "begin"
%token T_end            "end"
%token T_program        "program"
%token T_result         "result"
%token T_return         "return"
%token T_goto           "goto"
%token T_label          "label"

%token T_while          "while"
%token T_do             "do"
    
%token T_if             "if"
%token T_then           "then"
%token T_else           "else"
    
%token T_var            "var"
%token T_function       "function"
%token T_set            ":="

%token T_integer        "integer"
%token T_real           "real"
%token T_array          "array"
%token T_boolean        "boolean"
%token T_char           "char"
%token T_of             "of"
%token T_new            "new"

%token T_and            "and"
%token T_or             "or"
%token T_not            "not"
%token T_mod            "mod"
%token T_div            "div"
%token T_noteq          "<>"
%token T_leq            "<="
%token T_geq            ">="

%token T_left_br        "["
%token T_right_br       "]"
%token T_mem            "@"
%token T_deref          "^"
%token T_op_plus        "+"
%token T_op_minus       "-"
%token T_op_mult        "*" 
%token T_op_div         "/"
%token T_eq             "="
%token T_lt             "<"
%token T_gt             ">"
%token T_semCol ";"
%token T_leftPar "("
%token T_rightPar ")"
%token T_period "."
%token T_col ":"
%token T_comma ","

%token T_true          "true"
%token T_false         "false"

%token<var> T_id
%token<num> integer_const
%token<real> real_const
%token<c> char_const
%token<var> string_lit
%token T_escape

%union {
    Body *body;
    Local *local;
    LocalList *locList;
    Block *blk;
    Header *hdr;
    Label *lbl;
    Id *id;
    IdList *idList;
    Decl *declaration;
    DeclList *decList;
    Formal *form;
    FormalList *formalList;
    Stmt *stmt;
    StmtList *stmt_list;
    Expr *expr;
    ExprList *exprList;
    Call *call;
    CallRVal *rCall;
    Type *type;
    RVal *rVal;
    LVal *lVal;

    int num;
    double real;
    char c;
    char *var;
}

%type<body> body
%type<local> local
%type<locList> localList
%type<blk> block
%type<hdr> header
%type<lbl> label
%type<idList> idList
%type<declaration> declaration
%type<decList> decList
%type<form> formal
%type<formalList> formalList
%type<stmt> stmt
%type<stmt_list> stmt_list
%type<expr> expr
%type<exprList> exprList
%type<call> call
%type<rCall> rCall
%type<type> type
%type<lVal> lVal
%type<rVal> rVal

%left<op> "+" "-" "="
%left<op> "*" "/" "div" "mod" "or"
%left<op> "(" ")" "[" "]" "and" "not" "<=" ">=" "<" ">" "<>"
%right<op> "@" "^"

%expect 1

%%

program : "program" T_id ";" body "."                       { cout << *$4 <<"\n"; }
    ;

body : localList block                                      { $$ = new Body{$1, $2}; }
    ;
localList :                                                 { $$ = new LocalList(); }
    | localList local                                       { $1->append($2); $$ = $1; }
    ;
local : "var" decList                                       { $$ = new Local($2); }
    | "label" label ";"                                     { $$ = new Local($2); }
    | header ";" body ";"                                   { $$ = new Local($1, $3); }
    | "forward" header ";"                                  { $$ = new Local($2); }
    ;
label : T_id idList ";"                                     { $2->append(new Id(ids.back())); $$ = new Label($2); ids.pop_back(); }
    ;
idList :                                                    { $$ = new IdList(); }
    | idList "," T_id                                       { $1->append(new Id(ids.back())); $$ = $1; ids.pop_back(); }
    ; 
decList : declaration                                       { $$ = new DeclList(); $$->append($1); }
    | decList declaration                                   { $1->append($2); $$ = $1; }
    ; 
declaration : T_id idList ":" type ";"                      { $2->append(new Id(ids.back())); $$ = new Decl($2, $4); ids.pop_back(); }
    ;

header : "procedure" T_id "(" formal formalList ")"         { $5->append($4); $$ = new Procedure(new Id(ids.back()), $5); ids.pop_back(); }
    | "procedure" T_id "(" ")"                               { $$ = new Procedure(new Id(ids.back())); ids.pop_back(); }
    | "function" T_id "(" formal formalList ")" ":" type    { $5->append($4); $$ = new Function(new Id(ids.back()), $8, $5); ids.pop_back(); }
    | "function" T_id "(" ")" ":" type                      { $$ = new Function(new Id(ids.back()), $6); ids.pop_back(); }
    ;

formalList :                                                { $$ = new FormalList(); }
    | formalList ";" formal                                 { $1->append($3); $$ = $1; }
    ;

formal : "var" T_id idList ":" type                         { $3->append(new Id(ids.back())); $$ = new Formal($3, $5); ids.pop_back(); }
    | T_id idList ":" type                                  { $2->append(new Id(ids.back())); $$ = new Formal($2, $4); ids.pop_back(); }
    ;

type : "integer"                                            { $$ = new Integer(); }
    | "real"                                                { $$ = new Real(); }
    | "boolean"                                             { $$ = new Boolean(); }
    | "char"                                                { $$ = new Char(); }
    | "array" "[" integer_const "]" "of" type               { $$ = new Array($6, $3); }
    | "array" "of" type                                     { $$ = new Array($3); }
    | "^" type                                              { $$ = new Pointer($2); }
    ;

block :
    "begin" stmt stmt_list "end"                            { $3->append($2); $$ = new Block($3); }
    ;
stmt_list :                                                 { $$ = new StmtList(); }
    | stmt_list ";" stmt                                    { $1->append($3); $$ = $1; }
    ;

stmt :                                                      { $$ = nullptr; }
    | lVal ":=" expr                                        { $$ = new Assign($1, $3); }
    | expr "^" ":=" expr                                    { $$ = new Assign($1, $4); }
    | block                                                 { $$ = $1; }
    | call                                                  { $$ = $1; }
    | "if" expr "then" stmt                                 { $$ = new ITE($2, $4); }
    | "if" expr "then" stmt "else" stmt                     { $$ = new ITE($2, $4, $6); }
    | "while" expr "do" stmt                                { $$ = new While($2, $4); }
    | T_id ":" stmt                                         { $$ = new IdLabel(new Id(ids.back()), $3); ids.pop_back(); }
    | "goto" T_id                                           { $$ = new Goto(new Id(ids.back())); ids.pop_back(); }
    | "return"                                              { $$ = new Return(); }
    | "new" "[" expr "]" lVal                               { $$ = new New($5, $3); }
    | "new" "[" expr "]" expr "^"                           { $$ = new New($5, $3); }
    | "new" lVal                                            { $$ = new New($2); }
    | "new" expr "^"                                        { $$ = new New($2); }
    | "dispose" "[" "]" lVal                                { $$ = new Dispose($4); }
    | "dispose" "[" "]" expr "^"                            { $$ = new Dispose($4); }
    | "dispose" lVal                                        { $$ = new Dispose($2); }
    | "dispose" expr "^"                                    { $$ = new Dispose($2); }
    ;

expr : lVal                                                 { $$ = $1; }
    | rVal                                                  { $$ = $1; }
    ;
exprList :                                                  { $$ = new ExprList(); }
    | exprList "," expr                                     { $1->append($3); $$ = $1; }
    ;

lVal : T_id                                                 { $$ = new Id(ids.back()); ids.pop_back(); }
    | "result"                                              { $$ = new Result(); }
    | string_lit                                            { $$ = new StringLit(stringLits.back()); stringLits.pop_back(); }
    | lVal "[" expr "]"                                     { $$ = new ArrayItem($1, $3); }
    | "(" lVal ")"                                          { $$ = $2; }
    ;
rVal : integer_const                                        { $$ = new IntConst(constInts.back()); constInts.pop_back(); }
    | "true"                                                { $$ = new BoolConst(true); }
    | "false"                                               { $$ = new BoolConst(false); }
    | real_const                                            { $$ = new RealConst(constReals.back()); constReals.pop_back(); }
    | char_const                                            { $$ = new CharConst(constChars.back()); constChars.pop_back(); }
    | "(" rVal ")"                                          { $$ = $2; }
    | "nil"                                                 { $$ = new Nil(); }
    | rCall                                                 { $$ = $1; }
    | "@" lVal                                              { $$ = new Reference($2); }
    | "not" expr                                            { $$ = new UnOp(operators.back(), $2); operators.pop_back(); }
    | "+" expr                                              { $$ = new UnOp(operators.back(), $2); operators.pop_back(); }
    | "-" expr                                              { $$ = new UnOp(operators.back(), $2); operators.pop_back(); }
    | expr "+" expr                                         { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "-" expr                                         { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "*" expr                                         { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "/" expr                                         { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "=" expr                                         { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "<" expr                                         { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr ">" expr                                         { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "div" expr                                       { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "mod" expr                                       { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "or" expr                                        { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "<>" expr                                        { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr "<=" expr                                        { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    | expr ">=" expr                                        { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
    ;

call : T_id "(" expr exprList ")"                                { $4->append($3); $$ = new Call(new Id(ids.back()), $4); ids.pop_back(); }
    | T_id "(" ")"                                          { $$ = new Call(new Id(ids.back())); ids.pop_back(); }
    ;

rCall : T_id "(" expr exprList ")"                                { $4->append($3); $$ = new CallRVal(new Id(ids.back()), $4); ids.pop_back(); }
    | T_id "(" ")"                                          { $$ = new CallRVal(new Id(ids.back())); ids.pop_back(); }
    ;


%%

void yyerror(const char *msg) {
    //printf("Syntax error: %s\n", msg);
    fprintf(stderr, "%s\n", msg);
    exit(42);
}

int main() {
    int result = yyparse();
    //if (result == 0) printf("Success.\n");
    return result;
}