%require "3.8"
%language "C++"
%locations
%output "parser.cpp"
%defines "parser.hpp"
%verbose
%define api.location.file "location.hpp"

%{
    #include <cstdio>
    #include <memory>
    #include <string>
%}

%code requires {
    #include "ast.hpp"
}

%{
    #include "lexer.hpp"
    YY_DECL;

    #include "basic.hpp"
    #include "symbol.hpp"

    static std::unique_ptr<Body> ast;
    SymbolTable st;
%}


%define api.value.type variant
%define parse.error verbose
%define api.token.constructor
%define parse.trace
%define api.value.automove
%define parse.assert


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

%token<char *> T_id
%token<int> integer_const
%token<double> real_const
%token<char> char_const
%token<char *> string_lit
%token T_escape


%type<std::unique_ptr<Body>> body
%type<std::unique_ptr<Local>> local
%type<std::unique_ptr<LocalList>> localList
%type<std::unique_ptr<Block>> block
%type<std::unique_ptr<Header>> header
%type<std::unique_ptr<Label>> label
%type<std::unique_ptr<IdList>> idList
%type<std::unique_ptr<Decl>> declaration
%type<std::unique_ptr<DeclList>> decList
%type<std::unique_ptr<Formal>> formal
%type<std::unique_ptr<FormalList>> formalList
%type<std::unique_ptr<Stmt>> stmt
%type<std::unique_ptr<StmtList>> stmt_list
%type<std::unique_ptr<Expr>> expr
%type<std::unique_ptr<ExprList>> exprList
%type<std::unique_ptr<Call>> call
%type<std::unique_ptr<CallRVal>> rCall
%type<std::unique_ptr<Type>> type
%type<std::unique_ptr<LVal>> lVal
%type<std::unique_ptr<RVal>> rVal

%left<char *> "+" "-" "="
%left<char *> "*" "/" "div" "mod" "or"
%left<char *> "(" ")" "[" "]" "and" "not" "<=" ">=" "<" ">" "<>"
%right<char *> "@" "^"

%expect 1

%%

program : "program" T_id ";" body "." {
        /* std::cout << *$4 <<"\n"; */
        ast = $4;
        /* st.enterScope();
        $4->sem();
        st.exitScope(); */
    }
    ;

body : localList block                                      { $$ = std::make_unique<Body>($1, $2); }
    ;
localList :                                                 { $$ = std::make_unique<LocalList>(); }
    | localList local                                       { $$ = $1; $$->append($2); }
    ;
local : "var" decList                                       { $$ = std::make_unique<Local>($2); }
    | "label" label ";"                                     { $$ = std::make_unique<Local>($2); }
    | header ";" body ";"                                   { $$ = std::make_unique<Local>($1, $3); }
    | "forward" header ";"                                  { $$ = std::make_unique<Local>($2); }
    ;
label : T_id idList ";"                                     { auto x = $2; x->append(std::make_unique<Id>(ids.back())); ids.pop_back(); $$ = std::make_unique<Label>(std::move(x)); /* $2->append(std::make_unique<Id>(ids.back())); $$ = new Label($2); */ }
    ;
