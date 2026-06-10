#ifndef _ASS_HPP_
#define _ASS_HPP_

#include <cmath>
#include <algorithm>
#include <vector>
#include "quads.hpp"
#include "ast.hpp"

extern std::unique_ptr<Body> ast;
extern std::string assembly;
extern std::vector<std::string> stringsUsed, libsUsed;


void prologue() {
	// assembly = "xseg\tsegment public 'code'\n\t\tassume cs:xseg, ds:xseg, ss:xseg\n\t\torg 100h\n";
	// assembly += "main\tproc near\n\t\tcall near ptr _" + ast->getBodyName() + "_1\n\t\tmov ax, 4C00h\n\t\tint 21h\nmain\tendp\n"; 

	assembly = "\t\tglobal main\n";

	for (const auto &x: libsUsed) assembly += "\t\textern _" + x + "\n";
	assembly += "\n\t\tsection .text\nmain:\n";
}
void epilogue() {
	// for (const auto &l : libsUsed) assembly += "\t\textrn _" + l + "\n";
	// for (long unsigned int i = 1; i <= stringsUsed.size(); ++i) assembly += "@str_" + std::to_string(i) + "\tdb '" + stringsUsed[i] + "'\n";
	// assembly += "xseg\tends\n\t\tend main";
	for (unsigned long int i = 0; i < stringsUsed.size(); ++i) {
		assembly += "str" + std::to_string(i+1) +":\tdb\t\"" + stringsUsed[i] + "\", 0\n";
	}
}

void translate(std::vector<quad> fQL) {
	for (const auto &q : fQL) {
		std::string op = q.getOpname();
		if (op.compare("") == 0);
		else std::cout << "Op " << op << " not yet supported." << std::endl;
	}
}

void load(std::string s) {
    // if (is_number(s)) assembly += "\t\tmov R, " + s + "\n";
    // else if (s.compare("true") == 0) assembly += "\t\tmov R, 1\n";
    // else if (s.compare("false") == 0) assembly += "\t\tmov R, 0\n";
    //FIXME: a lot of shit missing, probably wrong commands too
}

#endif