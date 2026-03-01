#ifndef __LEXER_HPP__
#define __LEXER_HPP__
//Full disclosure, I don't think this is needed, but it was in the example

int yylex();
void yyerror(const char *msg);

#endif