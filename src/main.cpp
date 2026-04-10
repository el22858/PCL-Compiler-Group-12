#include <cstdio>
#include <memory>
#include <string>
#include <fstream>

#include "lexer.hpp"
#include "basic.hpp"
#include "symbol.hpp"
#include "ast.hpp"

#include "parser.hpp"

extern FILE *yyin;
std::unique_ptr<Body> ast;
std::string name;

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
	st.insertFormal("writeBoolean", std::make_unique<TypeProc>(), std::move(fL));

	// procedurewriteString (vars:array ofchar)
	id = std::make_unique<Id>("s");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Array>(std::make_unique<Char>()));
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("writeString", std::make_unique<TypeProc>(), std::move(fL));


	// function readInteger ():integer
	st.insertFormal("readInteger", std::make_unique<Integer>(), std::make_unique<FormalList>());

	// function readBoolean ():boolean
	st.insertFormal("readBoolean", std::make_unique<Boolean>(), std::make_unique<FormalList>());

	// function readChar ():char
	st.insertFormal("readChar", std::make_unique<Char>(), std::make_unique<FormalList>());

	// function readReal ():real
	st.insertFormal("readReal", std::make_unique<Real>(), std::make_unique<FormalList>());

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

	st.insertFormal("readString", std::make_unique<TypeProc>(), std::move(fL));


	// function abs (n:integer):integer
	id = std::make_unique<Id>("n");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));

	st.insertFormal("abs", std::make_unique<Integer>(), std::move(fL));

	// procedurewriteReal (r:real)
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("writeReal", std::make_unique<TypeProc>(), std::move(fL));

	// function fabs (r:real) :real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("fabs", std::make_unique<Real>(), std::move(fL));

	// function sqrt (r:real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("sqrt", std::make_unique<Real>(), std::move(fL));

	// function sin (r : real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("sin", std::make_unique<Real>(), std::move(fL));

	// function cos (r : real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("cos", std::make_unique<Real>(), std::move(fL));

	// function tan (r : real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("tan", std::make_unique<Real>(), std::move(fL));

	// function arctan (r:real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("arctan", std::make_unique<Real>(), std::move(fL));

	// function exp (r : real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("exp", std::make_unique<Real>(), std::move(fL));

	// function ln (r:real):real
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("ln", std::make_unique<Real>(), std::move(fL));

	// function pi () :real
	st.insertFormal("pi", std::make_unique<Real>(), std::make_unique<FormalList>());


	// function trunc(r : real):integer
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("trunc", std::make_unique<Integer>(), std::move(fL));

	// function round(r : real):integer
	id = std::make_unique<Id>("r");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Real>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("round", std::make_unique<Integer>(), std::move(fL));


	// procedurewriteChar (c:char)
	id = std::make_unique<Id>("c");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Char>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("writeChar", std::make_unique<TypeProc>(), std::move(fL));


	// function ord (c : char): integer
	id = std::make_unique<Id>("c");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Char>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("ord", std::make_unique<Integer>(), std::move(fL));


	// procedurewriteInteger (n:integer)
	id = std::make_unique<Id>("n");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("writeInteger", std::make_unique<TypeProc>(), std::move(fL));

	// function chr (n : integer) : char
	id = std::make_unique<Id>("n");
	iL = std::make_unique<IdList>();
	iL->append(std::move(id));
	f = std::make_unique<Formal>(std::move(iL), std::make_unique<Integer>());
	fL = std::make_unique<FormalList>();
	fL->append(std::move(f));
	st.insertFormal("chr", std::make_unique<Char>(), std::move(fL));

}



std::vector<quad> finalQuadList;
int quadNextTemp;

int quadNEXTQUAD() { return finalQuadList.size() + 1; }

void quadGENQUAD(std::string op, std::string x, std::string y, std::string z) { finalQuadList.push_back(quad(op, x, y, z)); }

int quadNEWTEMP() { return quadNextTemp++; }

std::vector<int> quadMAKELIST(int x) {
	std::vector<int> list;
	list.push_back(x);
	return list;
}

std::vector<int> quadEMPTYLIST() {
	std::vector<int> list;
	return list;
}

//std::vector<quad> quadMAKELIST(quad x) {
//    std::vector<quad> list;
//    list.push_back(x);
//    return list;
//}

std::vector<int> quadMERGELISTS(std::vector<int> l1, std::vector<int> l2) {
	std::vector<int> l;

	if (l1.size() >= l2.size()) {
		l = l1;
		l.insert(l.end(), l2.begin(), l2.end());
	} else {
		l = l2;
		l.insert(l.begin(), l1.begin(), l1.end());
	}
	return l;
}

void quadBACKPATCH(std::vector<int> l, std::string newAd) { for (const auto &x : l) finalQuadList[x].setOp3(newAd); }

int main(int argc, char** argv) {
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

	st.exitScope();

	int printedIntermediate = 0;

	for (int i=1; i<argc; ++i) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'i':
					std::cout << finalQuadList;
					printedIntermediate = 1;
					break;

                case 'O':
                
                case 'f':
                
                case 'o':

				default:
					break;
			}
		}
	}
	if (!printedIntermediate){
		std::string intermediateOutput = "./" + ast->getBodyName() + ".imm";
		std::ofstream imFile(intermediateOutput);
		imFile << finalQuadList;
	}
}