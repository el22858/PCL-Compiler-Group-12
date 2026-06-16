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


inline void prologue(std::string mainName) {
	// assembly = "xseg\tsegment public 'code'\n\t\tassume cs:xseg, ds:xseg, ss:xseg\n\t\torg 100h\n";
	// assembly += "main\tproc near\n\t\tcall near ptr _" + ast->getBodyName() + "_1\n\t\tmov ax, 4C00h\n\t\tint 21h\nmain\tendp\n"; 

	assembly = "\t\tglobal main\n";

	for (const auto &x: libsUsed) assembly += "\t\textern _" + x + "\n";
	assembly += "\n\t\tsection .text\nmain:\n\t\tcall _" + mainName + "\n\t\tret";
}
inline void epilogue() {
	// for (const auto &l : libsUsed) assembly += "\t\textrn _" + l + "\n";
	// for (long unsigned int i = 1; i <= stringsUsed.size(); ++i) assembly += "@str_" + std::to_string(i) + "\tdb '" + stringsUsed[i] + "'\n";
	// assembly += "xseg\tends\n\t\tend main";
	for (unsigned long int i = 0; i < stringsUsed.size(); ++i) {
		assembly += "str" + std::to_string(i+1) +":\tdb\t\"" + stringsUsed[i] + "\", 0\n";
	}
}

inline void load(std::string R, std::string a) {
    if (is_number(a)) assembly += "\t\tmov\t\t" + R + ", " + a + "\n";
    else if (a.compare("true") == 0) assembly += "\t\tmov\t\t" + R + ", 1\n";
    else if (a.compare("false") == 0) assembly += "\t\tmov\t\t" + R + ", 0\n";
	// ??? if a is char
	else if (a.compare("nil") == 0) assembly += "\t\tmov\t\t" + R + ", 0\n";
    //FIXME: a lot of shit missing, probably wrong commands too
}

inline void loadReal(std::string a) {}

inline void loadAddr(std::string R, std::string a) {}

inline void store(std::string R, std::string a) {}

inline void storeReal(std::string a) {}

inline std::string label(std::string z) {}

inline std::string name(std::string z) {}

inline void translate(std::vector<quad> fQL) {
	std::string ax = "rax", cx = "rcx", dx = "rdx", st0 = "???", st1 = "???", sp = "rsi", bp = "rbp";

	for (const auto &q : fQL) {
		std::string op = q.getOpname(), x = q.getOp1(), y = q.getOp2(), z = q.getOp3();
		if (op.compare("call") == 0) assembly += "\t\tcall\t_" + z + "\n";
		else if (op.compare("par") == 0) {
			if (y.compare("V") == 0); // ??? FIXME
			else {
				loadAddr(sp, x);
				assembly += "\t\tpush\tsi\n";
			}
		}
		else if (op.compare("unit") == 0) assembly += "_" + q.getOp1() + ":\n";
		else if (op.compare("endu") == 0) assembly += "\t\tret\n";
		else if (op.compare("ret") == 0) assembly += "\t\tjmp\tendof(???)";
		else if (op.compare(":=") == 0) {
			if (q.withReal()) {
				loadReal(x);
				storeReal(z);
			} else {
				load(ax, x);
				store(cx, z);
			}
		} else if (op.compare("array") == 0) {
			load(ax, y);
			assembly += "\t\tmov\t" + cx + ", ???\n";
			assembly += "\t\timul\t" + cx + "\n";
			loadAddr(cx, x);
			assembly += "\t\tadd\t" + ax + ", " + cx + "\n";
			store(ax, z);
		} else if (op.compare("+") == 0) {
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfaddp\t" + st0 + ", " + st1 + "\n";
				storeReal(z);
			} else {
				load(ax, x);
				load(cx, y);
				assembly += "\t\tadd\t" + ax + ", " + cx + "\n";
				store(ax, z);
			}
		} else if (op.compare("-") == 0) {
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfsubp\t" + st0 + ", " + st1 + "\n";
				storeReal(z);
			} else {
				load(ax, x);
				load(cx, y);
				assembly += "\t\tsub\t" + ax + ", " + cx + "\n";
				store(ax, z);
			}
		} else if (op.compare("*") == 0) {
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfmulp\t" + st0 + ", " +  st1 + "\n";
				storeReal(z);
			} else {
				load(ax, x);
				load(cx, y);
				assembly += "\t\timul\t" + ax + ", " + cx + "\n";
				store(ax, z);
			}
		} else if (op.compare("/") == 0) {
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfdivp\t" + st1 + ", " + st0 + "\n";
				storeReal(z);
			} else {
				load(ax, x);
				assembly += "\t\tcwd\n";
				load(cx, y);
				assembly += "\t\tidiv\t" + cx + "\n";
				store(ax, z);
			}
		} else if (op.compare("%") == 0) {
			load(ax, x);
			assembly += "\t\tcwd\n";
			load(cx, y);
			assembly += "\t\tidiv\t" + cx + "\n";
			store(dx, z);
		} else if (op.compare("=") == 0) {
			std::string instr = "???";
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", " + "???" + "\n";
			} else {
				load(ax, x);
				load(dx, y);
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare("<>") == 0) {
			std::string instr = "???";
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", " + "???" + "\n";
			} else {
				load(ax, x);
				load(dx, y);
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare("<") == 0) {
			std::string instr = "???";
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", " + "???" + "\n";
			} else {
				load(ax, x);
				load(dx, y);
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare(">") == 0) {
			std::string instr = "???";
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", " + "???" + "\n";
			} else {
				load(ax, x);
				load(dx, y);
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare("<=") == 0) {
			std::string instr = "???";
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", " + "???" + "\n";
			} else {
				load(ax, x);
				load(dx, y);
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare(">=") == 0) {
			std::string instr = "???";
			if (q.withReal()) {
				loadReal(x);
				loadReal(y);
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", " + "???" + "\n";
			} else {
				load(ax, x);
				load(dx, y);
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare("ifb") == 0) {
			load("al", x);
			assembly += "\t\tor\tal, al\n";
			assembly += "\t\tjnz\t" + label(z);
		} else if (op.compare("jump") == 0) assembly += "\t\tjmp\t" + label(z) + "\n";
		else if (op.compare("jumpl") == 0) assembly += "\t\tjmp\t" + label(z) + "\n";
		else if (op.compare("label") == 0) assembly += label(z) + ":\n";
		else std::cout << "Op " << op << " not yet supported." << std::endl;
	}
}

#endif