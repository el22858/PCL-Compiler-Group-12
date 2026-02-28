%{
    #define T_eof 0

    #define T_and 1
    #define T_array 2
    #define T_begin 3
    #define T_boolean 4
    #define T_char 5
    #define T_dispose 6
    #define T_div 7
    #define T_do 8
    #define T_else 9
    #define T_end 10
    #define T_false 11
    #define T_forward 12
    #define T_function 13
    #define T_goto 14
    #define T_if 15
    #define T_integer 16
    #define T_label 17
    #define T_mod 18
    #define T_new 19
    #define T_nil 20
    #define T_not 21
    #define T_of 22
    #define T_or 23
    #define T_procedure 24
    #define T_program 25
    #define T_real 26
    #define T_result 27
    #define T_return 28
    #define T_then 29
    #define T_true 30
    #define T_var 31
    #define T_while 32

    #define T_const 33
    #define T_id 34
%}

D [0-9]
L [a-z]
W [ \t\n]

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


[\=\+\-\*] { return yytext[0]; }

{D}+ { return T_const; }
{L} { return T_id; }
{W}+ {  }

'.* {  }

. { fprintf(stderr, "Illegal character %c\n", yytext[0]); }

%%

int main(void) {
    int token;
    do {
        token = yylex();
        printf("token = %d, lexeme = %s\n", token, yytext);
    } while (token != T_eof);
}