idList :                                                    { $$ = std::make_unique<IdList>(); }
    | idList "," T_id                                       { $$ = $1; $$->append(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    ; 
decList : declaration                                       { $$ = std::make_unique<DeclList>(); $$->append($1); }
    | decList declaration                                   { $$ = $1; $$->append($2); }
    ; 
declaration : T_id idList ":" type ";"                      {  auto x = $2; x->append(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Decl>(std::move(x), $4); /* $2->append(std::make_unique<Id>(ids.back())); $$ = new Decl($2, $4); */ ids.pop_back(); }
    ;

header : "procedure" T_id "(" formal formalList ")"         { auto x = $5; x->append($4); $$ = std::make_unique<Procedure>(std::make_unique<Id>(ids.back()), std::move(x)); /* $5->append($4); $$ = std::make_unique<Procedure>(std::make_unique<Id>(ids.back()), $5); */ ids.pop_back(); }
    | "procedure" T_id "(" ")"                              { $$ = std::make_unique<Procedure>(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    | "function" T_id "(" formal formalList ")" ":" type    { auto x = $5; x->append($4); $$ = std::make_unique<Function>(std::make_unique<Id>(ids.back()), $8, std::move(x)); /* $5->append($4); $$ = std::make_unique<Function>(std::make_unique<Id>(ids.back()), $8, $5); */ ids.pop_back(); }
    | "function" T_id "(" ")" ":" type                      { $$ = std::make_unique<Function>(std::make_unique<Id>(ids.back()), $6); ids.pop_back(); }
    ;

formalList :                                                { $$ = std::make_unique<FormalList>(); }
    | formalList ";" formal                                 { $$ = $1; $$->append($3); }
    ;

formal : "var" T_id idList ":" type                         { auto x = $3; x->append(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Formal>(std::move(x), $5); /* $3->append(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Formal>($3, $5); */ ids.pop_back(); }
    | T_id idList ":" type                                  { auto x = $2; x->append(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Formal>(std::move(x), $4); /* $2->append(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Formal>($2, $4); */ ids.pop_back(); }
    ;

type : "integer"                                            { $$ = std::make_unique<Integer>(); }
    | "real"                                                { $$ = std::make_unique<Real>(); }
    | "boolean"                                             { $$ = std::make_unique<Boolean>(); }
    | "char"                                                { $$ = std::make_unique<Char>(); }
    | "array" "[" integer_const "]" "of" type               { $$ = std::make_unique<Array>($6, $3); }
    | "array" "of" type                                     { $$ = std::make_unique<Array>($3); }
    | "^" type                                              { $$ = std::make_unique<Pointer>($2); }
    ;

block :
    "begin" stmt stmt_list "end"                            { auto x = $3; x->append($2); $$ = std::make_unique<Block>(std::move(x));  /* $3->append($2); $$ = std::make_unique<Block>($3); */ }
    ;
stmt_list :                                                 { $$ = std::make_unique<StmtList>(); }
    | stmt_list ";" stmt                                    { $$ = $1; $$->append($3); }
    ;

stmt :                                                      { $$ = nullptr; }
    | lVal ":=" expr                                        { $$ = std::make_unique<Assign>($1, $3); }
    | expr "^" ":=" expr                                    { $$ = std::make_unique<Assign>($1, $4); }
    | block                                                 { $$ = $1; }
    | call                                                  { $$ = $1; }
    | "if" expr "then" stmt                                 { $$ = std::make_unique<ITE>($2, $4); }
    | "if" expr "then" stmt "else" stmt                     { $$ = std::make_unique<ITE>($2, $4, $6); }
    | "while" expr "do" stmt                                { $$ = std::make_unique<While>($2, $4); }
    | T_id ":" stmt                                         { $$ = std::make_unique<IdLabel>(std::make_unique<Id>(ids.back()), $3); ids.pop_back(); }
    | "goto" T_id                                           { $$ = std::make_unique<Goto>(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    | "return"                                              { $$ = std::make_unique<Return>(); }
    | "new" "[" expr "]" lVal                               { $$ = std::make_unique<New>($5, $3); }
    | "new" "[" expr "]" expr "^"                           { $$ = std::make_unique<New>($5, $3); }
    | "new" lVal                                            { $$ = std::make_unique<New>($2); }
    | "new" expr "^"                                        { $$ = std::make_unique<New>($2); }
    | "dispose" "[" "]" lVal                                { $$ = std::make_unique<Dispose>($4); }
    | "dispose" "[" "]" expr "^"                            { $$ = std::make_unique<Dispose>($4); }
    | "dispose" lVal                                        { $$ = std::make_unique<Dispose>($2); }
    | "dispose" expr "^"                                    { $$ = std::make_unique<Dispose>($2); }
    ;

expr : lVal                                                 { $$ = $1; }
    | rVal                                                  { $$ = $1; }
    ;
exprList :                                                  { $$ = std::make_unique<ExprList>(); }
    | exprList "," expr                                     { $$ = $1; $$->append($3); }
    ;

lVal : T_id                                                 { $$ = std::make_unique<Id>(ids.back()); ids.pop_back(); }
    | "result"                                              { $$ = std::make_unique<Result>(); }
    | string_lit                                            { $$ = std::make_unique<StringLit>(stringLits.back()); stringLits.pop_back(); }
    | lVal "[" expr "]"                                     { $$ = std::make_unique<ArrayItem>($1, $3); }
    | "(" lVal ")"                                          { $$ = $2; }
    ;
rVal : integer_const                                        { $$ = std::make_unique<IntConst>(constInts.back()); constInts.pop_back(); }
    | "true"                                                { $$ = std::make_unique<BoolConst>(true); }
    | "false"                                               { $$ = std::make_unique<BoolConst>(false); }
    | real_const                                            { $$ = std::make_unique<RealConst>(constReals.back()); constReals.pop_back(); }
    | char_const                                            { $$ = std::make_unique<CharConst>(constChars.back()); constChars.pop_back(); }
    | "(" rVal ")"                                          { $$ = $2; }
    | "nil"                                                 { $$ = std::make_unique<Nil>(); }
    | rCall                                                 { $$ = $1; }
    | "@" lVal                                              { $$ = std::make_unique<Reference>($2); }
    | "not" expr                                            { $$ = std::make_unique<UnOp>(operators.back(), $2); operators.pop_back(); }
    | "+" expr                                              { $$ = std::make_unique<UnOp>(operators.back(), $2); operators.pop_back(); }
    | "-" expr                                              { $$ = std::make_unique<UnOp>(operators.back(), $2); operators.pop_back(); }
    | expr "+" expr                                         { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "-" expr                                         { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "*" expr                                         { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "/" expr                                         { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "=" expr                                         { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "<" expr                                         { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr ">" expr                                         { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "div" expr                                       { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "mod" expr                                       { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "or" expr                                        { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "<>" expr                                        { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr "<=" expr                                        { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    | expr ">=" expr                                        { $$ = std::make_unique<BinOp>($1, operators.back(), $3); operators.pop_back(); }
    ;

call : T_id "(" expr exprList ")"                           { auto x = $4; x->append($3); $$ = std::make_unique<Call>(std::make_unique<Id>(ids.back()), std::move(x)); /* $4->append($3); $$ = std::make_unique<Call>(std::make_unique<Id>(ids.back()), $4); */ ids.pop_back(); }
    | T_id "(" ")"                                          { $$ = std::make_unique<Call>(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    ;

rCall : T_id "(" expr exprList ")"                          { auto x = $4; x->append($3); $$ = std::make_unique<CallRVal>(std::make_unique<Id>(ids.back()), std::move(x)); /* $4->append($3); $$ = std::make_unique<Call>(std::make_unique<Id>(ids.back()), $4); */ ids.pop_back(); }
    | T_id "(" ")"                                          { $$ = std::make_unique<CallRVal>(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    ;


%%

void yyerror(const char *msg) {
    //printf("Syntax error: %s\n", msg);
    fprintf(stderr, "%s\n", msg);
    exit(42);
}

void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << "\033[91mError: " << m << " at line "
            << l.begin.line << "\033[0m" << std::endl;
  std::exit(1);
}

int main() {
    yy::parser p;
    int result = p.parse();
    cout << *ast << "\n";
    //if (result == 0) printf("Success.\n");
    return result;
}