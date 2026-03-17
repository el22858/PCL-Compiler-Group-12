#ifndef __LEXER_HPP__
#define __LEXER_HPP__
//Full disclosure, I don't think this is needed, but it was in the example

#include <vector>
using namespace std;

extern vector<char *> ids, stringLits, operators;
extern vector<char> constChars;
extern vector<int> constInts;
extern vector<double> constReals;

int yylex();
void yyerror(const char *msg);

#endif