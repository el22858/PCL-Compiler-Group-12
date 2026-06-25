#include <cstdio>
#include <memory>
#include <string>
#include <fstream>

#include "lexer.hpp"
#include "basic.hpp"
#include "symbol.hpp"
#include "ast.hpp"
#include "opt.hpp"

#include "ass.hpp"
#include "parser.hpp"

extern FILE *yyin;
std::unique_ptr<Body> ast;
std::string name, assembly;
std::vector<std::string> stringsUsed, libsUsed;
std::vector<std::pair<std::string, int>> tmpLine;
std::vector<int> labels, jumps;
int funcNum;

void initLibs() {
	std::unique_ptr<Id> id;
	std::unique_ptr<IdList> iL;
	std::unique_ptr<Formal> f;
	std::unique_ptr<FormalList> fL;

	// procedurewriteBoolean (b:boolean)
	id = std::make_unique<Id>("b");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Boolean>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("writeBoolean", std::make_unique<TypeProc>(), std::move(fL), true);

	// procedurewriteString (vars:array ofchar)
	id = std::make_unique<Id>("s");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Array>(std::make_unique<Char>()), true);
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("writeString", std::make_unique<TypeProc>(), std::move(fL), true);


	// function readInteger ():integer
	st.insertFormal("readInteger", std::make_unique<Integer>(), std::make_unique<FormalList>(), true);

	// function readBoolean ():boolean
	st.insertFormal("readBoolean", std::make_unique<Boolean>(), std::make_unique<FormalList>(), true);

	// function readChar ():char
	st.insertFormal("readChar", std::make_unique<Char>(), std::make_unique<FormalList>(), true);

	// function readReal ():real
	st.insertFormal("readReal", std::make_unique<Real>(), std::make_unique<FormalList>(), true);

	// procedure readString (size:integer;var s :arrayof char)
	id = std::make_unique<Id>("size");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));

	id = std::make_unique<Id>("s");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Array>(std::make_unique<Char>()));
	fL->append(std::move(f));

	st.insertFormal("readString", std::make_unique<TypeProc>(), std::move(fL), true);


	// function abs (n:integer):integer
	id = std::make_unique<Id>("n");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));

	st.insertFormal("abs", std::make_unique<Integer>(), std::move(fL), true);

	// procedurewriteReal (r:real)
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("writeReal", std::make_unique<TypeProc>(), std::move(fL), true);

	// function fabs (r:real) :real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("fabs", std::make_unique<Real>(), std::move(fL), true);

	// function sqrt (r:real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("sqrt", std::make_unique<Real>(), std::move(fL), true);

	// function sin (r : real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("sin", std::make_unique<Real>(), std::move(fL), true);

	// function cos (r : real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("cos", std::make_unique<Real>(), std::move(fL), true);

	// function tan (r : real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("tan", std::make_unique<Real>(), std::move(fL), true);

	// function arctan (r:real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("arctan", std::make_unique<Real>(), std::move(fL), true);

	// function exp (r : real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("exp", std::make_unique<Real>(), std::move(fL), true);

	// function ln (r:real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("ln", std::make_unique<Real>(), std::move(fL), true);

	// function pi () :real
	st.insertFormal("pi", std::make_unique<Real>(), std::make_unique<FormalList>(), true);


	// function trunc(r : real):integer
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("trunc", std::make_unique<Integer>(), std::move(fL), true);

	// function round(r : real):integer
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("round", std::make_unique<Integer>(), std::move(fL), true);


	// procedurewriteChar (c:char)
	id = std::make_unique<Id>("c");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Char>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("writeChar", std::make_unique<TypeProc>(), std::move(fL), true);


	// function ord (c : char): integer
	id = std::make_unique<Id>("c");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Char>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("ord", std::make_unique<Integer>(), std::move(fL), true);


	// procedurewriteInteger (n:integer)
	id = std::make_unique<Id>("n");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("writeInteger", std::make_unique<TypeProc>(), std::move(fL), true);

	// function chr (n : integer) : char
	id = std::make_unique<Id>("n");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("chr", std::make_unique<Char>(), std::move(fL), true);

}



std::vector<quad> finalQuadList;
int quadNextTemp, n_cur;
bool hasChanged;

int main(int argc, char** argv) {
	funcNum = 0;
	yyin = fopen(argv[1], "r");
	if (yyin == nullptr) {
		perror(argv[1]);
		return 1;
	}
	yyrestart(yyin);

	yy::parser p;
	p.parse();
	//std::cout << ast->getName() << std::endl;
	//std::cout << *ast << std::endl;

	quadNextTemp = 1;
	st.enterScope();
	st.insertParent(name);
	initLibs();
	ast->sem();
	ast->igen();

	st.exitScope();

	int printIMM = 0, printASS = 0, opt = 0;

	for (int i=2; i<argc; ++i) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'i':
					printIMM = 1;
					break;
                case 'O':
					opt = 1;
					break;
                case 'f':
					printASS = 1;
					break;
                case 'o':

				default:
					break;
			}
		}
	}

	if (opt) optimize();

	if (printIMM) std::cout << finalQuadList;
	else {
		std::string intermediateOutput = "./" + ast->getBodyName() + ".imm";
		std::ofstream imFile(intermediateOutput);
		imFile << finalQuadList;
	}

	// libsUsed.push_back("writeString");
	// stringsUsed.push_back("Hello world\n");
	translate(finalQuadList);
	prologue(ast->getBodyName());
	epilogue();

	if (printASS) std::cout << assembly;
	else {
		std::string assOutput = "./" + ast->getBodyName() + ".asm";
		std::ofstream asFile(assOutput);
		asFile << assembly;
	}
}