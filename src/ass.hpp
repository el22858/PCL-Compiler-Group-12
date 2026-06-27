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
static const std::string ax = "eax", cx = "ecx", dx = "edx", st0 = "rax", st1 = "rcx", si = "rsi", bp = "rbp", sp = "rsp", al = "al", di = "di", rdi = "rdi";
static std::string curFunc;
static int curFNum;



inline void prologue(std::string mainName) {
	// assembly = "xseg\tsegment public 'code'\n\t\tassume cs:xseg, ds:xseg, ss:xseg\n\t\torg 100h\n";
	// assembly += "main\tproc near\n\t\tcall near _" + ast->getBodyName() + "_1\n\t\tmov ax, 4C00h\n\t\tint 21h\nmain\tendp\n"; 

	std::string tmp;
	tmp = "\t\tglobal main\n";

	// for (const auto &x: libsUsed) assembly += "\t\textern " + x + "\n";
	for (unsigned long int i = 0; i < libsUsed.size(); ++i) tmp += "\t\textern _" + libsUsed[i] + "\n";
	tmp += "\n\t\tsection .text\nmain:\n\t\tcall _" + mainName + "_" + std::to_string(curFNum) + "\n\t\tret\n";
	assembly = tmp + assembly;
}

inline void epilogue() {
	// for (const auto &l : libsUsed) assembly += "\t\textrn _" + l + "\n";
	// for (long unsigned int i = 1; i <= stringsUsed.size(); ++i) assembly += "@str_" + std::to_string(i) + "\tdb '" + stringsUsed[i] + "'\n";
	// assembly += "xseg\tends\n\t\tend main";
	for (unsigned long int i = 0; i < stringsUsed.size(); ++i) {
		// assembly += "str" + std::to_string(i+1) +":\tdb\t\"" + stringsUsed[i].substr(1, stringsUsed[i].size()-2) + "\", 0\n";
		assembly += "str" + std::to_string(i+1) + ":\tdb\t\"";
		std::string tmp = stringsUsed[i].substr(1, stringsUsed[i].size()-2);
		for (unsigned long int j = 0; j < tmp.size(); ++j) {
			if (tmp[j] != '\\') assembly += tmp[j];
			else {
				++j;
				assembly += "\", ";
				switch (tmp[j]) {
				case 'n':
					assembly += "10, \"";
					break;
				case 't':
					assembly += "9, \"";
					break;
				case 'r':
					assembly += "13, \"";
					break;
				case '0':
					assembly += "0, \"";
					break;
				case '\\':
					assembly += "92, \"";
					break;
				case '\'':
					assembly += "39, \"";
					break;
				case '\"':
					assembly += "34, \"";
					break;
				default:
					break;
				}
			}
		}
		assembly +="\", 0\n";
	}
}

inline void getAR(int n) {
	assembly += "\t\tmov\t" + si + ", [" + bp + " + 4]\n";
	for (int i=1; i<n_cur-n; ++i) assembly += "\t\tmov\t" + si + ", [" + si + " + 4]\n";
}

inline void loadAddr(std::string R, std::string a, int a_n, int a_off, int a_size, std::string type);

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
		assembly += "\t\tmov\t" + R + ", " + size + " [" + di + "]\n";
	} else if ((a[0] == '{') && (a[a.length()-1] == '}')) loadAddr(R, a.substr(1, a.length()-2), a_n, a_off, a_size, type);
	else if (a_n == n_cur) {
		if (type.compare("V") == 0) {
			assembly += "\t\tmov\t" + R + ", " + size + " [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
		}
		else {
			assembly += "\t\tmov\t" + si + ", [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
			assembly += "\t\tmov\t" + R + ", " + size + " [" + si + "]\n";
		}
	} else {
		getAR(a_n);
		if (type.compare("V") == 0) assembly += "\t\tmov\t" + R + ", " + size + " [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
		else {
			assembly += "\t\tmov\t" + si + ", [" + si + (a_off >= 0 ? " + " : " - ") + std::to_string((abs(a_off))) + "]\n";
			assembly += "\t\tmov\t" + R + ", " + size + " [" + si + "]\n";
		}
	}
}

