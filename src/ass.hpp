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
extern int n_cur;
static std::string ax = "eax", cx = "ecx", dx = "edx", st0 = "rax", st1 = "rcx", si = "rsi", bp = "rbp", sp = "rsp", al = "al", di = "di";



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

inline void getAR(int n) {
	assembly += "\t\tmov\trsi, word ptr [rbp + 4]";
	for (int i=1; i<n_cur-n; ++i) assembly += "\t\tmov\trsi, word ptr [rsi + 4]";
}

inline void load(std::string R, std::string a, int a_n = 0, int a_off = 0, int a_size = 1, std::string type = "V") {
	std::string size;
	if (a_size == 1) size = "byte";
	else if (a_size == 2) size = "word";
	else if (a_size == 4) size = "dword";
	else if (a_size == 8) size = "qword";

    if (is_number(a)) assembly += "\t\tmov\t\t" + R + ", " + a + "\n";
    else if (a.compare("true") == 0) assembly += "\t\tmov\t\t" + R + ", 1\n";
    else if (a.compare("false") == 0) assembly += "\t\tmov\t\t" + R + ", 0\n";
	else if ((a[0] == '\'') && (a[a.length()-1] == '\'')) {
		if (a[1] != '\\') assembly += "\t\tmov\t\t" + R + ", " + std::to_string((int)a[1]) + "\n";
		else if (a[2] == 'n') assembly += "\t\tmov\t\t" + R + ", " + std::to_string((int)'\n') + "\n";
		else if (a[2] == 't') assembly += "\t\tmov\t\t" + R + ", " + std::to_string((int)'\t') + "\n";
		else if (a[2] == 'r') assembly += "\t\tmov\t\t" + R + ", " + std::to_string((int)'\r') + "\n";
		else if (a[2] == '0') assembly += "\t\tmov\t\t" + R + ", " + std::to_string((int)'\0') + "\n";
		else if (a[2] == '\\') assembly += "\t\tmov\t\t" + R + ", " + std::to_string((int)'\\') + "\n";
		else if (a[2] == '\'') assembly += "\t\tmov\t\t" + R + ", " + std::to_string((int)'\'') + "\n";
		else if (a[2] == '\"') assembly += "\t\tmov\t\t" + R + ", " + std::to_string((int)'\"') + "\n";
	}
	else if (a.compare("nil") == 0) assembly += "\t\tmov\t\t" + R + ", 0\n";
	else if ((a[0] == '[') && (a[a.length()-1] == ']')) {
		load(di, a.substr(1, a.length()-2), a_n, a_off, a_size, type);
		assembly += "\t\tmov\t" + R + ", " + size + " ptr [" + si + "]\n";
	} else if ((a[0] == '{') && (a[a.length()-1] == '}')) loadAddr(R, a);
	else if (a_n == n_cur) {
		if (type.compare("V") == 0) assembly += "\t\tmov\t" + R + ", " + size + " ptr [" + bp + " + " + std::to_string(a_off) + "]\n";
		else {
			assembly += "\t\tmov\t" + si + ", word ptr [" + bp + " + " + std::to_string(a_off) + "]\n";
			assembly += "\t\tmov\t" + R + ", " + size + " ptr [" + si + "]\n";
		}
	} else {
		getAR(a_n);
		if (type.compare("V") == 0) assembly += "\t\tmov\t" + R + ", " + size + " ptr [" + bp + " + " + std::to_string(a_off) + "]\n";
		else {
			assembly += "\t\tmov\t" + si + ", word ptr [" + si + " + " + std::to_string(a_off) + "]\n";
			assembly += "\t\tmov\t" + R + ", " + size + " ptr [" + si + "]\n";
		}
	}
}

