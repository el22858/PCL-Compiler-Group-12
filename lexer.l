%{
    #define T_eof 0

    #define T_dispose       1001
    #define T_forward       1002
    #define T_nil           1003

    #define T_procedure     1004
    #define T_begin         1005
    #define T_end           1006
    #define T_program       1007
    #define T_result        1008
    #define T_return        1009
    #define T_goto          1010
    #define T_label         1011

    #define T_while         1012
    #define T_do            1013
    
    #define T_if            1014
    #define T_then          1015
    #define T_else          1016
    
    #define T_var           1017
    #define T_function      1018
    #define T_set           1019

    #define T_integer       1020
    #define T_real          1021
    #define T_array         1022
    #define T_boolean       1023
    #define T_char          1024
    #define T_of            1025
    #define T_new           1026

    #define T_and           1027
    #define T_or            1028
    #define T_not           1029
    #define T_mod           1030
    #define T_div           1031
    #define T_noteq         1032
    #define T_leq           1033
    #define T_geq           1034

    #define T_true          1035
    #define T_false         1036

    #define T_id            1037
    #define T_const         1038
    #define T_escape        1039
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

"<>" { return T_noteq; }
">=" { return T_geq; }
"<=" { return T_leq; }
":=" { return T_set; }



{L}({L}|{D}|_)* { return T_id; /* variable or function/procedure name */ }
{D}+(\.{D}+(e[\+\-]?{D}+)?)? { return T_const; /* Any number, int or real */ }

'\\[ntr0\\\'\"]' { return T_escape; /* All escape characters */ }
'.' { return T_const; /* Any character, other than '\n' */ }
\".*\" { return T_const; /* Strings, might have to change this so I can tell it's a char array */ }

{W}+ { /* Whitespace, do nothing */ }

[\=\+\-\*\\/<\^\>\@\[\]\;\.\(\)\:\,] { return yytext[0]; /* Any single character we don't want in ' '  */ }

\(\*.*\*\) { /* This one is for comments */ }

. { fprintf(stderr, "Illegal character %c\n", yytext[0]); }

%%

int main(void) {
    int token;
    do {
        token = yylex();
        printf("token = %d, lexeme = %s\n", token, yytext);
    } while (token != T_eof);
}