inline void loadReal(std::string a, int a_n = 0, int a_off = 0, int a_size = 8, std::string type = "V") {
	std::string size, instr;
	if (a_size == 4) {
		size = "dword";
		instr = "fild";
	} else if (a_size == 8) {
		size = "qword";
		instr = "fld";
	}

	if (is_real(a))  assembly += "\t\tfld\t" + a + "\n";
	else if ((a[0] == '[') && (a[a.length()-1] == ']')) {
		load(di, a.substr(1, a.length()-2), a_n, a_off, a_size);
		assembly += "\t\t" + instr + "\t" + size + " [" + di + "]\n";
	} else if (a_n == n_cur) {
		if (type.compare("V") == 0) assembly += "\t\t" + instr + "\t" + size + " [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
		else {
			assembly += "\t\tmov\t" + si + ", [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
			assembly += "\t\t" + instr + "\t" + size + " [" + si + "]\n";
		}
	} else {
		getAR(a_n);
		if (type.compare("V") == 0) assembly += "\t\t" + instr + "\t" + size + " [" + si + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
		else {
			assembly += "\t\tmov\t" + si + ", [" + si + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) +"]\n";
			assembly += "\t\t" + instr + "\t" + size + " [" + si + "]\n";
		}
	}
}

inline void loadAddr(std::string R, std::string a, int a_n = 0, int a_off = 0, int a_size = 10, std::string type = "V") {
	std::string size;
	if (a_size == 1) size = "byte";
	else if (a_size == 2) size = "word";
	else if (a_size == 4) size = "dword";
	else if (a_size == 8) size = "qword";

	if ((a[0] == '\"') && (a[a.length()-1] == '\"')) {
		stringsUsed.push_back(a);
		assembly += "\t\tmov\t" + rdi + ", str" + std::to_string(stringsUsed.size()) + "\n"; 
	} else if ((a[0] == '[') && (a[a.length()-1] == ']')) load(R, a.substr(1, a.length()-2), a_n, a_off, a_size, type);
	else if (a_n == n_cur) {
		if (type.compare("V") == 0) assembly += "\t\tlea\t" + R + ", " + size + " [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
		else assembly += "\t\tmov\t" + R + ", word [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
	} else {
		getAR(a_n);
		if (type.compare("V") == 0) assembly += "\t\tlea\t" + R + ", " + size + " [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
		else assembly += "\t\tmov\t" + R + ", word [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
	}
}

inline void store(std::string R, std::string a, int a_n = 0, int a_off = 0, int a_size = 1, std::string type = "V") {
	std::string size;
	if (a_size == 1) size = "byte";
	else if (a_size == 2) size = "word";
	else if (a_size == 4) size = "dword";
	else if (a_size == 8) size = "qword";

    if ((a[0] == '[') && (a[a.length()-1] == ']')) {
		load(di, a.substr(1, a.length()-2), a_n, a_off, a_size, type);
		assembly += "\t\tmov\t" + size + " [" + di + "], " + R + "\n";
	} else if (a_n == n_cur) {
		if (type.compare("V") == 0) assembly += "\t\tmov\t" + size + " [" + bp + (a_off >= 0 ?  " + " : " - ") + std::to_string(abs(a_off)) + "], " + R + "\n";
		else {
			assembly += "\t\tmov\t" + si + ", word [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
			assembly += "\t\tmov\t" + size + " [" + si + "], " + R + "\n";
		}
	} else {
		getAR(a_n);
		if (type.compare("V") == 0) assembly += "\t\tmov\t" + size + " [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "], " + R + "\n";
		else {
			assembly += "\t\tmov\t" + si + ", word [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
			assembly += "\t\tmov\t" + size + " [" + si + "], " + R + "\n";
		}
	}
}

inline void storeReal(std::string a, int a_n = 0, int a_off = 0, int a_size = 8, std::string type = "V") {
	std::string size, instr;
	if (a_size == 4) {
		size = "dword";
		instr = "fild";
	} else if (a_size == 8) {
		size = "qword";
		instr = "fld";
	}

	if ((a[0] == '[') && (a[a.length()-1] == ']')) {
		load(di, a.substr(1, a.length()-2), a_n, a_off, a_size);
		assembly += "\t\t" + instr + "\t" + size + " [" + di + "]\n";
	} else if (a_n == n_cur) {
		if (type.compare("V") == 0) assembly += "\t\t" + instr + "\t" + size + " [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
		else {
			assembly += "\t\tmov\t" + si + ", word [" + bp + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
			assembly += "\t\t" + instr + "\t" + size + " [" + si + "]\n";
		}
	} else {
		getAR(a_n);
		if (type.compare("V") == 0) assembly += "\t\t" + instr + "\t" + size + " [" + si + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) + "]\n";
		else {
			assembly += "\t\tmov\t" + si + ", word [" + si + (a_off >= 0 ? " + " : " - ") + std::to_string(abs(a_off)) +"]\n";
			assembly += "\t\t" + instr + "\t" + size + " [" + si + "]\n";
		}
	}
}

