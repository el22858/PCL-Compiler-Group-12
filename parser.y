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
    #include <fstream>

    extern FILE *yyin;
%}

%code requires {
    #include "ast.hpp"
}

%{
    #include "lexer.hpp"
    YY_DECL;

    #include "basic.hpp"
    #include "symbol.hpp"

    std::string name;
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
%token<char *> char_const
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

%nonassoc<char *> "=" ">" "<" ">=" "<=" "<>"
%left<char *> "+" "-" "or"
%left<char *> "*" "/" "div" "mod" "and"
%nonassoc<char *> "not"
%nonassoc<char *> "^"
%nonassoc<char *> "@"
%nonassoc<char *> "[" "]" "(" ")"


%%

program : "program" T_id ";" body "." {
        /* std::cout << *$4 <<"\n"; */
        ast = $4;
        ast->setName($2);
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
label : T_id idList                                         { auto x = $2; x->appendAtStart(std::make_unique<Id>(ids.back())); ids.pop_back(); $$ = std::make_unique<Label>(std::move(x)); /* $2->append(std::make_unique<Id>(ids.back())); $$ = new Label($2); */ }
    ;
idList :                                                    { $$ = std::make_unique<IdList>(); }
    | idList "," T_id                                       { $$ = $1; $$->append(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    ; 
decList : declaration                                       { $$ = std::make_unique<DeclList>(); $$->append($1); }
    | decList declaration                                   { $$ = $1; $$->append($2); }
    ; 
declaration : T_id idList ":" type ";"                      {  auto x = $2; x->appendAtStart(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Decl>(std::move(x), $4); /* $2->append(std::make_unique<Id>(ids.back())); $$ = new Decl($2, $4); */ ids.pop_back(); }
    ;

header : "procedure" T_id "(" formal formalList ")"         { auto x = $5; x->appendAtStart($4); $$ = std::make_unique<Procedure>(std::make_unique<Id>(ids.back()), std::move(x)); /* $5->append($4); $$ = std::make_unique<Procedure>(std::make_unique<Id>(ids.back()), $5); */ ids.pop_back(); }
    | "procedure" T_id "(" ")"                              { std::cout << ids.back() << std::endl; $$ = std::make_unique<Procedure>(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    | "function" T_id "(" formal formalList ")" ":" type    { auto x = $5; x->appendAtStart($4); $$ = std::make_unique<Function>(std::make_unique<Id>(ids.back()), $8, std::move(x)); /* $5->append($4); $$ = std::make_unique<Function>(std::make_unique<Id>(ids.back()), $8, $5); */ ids.pop_back(); }
    | "function" T_id "(" ")" ":" type                      { $$ = std::make_unique<Function>(std::make_unique<Id>(ids.back()), $6); ids.pop_back(); }
    ;

formalList :                                                { $$ = std::make_unique<FormalList>(); }
    | formalList ";" formal                                 { $$ = $1; $$->append($3); }
    ;

formal : "var" T_id idList ":" type                         { auto x = $3; x->appendAtStart(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Formal>(std::move(x), $5); /* $3->append(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Formal>($3, $5); */ ids.pop_back(); }
    | T_id idList ":" type                                  { auto x = $2; x->appendAtStart(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Formal>(std::move(x), $4); /* $2->append(std::make_unique<Id>(ids.back())); $$ = std::make_unique<Formal>($2, $4); */ ids.pop_back(); }
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
    "begin" stmt stmt_list "end"                            { auto x = $3; x->appendAtStart($2); $$ = std::make_unique<Block>(std::move(x));  /* $3->append($2); $$ = std::make_unique<Block>($3); */ }
    ;
stmt_list :                                                 { $$ = std::make_unique<StmtList>(); }
    | stmt_list ";" stmt                                    { $$ = $1; $$->append($3); }
    ;

stmt :                                                      { $$ = nullptr; }
    | lVal ":=" expr                                        { $$ = std::make_unique<Assign>($1, $3); }
    | block                                                 { $$ = $1; }
    | call                                                  { $$ = $1; }
    | "if" expr "then" stmt                                 { $$ = std::make_unique<ITE>($2, $4); }
    | "if" expr "then" stmt "else" stmt                     { $$ = std::make_unique<ITE>($2, $4, $6); }
    | "while" expr "do" stmt                                { $$ = std::make_unique<While>($2, $4); }
    | T_id ":" stmt                                         { $$ = std::make_unique<IdLabel>(std::make_unique<Id>(ids.back()), $3); ids.pop_back(); }
    | "goto" T_id                                           { $$ = std::make_unique<Goto>(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    | "return"                                              { $$ = std::make_unique<Return>(); }
    | "new" "[" expr "]" lVal                               { $$ = std::make_unique<New>($5, $3); }
    | "new" lVal                                            { $$ = std::make_unique<New>($2); }
    | "dispose" "[" "]" lVal                                { $$ = std::make_unique<Dispose>($4, true); }
    | "dispose" lVal                                        { $$ = std::make_unique<Dispose>($2, false); }
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
    | expr "^"                                              { $$ = std::make_unique<Deref>($1); }
    ;
rVal : integer_const                                        { $$ = std::make_unique<IntConst>(constInts.back()); constInts.pop_back(); }
    | "true"                                                { $$ = std::make_unique<BoolConst>(true); }
    | "false"                                               { $$ = std::make_unique<BoolConst>(false); }
    | real_const                                            { $$ = std::make_unique<RealConst>(constReals.back()); constReals.pop_back(); }
    | char_const                                            { $$ = std::make_unique<CharConst>(constChars.back()); constChars.pop_back(); }
    | "(" rVal ")"                                          { $$ = $2; }
    | "nil"                                                 { $$ = std::make_unique<NilRVal>(); }
    | rCall                                                 { $$ = $1; }
    | "@" lVal                                              { $$ = std::make_unique<Reference>($2); }
    | "not" expr                                            { $$ = std::make_unique<UnOp>(operators.back(), $2); operators.pop_back(); }
    | "+" expr                                              { $$ = std::make_unique<UnOp>(operators.back(), $2); operators.pop_back(); }
    | "-" expr                                              { $$ = std::make_unique<UnOp>(operators.back(), $2); operators.pop_back(); }
    | expr "+" expr                                         { $$ = std::make_unique<BinOp>($1, "+", $3); operators.pop_back(); }
    | expr "-" expr                                         { $$ = std::make_unique<BinOp>($1, "-", $3); operators.pop_back(); }
    | expr "*" expr                                         { $$ = std::make_unique<BinOp>($1, "*", $3); operators.pop_back(); }
    | expr "/" expr                                         { $$ = std::make_unique<BinOp>($1, "/", $3); operators.pop_back(); }
    | expr "=" expr                                         { $$ = std::make_unique<BinOp>($1, "=", $3); operators.pop_back(); }
    | expr "<" expr                                         { $$ = std::make_unique<BinOp>($1, "<", $3); operators.pop_back(); }
    | expr ">" expr                                         { $$ = std::make_unique<BinOp>($1, ">", $3); operators.pop_back(); }
    | expr "div" expr                                       { $$ = std::make_unique<BinOp>($1, "div", $3); operators.pop_back(); }
    | expr "mod" expr                                       { $$ = std::make_unique<BinOp>($1, "mod", $3); operators.pop_back(); }
    | expr "or" expr                                        { $$ = std::make_unique<BinOp>($1, "or", $3); operators.pop_back(); }
    | expr "and" expr                                        { $$ = std::make_unique<BinOp>($1, "and", $3); operators.pop_back(); }
    | expr "<>" expr                                        { $$ = std::make_unique<BinOp>($1, "<>", $3); operators.pop_back(); }
    | expr "<=" expr                                        { $$ = std::make_unique<BinOp>($1, "<=", $3); operators.pop_back(); }
    | expr ">=" expr                                        { $$ = std::make_unique<BinOp>($1, ">=", $3); operators.pop_back(); }
    ;

call : T_id "(" expr exprList ")"                           { auto x = $4; x->appendAtStart($3); $$ = std::make_unique<Call>(std::make_unique<Id>(ids.back()), std::move(x)); /* $4->append($3); $$ = std::make_unique<Call>(std::make_unique<Id>(ids.back()), $4); */ ids.pop_back(); }
    | T_id "(" ")"                                          { $$ = std::make_unique<Call>(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    ;

rCall : T_id "(" expr exprList ")"                          { auto x = $4; x->appendAtStart($3); $$ = std::make_unique<CallRVal>(std::make_unique<Id>(ids.back()), std::move(x)); /* $4->append($3); $$ = std::make_unique<Call>(std::make_unique<Id>(ids.back()), $4); */ ids.pop_back(); }
    | T_id "(" ")"                                          { $$ = std::make_unique<CallRVal>(std::make_unique<Id>(ids.back())); ids.pop_back(); }
    ;


%%

void yyerror(std::string msg) {
    //printf("Syntax error: %s\n", msg);
    //fprintf(stderr, "%s\n", msg);
    std::cout<<msg<<std::endl;
    exit(42);
}

void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << "\033[91mError: " << m << " at line "
            << l.begin.line << "\033[0m" << std::endl;
  std::exit(1);
}

void initLibs() {
    std::unique_ptr<Id> id;
    std::unique_ptr<IdList> iL;
    std::unique_ptr<Formal> f;
    std::unique_ptr<FormalList> fL;

    // procedurewriteBoolean (b:boolean)
    id = std::make_unique<Id>("b");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Boolean>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("writeBoolean", std::make_unique<TypeProc>(), std::move(fL));

    // procedurewriteString (vars:array ofchar)
    id = std::make_unique<Id>("s");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Array>(std::make_unique<Char>()));
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("writeString", std::make_unique<TypeProc>(), std::move(fL));


    // function readInteger ():integer
    st.insertFormal("readInteger", std::make_unique<Integer>(), std::make_unique<FormalList>());

    // function readBoolean ():boolean
    st.insertFormal("readBoolean", std::make_unique<Boolean>(), std::make_unique<FormalList>());

    // function readChar ():char
    st.insertFormal("readChar", std::make_unique<Char>(), std::make_unique<FormalList>());

    // function readReal ():real
    st.insertFormal("readReal", std::make_unique<Real>(), std::make_unique<FormalList>());

    // procedure readString (size:integer;var s :arrayof char)
    id = std::make_unique<Id>("size");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));

    id = std::make_unique<Id>("s");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Array>(std::make_unique<Char>()));
    fL->append(std::move(f));

    st.insertFormal("readString", std::make_unique<TypeProc>(), std::move(fL));


    // function abs (n:integer):integer
    id = std::make_unique<Id>("n");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));

    st.insertFormal("abs", std::make_unique<Integer>(), std::move(fL));

    // procedurewriteReal (r:real)
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("writeReal", std::make_unique<TypeProc>(), std::move(fL));

    // function fabs (r:real) :real
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("fabs", std::make_unique<Real>(), std::move(fL));

    // function sqrt (r:real):real
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("sqrt", std::make_unique<Real>(), std::move(fL));

    // function sin (r : real):real
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("sin", std::make_unique<Real>(), std::move(fL));

    // function cos (r : real):real
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("cos", std::make_unique<Real>(), std::move(fL));

    // function tan (r : real):real
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("tan", std::make_unique<Real>(), std::move(fL));

    // function arctan (r:real):real
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("arctan", std::make_unique<Real>(), std::move(fL));

    // function exp (r : real):real
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("exp", std::make_unique<Real>(), std::move(fL));

    // function ln (r:real):real
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("ln", std::make_unique<Real>(), std::move(fL));

    // function pi () :real
    st.insertFormal("pi", std::make_unique<Real>(), std::make_unique<FormalList>());


    // function trunc(r : real):integer
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("trunc", std::make_unique<Integer>(), std::move(fL));

    // function round(r : real):integer
    id = std::make_unique<Id>("r");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("round", std::make_unique<Integer>(), std::move(fL));


    // procedurewriteChar (c:char)
    id = std::make_unique<Id>("c");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Char>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("writeChar", std::make_unique<TypeProc>(), std::move(fL));


    // function ord (c : char): integer
    id = std::make_unique<Id>("c");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Char>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("ord", std::make_unique<Integer>(), std::move(fL));


    // procedurewriteInteger (n:integer)
    id = std::make_unique<Id>("n");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("writeInteger", std::make_unique<TypeProc>(), std::move(fL));
    
    // function chr (n : integer) : char
    id = std::make_unique<Id>("n");
    iL = std::make_unique<IdList>();
    iL->append(std::move(id));
    f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
    fL = std::make_unique<FormalList>();
    fL->append(std::move(f));
    st.insertFormal("chr", std::make_unique<Char>(), std::move(fL));

}



std::vector<quad> finalQuadList;
int quadNextTemp;

quad quadGENQUAD(std::string op, std::string x, std::string y, std::string z) { return quad(op, x, y, z); }

int quadNEWTEMP() { return quadNextTemp++; }

std::vector<int> quadMAKELIST(int x) {
    std::vector<int> list;
    list.push_back(x);
    return list;
}

std::vector<quad> quadEMPTYLIST() {
    std::vector<quad> list;
    return list;
}

//std::vector<quad> quadMAKELIST(quad x) {
//    std::vector<quad> list;
//    list.push_back(x);
//    return list;
//}

std::vector<quad> quadMERGELISTS(std::vector<quad> l1, std::vector<quad> l2) {
    std::vector<quad> l;

    if (l1.size() >= l2.size()) {
        l = l1;
        l.insert(l.end(), l2.begin(), l2.end());
    } else {
        l = l2;
        l.insert(l.begin(), l1.begin(), l1.end());
    }

    //std::cout << l << std::endl;
    return l;
}


int main(int argc, char** argv) {
    //yy::parser p;
    //int result = p.parse();
    // cout << ast->getName() << "\n";
    //if (result == 0) printf("Success.\n");
    //return result;

    yyin = fopen(argv[1], "r");
    if (yyin == nullptr) {
      perror(argv[1]);
      return 1;
    }
    yyrestart(yyin);

    yy::parser p;
    p.parse();
    //std::cout << ast->getName() << std::endl;
    //std::cout << *ast << std::endl;

    quadNextTemp = 1;
    finalQuadList;
    st.enterScope();
    st.insertParent(name);
    initLibs();
    ast->sem();
    
    std::string dude = "./" + ast->getBodyName() + ".imm";
    std::ofstream imFile(dude);
    st.exitScope();

    imFile << finalQuadList;
}