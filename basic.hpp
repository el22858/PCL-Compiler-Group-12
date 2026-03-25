#ifndef __BASIC_HPP__
#define __BASIC_HPP__
//Full disclosure, I don't think this is needed, but it was in the example

#include <vector>
using namespace std;

extern vector<char *> ids, stringLits, operators;
extern vector<char> constChars;
extern vector<int> constInts;
extern vector<double> constReals;

void yyerror(const char *msg);

#endif