inline void loadReal(std::string a, int a_n = 0, int a_off = 0, int a_size = 8, std::string type = "V") {
	std::string size;
	if (a_size == 4) size = "dword";
	else if (a_size == 8) size = "qword";

	if (is_real(a))  assembly += "\t\tfld\t" + a + "\n";
	else if ((a[0] == '[') && (a[a.length()-1] == ']')) {
		load(di, a.substr(1, a.length()-2));
		assembly += "\t\t???\t" + size + " ptr [" + di + "]\n";
	} else if (a_n == n_cur) {
		if (type.compare("V") == 0) assembly += "\t\t???\t" + size + " ptr [" + bp + " + " + std::to_string(a_off) + "]\n";
		else {
			assembly += "\t\tmov\t" + si + ", word ptr [" + bp + " + " + std::to_string(a_off) + "]\n";
			assembly += "\t\t???\t" + size + " ptr [" + si + "]\n";
		}
	} else {
		getAR(a_n);
		if (type.compare("V") == 0) assembly += "\t\t???\t" + size + " ptr [" + si + " + " + std::to_string(a_off) + "]\n";
		else {
			assembly += "\t\tmov\t" + si + ", word ptr [" + si + " + " + std::to_string(a_off) +"]\n";
			assembly += "\t\t???\t" + size + " ptr [" + di + "]\n";
		}
	}
}

inline void loadAddr(std::string R, std::string a) {}

inline void store(std::string R, std::string a) {}

inline void storeReal(std::string a) {}

inline std::string label(std::string z) {
	if (is_number(z)) return "@" + z;
	return "@????";
}

inline std::string endof(std::string x) { return "@???"; }

inline std::string ass_name(std::string z) { return "_" + z + "_???"; }

inline void updateAL(int n) {
	if (n_cur < n) assembly += "\t\tpush\trbp";
	else if (n_cur == n) assembly += "\t\tpush\tword ptr [rbp + 4]";
	else {
		getAR(n);
		assembly += "\t\tpush\tword ptr [rsi + 4]";
	}
}

inline void translate(std::vector<quad> fQL) {

	for (const auto &q : fQL) {
		assembly += label(std::to_string(q.getTag())) + "\n";
		std::string op = q.getOpname(), x = q.getOp1(), y = q.getOp2(), z = q.getOp3();
		if (op.compare("call") == 0) {
			assembly += "\t\tsub\t" + sp + ", 2\n";
			updateAL(q.getZdepth());
			assembly += "\t\tcall\t" + ass_name(z) + "\n";
			assembly += "\t\tadd\t" + sp + ", ???\n";
		} else if (op.compare("par") == 0) {
			if (y.compare("V") == 0) {
				if (q.withReal()) {
					loadReal(x);
					assembly += "\t\tsub" + sp + ", 10\n";
					assembly += "\t\tmov\t" + si + ", " + sp + "\n";
					assembly += "\t\tfstp\ttbyte ptr [" + si + "]\n";
				} else; // ???
			} else {
				loadAddr(sp, x);
				assembly += "\t\tpush\tsi\n";
			}
		}
		else if (op.compare("unit") == 0) {
			n_cur = q.getXdepth();
			assembly += ass_name(x) + "\n";
			assembly += "\t\tpush\t" + bp + "\n";
			assembly += "\t\tmov\t" + bp + ", " + sp + "\n";
			assembly += "\t\tsub\t" + sp + ", ???\n";
		} else if (op.compare("endu") == 0) {
			assembly += endof(x) + ":";
			assembly += "\t\tmov\t" + sp + ", " + bp + "\n";
			assembly += "\t\tpop\t" + bp + "\n";
			assembly += "\t\tret\n";
		} else if (op.compare("ret") == 0) assembly += "\t\tjmp\t" + endof("???") + "\n";
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
			load(al, x);
			assembly += "\t\tor\tal, al\n";
			assembly += "\t\tjnz\t" + label(z);
		} else if (op.compare("jump") == 0) assembly += "\t\tjmp\t" + label(z) + "\n";
		else if (op.compare("jumpl") == 0) assembly += "\t\tjmp\t" + label(z) + "\n";
		else if (op.compare("label") == 0) assembly += label(z) + ":\n";
		else std::cout << "Op " << op << " not yet supported." << std::endl;
	}
}

#endif