inline std::string label(std::string z) {
	if (is_number(z)) return "@" + z;
	return "@" + curFunc + "_" + std::to_string(curFNum) + "_" + z;
}

inline std::string endof(std::string x) { return "@" + x + "_" + std::to_string(curFNum); }

inline std::string ass_name(std::string z) { return "_" + z + "_" + std::to_string(curFNum); }

inline void updateAL(int n) {
	if (n_cur < n) assembly += "\t\tpush\t" + bp + "\n";
	else if (n_cur == n) assembly += "\t\tpush\tword [" + bp + " + 4]\n";
	else {
		getAR(n);
		assembly += "\t\tpush\tword [" + si + " + 4]\n";
	}
}

inline void translate(std::vector<quad> fQL) {
	curFNum = 1;

	for (const auto &q : fQL) {
		assembly += label(std::to_string(q.getTag())) + ":\n";
		std::string op = q.getOpname(), x = q.getOp1(), y = q.getOp2(), z = q.getOp3();
		if (op.compare("call") == 0) {
			assembly += "\t\tsub\t" + sp + ", 2\n";
			updateAL(q.getZdepth());
			if (!q.withReal()) assembly += "\t\tcall\t" + ass_name(z) + "\n";
			else {
				assembly += "\t\tcall\t_" + z + "\n";
				libsUsed.push_back(z);
			}
			assembly += "\t\tadd\t" + sp + ", " + std::to_string(q.getZsize() + 4) + "\n";
		} else if (op.compare("par") == 0) {
			if (y.compare("V") == 0) {
				if (q.getXsize() == 1) {
					load(al, x, q.getXdepth(), q.getXoffset(), q.getXsize());
					assembly += "\t\tsub\t" + sp + ", 1\n";
					assembly += "\t\tmov\t" + si + ", " + sp + "\n";
					assembly += "\t\tmov\tbyte [" + si + "], " + al + "\n";
				} else if(q.getXsize() == 4) {
					load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
					// assembly += "\t\tpush\t" + ax + "\n";
					assembly += "\t\tsub\t" + sp + ", 4\n";
					assembly += "\t\tmov\t[" + sp + " + 4], " + ax + "\n";
				} else {
					loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
					assembly += "\t\tsub\t" + sp + ", 10\n";
					assembly += "\t\tmov\t" + si + ", " + sp + "\n";
					assembly += "\t\tfstp\ttbyte [" + si + "]\n";
				}
			} else {
				loadAddr(rdi, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				assembly += "\t\tpush\t" + rdi + "\n";
			}
		}
		else if (op.compare("unit") == 0) {
			n_cur = q.getXdepth();
			curFunc = x;
			curFNum++;
			assembly += ass_name(x) + "\n";
			assembly += "\t\tpush\t" + bp + "\n";
			assembly += "\t\tmov\t" + bp + ", " + sp + "\n";
			assembly += "\t\tsub\t" + sp + ", " + std::to_string(q.getXsize()) + "\n";
		} else if (op.compare("endu") == 0) {
			assembly += endof(x) + ":";
			assembly += "\t\tmov\t" + sp + ", " + bp + "\n";
			assembly += "\t\tpop\t" + bp + "\n";
			assembly += "\t\tret\n";
		} else if (op.compare("ret") == 0) assembly += "\t\tjmp\t" + endof(curFunc) + "\n";
		else if (op.compare(":=") == 0) {
			if (q.withReal()) {
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				storeReal(z, q.getZdepth(), q.getZoffset(), q.getZsize());
			} else {
				if (q.getXsize() == 4) {
					load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
					store(ax, z, q.getZdepth(), q.getZoffset(), q.getZsize());
				} else  {
					load(al, x, q.getXdepth(), q.getXoffset(), q.getXsize());
					store(al, z, q.getZdepth(), q.getZoffset(), q.getZsize());
				}
			}
		} else if (op.compare("array") == 0) {
			load(ax, y, q.getYdepth(), q.getYoffset(), q.getYsize());
			assembly += "\t\tmov\t" + cx + ", " + std::to_string(q.getXsize()) + "\n";
			assembly += "\t\timul\t" + cx + "\n";
			loadAddr(cx, x, q.getXdepth(), q.getXoffset(), q.getXsize());
			assembly += "\t\tadd\t" + ax + ", " + cx + "\n";
			store(ax, z, q.getZdepth(), q.getZoffset(), q.getZsize());
		} else if (op.compare("+") == 0) {
			if (q.withReal()) {
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfaddp\t" + st0 + ", " + st1 + "\n";
				storeReal(z, q.getZdepth(), q.getZoffset(), q.getZsize());
			} else {
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				load(cx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tadd\t" + ax + ", " + cx + "\n";
				store(ax, z, q.getZdepth(), q.getZoffset(), q.getZsize());
			}
		} else if (op.compare("-") == 0) {
			if (q.withReal()) {
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfsubp\t" + st0 + ", " + st1 + "\n";
				storeReal(z, q.getZdepth(), q.getZoffset(), q.getZsize());
			} else {
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				load(cx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tsub\t" + ax + ", " + cx + "\n";
				store(ax, z, q.getZdepth(), q.getZoffset(), q.getZsize());
			}
		} else if (op.compare("*") == 0) {
			if (q.withReal()) {
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfmulp\t" + st0 + ", " +  st1 + "\n";
				storeReal(z, q.getZdepth(), q.getZoffset(), q.getZsize());
			} else {
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				load(cx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\timul\t" + ax + ", " + cx + "\n";
				store(ax, z, q.getZdepth(), q.getZoffset(), q.getZsize());
			}
		} else if (op.compare("/") == 0) {
			if (q.withReal()) {
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfdivp\t" + st1 + ", " + st0 + "\n";
				storeReal(z, q.getZdepth(), q.getZoffset(), q.getZsize());
			} else {
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				assembly += "\t\tcwd\n";
				load(cx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tidiv\t" + cx + "\n";
				store(ax, z, q.getZdepth(), q.getZoffset(), q.getZsize());
			}
		} else if (op.compare("%") == 0) {
			load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
			assembly += "\t\tcwd\n";
			load(cx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
			assembly += "\t\tidiv\t" + cx + "\n";
			store(dx, z, q.getZdepth(), q.getZoffset(), q.getZsize());
		} else if (op.compare("=") == 0) {
			std::string instr;
			if (q.withReal()) {
				instr = "jnz";
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", 4000h\n";
			} else {
				instr = "je";
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				load(dx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare("<>") == 0) {
			std::string instr;
			if (q.withReal()) {
				instr = "jz";
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", 4000h\n";
			} else {
				instr = "jne";
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				load(dx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare("<") == 0) {
			std::string instr;
			if (q.withReal()) {
				instr = "jz";
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", 4500h\n";
			} else {
				instr = "jg";
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				load(dx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare(">") == 0) {
			std::string instr;
			if (q.withReal()) {
				instr = "jnz";
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", 0100h\n";
			} else {
				instr = "jl";
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				load(dx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare("<=") == 0) {
			std::string instr;
			if (q.withReal()) {
				instr = "jz";
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", 0100h\n";
			} else {
				instr = "jge"; 
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				load(dx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare(">=") == 0) {
			std::string instr;
			if (q.withReal()) {
				instr = "jnz";
				loadReal(x, q.getXdepth(), q.getXoffset(), q.getXsize());
				loadReal(y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tfcompp\n";
				assembly += "\t\tfstsw\t" + ax + "\n";
				assembly += "\t\ttest\t" + ax + ", 4500h\n";
			} else {
				instr = "jle";
				load(ax, x, q.getXdepth(), q.getXoffset(), q.getXsize());
				load(dx, y, q.getYdepth(), q.getYoffset(), q.getYsize());
				assembly += "\t\tcmp\t" + ax + ", " + dx + "\n";
			}
			assembly += "\t\t" + instr + "\t" + label(z) + "\n";
		} else if (op.compare("ifb") == 0) {
			load(al, x, q.getXdepth(), q.getXoffset(), q.getXsize());
			assembly += "\t\tor\t" + al + ", " + al + "\n";
			assembly += "\t\tjnz\t" + label(z) + "\n";
		} else if (op.compare("jump") == 0) assembly += "\t\tjmp\t" + label(z) + "\n";
		else if (op.compare("jumpl") == 0) assembly += "\t\tjmp\t" + label(z) + "\n";
		else if (op.compare("label") == 0) assembly += label(z) + ":\n";
		else std::cout << "Op " << op << " not yet supported." << std::endl;
	}
}

#endif