#ifndef __BASIC_HPP__
#define __BASIC_HPP__
//Full disclosure, I don't think this is needed, but it was in the example

#include <iostream>
#include <vector>

extern std::vector<char *> ids, stringLits, operators;
extern std::vector<char *> constChars;
extern std::vector<int> constInts;
extern std::vector<double> constReals;

void yyerror(std::string msg);

#endif