#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <cstring>
#include <vector>

#include "quads.hpp"
#include "tree.hpp"
#include "type.hpp"
#include "symbol.hpp"


class Stmt : public AST {
	private:
	public:
		std::vector<int> quadNEXT;
};


class StmtList : public Stmt {
	private:
		std::vector<std::unique_ptr<Stmt>> stmt_list;
	public:
		StmtList() : stmt_list() {}

		virtual void printAST(std::ostream &out) const override {
			out << "StatementList(";
			bool start = true;

			if (stmt_list.size() > 0 ) {
				for (const auto &x : stmt_list) {
					if (!start) out << ", ";
					start = false;
					if (x) x->printAST(out);
				}
			}
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "StatementList(";
			bool start = true;

			if (stmt_list.size() > 0) {
				for (const auto &x : stmt_list) {
					if (!start) res += ", ";
					start = false;
					if (x) res += x->getName();
				}
			}
			res += ")";
			return res;
		}
		std::vector<std::unique_ptr<Stmt>> const& getList() { return stmt_list; }
		virtual void sem() override {
			for (auto &s : stmt_list) {
				if (s) s->sem();
			}
		}

		void append(std::unique_ptr<Stmt> s) { /*stmt_list.insert(stmt_list.begin(), std::move(s));*/ stmt_list.push_back(std::move(s)); }
		void appendAtStart(std::unique_ptr<Stmt> s) { stmt_list.insert(stmt_list.begin(), std::move(s)); }

		virtual void igen() override {
			for (auto &s : stmt_list) {
				if (s) s->igen();
			}
		}
};


class Expr : public AST {
	protected:
	public:
		std::shared_ptr<Type> type;
		std::string place;
		std::vector<int> quadTRUE, quadFALSE;
		virtual bool isRes() { return false; }
		virtual bool isCond() { return false; }
		virtual std::string getOp() { return ""; } 
		Types getType() { return type->getType(); }
		bool typeCheck(Types t) { return type->getType() == t; }
		void quadGENBOOL() {
			quadTRUE = quadMAKELIST(quadNEXTQUAD());
			quadGENQUAD("ifb", place, "-", "*");
			quadFALSE = quadMAKELIST(quadNEXTQUAD());
			quadGENQUAD("jump", "-", "-", "*");
		}
};


class ExprList : public Expr {
	private:
		std::vector<std::unique_ptr<Expr>> expr_list;
	public:
		ExprList() : expr_list() {}

		void append(std::unique_ptr<Expr> e) { expr_list.push_back(std::move(e)); }
		void appendAtStart(std::unique_ptr<Expr> e) { expr_list.insert(expr_list.begin(), std::move(e)); }
		virtual void printAST(std::ostream &out) const override {
			out << "ExpressionList(";
			bool start = true;

			for (const auto &x : expr_list) {
				if (!start) out << ", ";
				start = false;
				x->printAST(out);
			}
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "ExpressionList(";
			bool start = true;

			for (const auto &x : expr_list) {
				if (!start) res += ", ";
				start = false;
				res += x->getName();
			}
			res += ")";
			return res;
		}
		std::vector<std::unique_ptr<Expr>> const& getList() { return expr_list; }

		virtual void sem() override { for (auto &e : expr_list) e->sem(); }

		virtual void igen() override { for (auto &e : expr_list) e->igen(); }
};


class Block : public Stmt {
	private:
		std::unique_ptr<StmtList> stmt_list;
	public:
		Block(std::unique_ptr<StmtList> sL = std::move(std::make_unique<StmtList>())) : stmt_list(std::move(sL)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Block(";
			stmt_list->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Block(";
			res += stmt_list->getName();
			res += ")";
			return res;
		}
		virtual void sem() override { for (auto &s : stmt_list->getList()) s->sem(); }

		virtual void igen() override {
			std::vector<int> L = quadEMPTYLIST();
			for (auto &s : stmt_list->getList()) {
				s->igen();
				quadBACKPATCH(L, std::to_string(quadNEXTQUAD()));
				L = s->quadNEXT;
			}
			quadNEXT = L;
		}
};


class ITE : public Stmt {
	private:
		std::unique_ptr<Expr> expr;
		std::unique_ptr<Stmt> stmt1, stmt2;
	public:
		ITE(std::unique_ptr<Expr> e, std::unique_ptr<Stmt> s1, std::unique_ptr<Stmt> s2 = nullptr) : expr(std::move(e)), stmt1(std::move(s1)), stmt2(std::move(s2)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "If(";
			expr->printAST(out);
			out << ", ";
			stmt1->printAST(out);
			if (stmt2 != nullptr) {
				out << ", ";
				stmt2->printAST(out);
			}
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "If(";
			res += expr->getName();
			res += ", ";
			res += stmt1->getName();
			if (stmt2) {
				res += ", ";
				res += stmt2->getName();
			}
			res += ")";
			return res;
		}
		virtual void sem() override {
			expr->sem();
			if (expr->typeCheck(TYPE_BOOLEAN)) {
				stmt1->sem();
				if (stmt2)  stmt2->sem();
			} else yyerror("Expected boolean for ITE.");
		}

		virtual void igen() override {
			std::vector<int> L1, L2;
			expr->igen();
			quadBACKPATCH(expr->quadTRUE, std::to_string(quadNEXTQUAD()));
			L1 = expr->quadFALSE;
			L2 = quadEMPTYLIST();

			stmt1->igen();

			if (stmt2) {
				L1 = quadMAKELIST(quadNEXTQUAD());
				quadGENQUAD("jump", "-", "-", "*");
				quadBACKPATCH(expr->quadFALSE, std::to_string(quadNEXTQUAD()));

				stmt2->igen();
				L2 = stmt2->quadNEXT;
			}

			L1 = quadMERGELISTS(L1, stmt1->quadNEXT);
			quadNEXT = quadMERGELISTS(L1, L2);
		}
};


class While : public Stmt {
	private:
		std::unique_ptr<Expr> expr;
		std::unique_ptr<Stmt> stmt;
	public:
		While(std::unique_ptr<Expr> e, std::unique_ptr<Stmt> s) : expr(std::move(e)), stmt(std::move(s)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "While(";
			expr->printAST(out);
			out << ", ";
			stmt->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "While(";
			res += expr->getName();
			res += ", ";
			res += stmt->getName();
			res += ")";
			return res;
		}
		virtual void sem() override {
			expr->sem();
			if (expr->typeCheck(TYPE_BOOLEAN)) stmt->sem();
			else yyerror("Expected boolean for While loop.");
		}

		virtual void igen() override {
			int Q = quadNEXTQUAD();
			expr->igen();
			quadBACKPATCH(expr->quadTRUE, std::to_string(quadNEXTQUAD()));

			stmt->igen();
			quadBACKPATCH(stmt->quadNEXT, std::to_string(Q));
			quadGENQUAD("jump", "-", "-", std::to_string(Q));
			
			quadNEXT = expr->quadFALSE;
		}
};


class RVal: public Expr {
	public:
};

class LVal: public Expr {
	public:
};


class UnOp : public RVal {
	private:
		std::string op;
		std::unique_ptr<Expr> expr;
	public:
		UnOp(std::string s, std::unique_ptr<Expr> e) : op(s), expr(std::move(e)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "UnOp(" << op << ", ";
			expr->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "UnOp(";
			res += op;
			res += ", ";
			res += expr->getName();
			res += ")";
			return res;
		}
		virtual std::string getOp() override { return op; }
		virtual void sem() override {
			expr->sem();

			if (op.compare("+") == 0) {
				if (expr->typeCheck(TYPE_INTEGER)) type = std::make_shared<Integer>();
				else if (expr->typeCheck(TYPE_REAL)) type = std::make_shared<Real>();
				else yyerror("Expected number");
			} else if (op.compare("-") == 0) {
				if (expr->typeCheck(TYPE_INTEGER)) type = std::make_shared<Integer>();
				else if (expr->typeCheck(TYPE_REAL)) type = std::make_shared<Real>();
				else yyerror("Expected number");
			} else if (op.compare("not") == 0) {
				if (expr->typeCheck(TYPE_BOOLEAN)) type = std::make_shared<Boolean>();
				else yyerror("Expected boolean.");
			} else yyerror("Unary Operator not recognized"); // Trash
		}

		virtual void igen() override {
			expr->igen();

			if (op.compare("+") == 0) place = expr->place;
			else if (op.compare("-") == 0) {
				place = "$" + std::to_string(quadNEWTEMP());
				quadGENQUAD(op, expr->place, "-", place);
			} else if (op.compare("not") == 0) {
				quadTRUE = expr->quadFALSE;
				quadFALSE = expr->quadTRUE;
			}
		}
};


class BinOp : public RVal {
	private:
		std::unique_ptr<Expr> expr1, expr2;
		std::string op;
	public:
		BinOp(std::unique_ptr<Expr> e1, std::string s, std::unique_ptr<Expr> e2) : expr1(std::move(e1)), expr2(std::move(e2)), op(s) {}

		virtual void printAST(std::ostream &out) const override {
			out << op << "(";
			expr1->printAST(out);
			out << ", " << op << ", ";
			expr2->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = op +"(";
			res += expr1->getName();
			res += ", ";
			res += expr2->getName();
			res += ")";
			return res;
		}
		virtual std::string getOp() override { return op; }
		virtual void sem() override {
			if ((op.compare("+") == 0) || (op.compare("-") == 0) || (op.compare("*") == 0)) {
				expr1->sem();
				expr2->sem();
				if (expr1->typeCheck(TYPE_INTEGER)) {
					if (expr2->typeCheck(TYPE_INTEGER)) type = std::make_shared<Integer>();
					else if (expr2->typeCheck(TYPE_REAL)) type = std::make_shared<Real>();
					else yyerror("Expected int or real.");
				} else if (expr1->typeCheck(TYPE_REAL)) {
					if ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL))) type = std::make_shared<Real>();
					else yyerror("Expected int or real.");
				}
			} else if (op.compare("/") == 0) {
				expr1->sem();
				expr2->sem();
				if (((expr1->typeCheck(TYPE_INTEGER)) || ((expr1->typeCheck(TYPE_REAL)))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = std::make_shared<Real>();
				else yyerror("Expected int or real.");
			} else if ((op.compare("div") == 0) || (op.compare("mod") == 0)) {
				expr1->sem();
				expr2->sem();
				if ((expr1->typeCheck(TYPE_INTEGER)) && (expr2->typeCheck(TYPE_INTEGER))) type = std::make_shared<Integer>();
				else yyerror("Expected int.");
			} else if ((op.compare("and") == 0) || (op.compare("or") == 0)) {
				expr1->sem();
				if (!expr1->typeCheck(TYPE_BOOLEAN)) yyerror("Expected boolean.");

				expr2->sem();
				if (!expr2->typeCheck(TYPE_BOOLEAN)) yyerror("Expected boolean.");
				type = std::make_shared<Boolean>();
			} else if ((op.compare("<") == 0) || (op.compare(">") == 0) || (op.compare("<=") == 0) || (op.compare(">=") == 0)) {
				expr1->sem();
				expr2->sem();
				if (((expr1->typeCheck(TYPE_INTEGER)) || (expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = std::make_shared<Boolean>();
				else yyerror("Expected int or real.");
			} else if ((op.compare("=") == 0) || (op.compare("<>") == 0)) {
				expr1->sem();
				expr2->sem();
				if (expr1->type->getName().compare(expr2->type->getName()) == 0) type = std::make_shared<Boolean>();
				else yyerror("Type mismatch!");
			}
		}

		virtual void igen() override {
			if ((op.compare("+") == 0) || (op.compare("-") == 0) || (op.compare("*") == 0) || (op.compare("/") == 0) || (op.compare("div") == 0) || (op.compare("mod") == 0)) {
				expr1->igen();
				expr2->igen();
				place = "$" + std::to_string(quadNEWTEMP());
				quadGENQUAD(op, expr1->place, expr2->place, place);
			} else if (op.compare("and") == 0) {
				expr1->igen();
				// quadGENQUAD("dude", "go", "to", std::to_string(quadNEXTQUAD()));
				quadBACKPATCH(expr1->quadTRUE, std::to_string(quadNEXTQUAD()));

				expr2->igen();
				quadTRUE = expr2->quadTRUE;
				quadFALSE = quadMERGELISTS(expr1->quadFALSE, expr2->quadFALSE);
			} else if (op.compare("or") == 0) {
				expr1->igen();
				quadBACKPATCH(expr1->quadFALSE, std::to_string(quadNEXTQUAD()));

				expr2->igen();
				quadTRUE = quadMERGELISTS(expr1->quadTRUE, expr2->quadTRUE);
				quadFALSE = expr2->quadFALSE;
			} else if ((op.compare("<") == 0) || (op.compare(">") == 0) || (op.compare("<=") == 0) || (op.compare(">=") == 0)) {
				expr1->igen();
				expr2->igen();

				quadTRUE = quadMAKELIST(quadNEXTQUAD());
				quadGENQUAD(op, expr1->place, expr2->place, "*");
				quadFALSE = quadMAKELIST(quadNEXTQUAD());
				quadGENQUAD("jump", "-", "-", "*");
			} else if ((op.compare("=") == 0) || (op.compare("<>") == 0)) {
				expr1->igen();
				expr2->igen();

				quadTRUE = quadMAKELIST(quadNEXTQUAD());
				quadGENQUAD(op, expr1->place, expr2->place, "*");
				quadFALSE = quadMAKELIST(quadNEXTQUAD());
				quadGENQUAD("jump", "-", "-", "*");
			}
		}
};


class IntConst : public RVal {
	private:
		int val;
	public:
		IntConst(int n) : val(n) {}

		virtual void printAST(std::ostream &out) const override { out << "IntConst(" << val << ")"; }
		virtual std::string getName() const override { return "IntConst(" + std::to_string(val) + ")"; }
		virtual void sem() override { type = std::make_shared<Integer>(); }

		virtual void igen() override { place = std::to_string(val); }
};


class BoolConst : public RVal {
	private:
		bool val;
	public:
		BoolConst(bool b) : val(b) {}

		virtual void printAST(std::ostream &out) const override { out << "BoolConst('" << val << "')"; }
		virtual std::string getName() const override { return (val) ? "BoolConst(true)" : "BoolConst(false)" ; }
		virtual void sem() override { type = std::make_shared<Boolean>(); }

		virtual void igen() override {
			place = (val) ? "true" : "false";
			quadGENBOOL();
		}
};


class RealConst : public RVal {
	private:
		float val;
	public:
		RealConst(float f) : val(f) {}

		virtual void printAST(std::ostream &out) const override { out << "RealConst(" << val << ")"; }
		virtual std::string getName() const override { return "RealConst(" + std::to_string(val) + ")"; }
		virtual void sem() override { type = std::make_shared<Real>(); }

		virtual void igen() override { place = std::to_string(val); }
};

class CharConst : public RVal {
	private:
		char *val;
	public:
		CharConst(char *c) : val(c) {}

		virtual void printAST(std::ostream &out) const override {out << "CharConst(" << val << ")"; }
		virtual std::string getName() const override { return "CharConst(" + std::string(val) + ")"; }
		virtual void sem() override { type = std::make_shared<Char>(); }

		virtual void igen() override { place = val; }
};

class NilLVal : public LVal {
	private:
	public:
		NilLVal() {}

		virtual void printAST(std::ostream &out) const override { out << "Nil()"; }
		virtual std::string getName() const override { return "Nil()"; }
		virtual void sem() override {
			type = std::make_shared<TypeNil>();

			place = "nil";
		}

		virtual void igen() override {  }
};

class NilRVal : public RVal {
	private:
	public:
		NilRVal() {}

		virtual void printAST(std::ostream &out) const override { out << "Nil()";}
		virtual std::string getName() const override { return "Nil()"; }
		virtual void sem() override { type = std::make_shared<TypeNil>(); }

		virtual void igen() override { place = "nil"; }
};


class StringLit : public LVal {
	private:
		std::string val;
	public:
		StringLit(std::string s) : val(s) {}

		virtual void printAST(std::ostream &out) const override { out << "StringLit(" << val << ")"; }
		virtual std::string getName() const override { return "StringLit(" + val + ")"; } 
		virtual void sem() override { type = std::make_shared<Array>(std::move(std::make_unique<Char>()), val.size()); }

		virtual void igen() { place = val; }
};


class Id : public LVal {
	private:
		std::string id;
		int offset;
	public:
		Id(std::string s) : id(s), offset(-1) {}

		virtual void printAST(std::ostream &out) const override { out << "Id(" << id << ")"; }
		virtual std::string getName() const override { return "Id(" + id + ")"; }
		virtual void sem() override {
			type = st.lookup(id)->type;
			STEntry *e = st.lookup(id);
			offset = e->offset;

			place = id;
		}
		std::string getId() { return id; }

		virtual void igen() {
			if (typeCheck(TYPE_BOOLEAN)) {
				quadTRUE = quadMAKELIST(quadNEXTQUAD());
				quadFALSE = quadMAKELIST(quadNEXTQUAD());
				quadGENQUAD("jump", "-", "-", "*");
			}
		}
};


class IdList : public AST {
	private:
		std::vector<std::unique_ptr<Id>> idList;
	public:
		IdList(): idList() {}

		std::vector<std::unique_ptr<Id>> const& getList() { return idList; }

		void append(std::unique_ptr<Id> id) { idList.push_back(std::move(id)); }
		void appendAtStart(std::unique_ptr<Id> id) { idList.insert(idList.begin(), std::move(id)); }
		virtual void printAST(std::ostream &out) const override {
			out << "IdList(";
			bool start = true;

			for (const auto &id : idList) {
				if (!start) out << ", ";
				start = false;
				id->printAST(out);
			}
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "IdList(";
			bool start = true;

			for (const auto &id : idList) {
				if (!start) res += ", ";
				start = false;
				res += id->getName();
			}
			res += ")";
			return res;
		}
		virtual void sem() override { for (auto &id : idList) id->sem(); }

		virtual void igen() override { for (auto &id : idList) id->igen(); }
};


class IdLabel : public Stmt {
	private:
		std::unique_ptr<Id> id;
		std::unique_ptr<Stmt> stmt;
	public:
		IdLabel(std::unique_ptr<Id> i, std::unique_ptr<Stmt> s) : id(std::move(i)), stmt(std::move(s)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "IdLabel(";
			id->printAST(out);
			out << ", ";
			stmt->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "IdLabel(";
			res += id->getName();
			res += ", ";
			res += stmt->getName();
			res += ")";
			return res;
		}
		virtual void sem() override {
			std::string c = id->getId();
			if (!st.isLabel(c)) yyerror("Not a label.");
			else {
				stmt->sem();
				st.insertLabelStmt(c, std::move(stmt));
			}
		}

		virtual void igen() override {
			quadGENQUAD("label", id->getId(), "-", "-");
			stmt->igen();
			quadNEXT = stmt->quadNEXT;
		}
};


class ArrayItem: public LVal {
	private:
		std::unique_ptr<LVal> lVal;
		std::unique_ptr<Expr> expr;
	public:
		ArrayItem(std::unique_ptr<LVal> l, std::unique_ptr<Expr> e) : lVal(std::move(l)), expr(std::move(e)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "ArrayItem(";
			lVal->printAST(out);
			out << ", ";
			expr->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "ArrayItem(";
			res += lVal->getName();
			res += ", ";
			res += expr->getName();
			res += ")";
			return res;
		}
		virtual void sem() override {
			lVal->sem();
			if (lVal->typeCheck(TYPE_RES)) lVal->type = st.lookup("result")->type;

			expr->sem();
			if (expr->typeCheck(TYPE_RES)) expr->type = st.lookup("result")->type;

			if (!(lVal->typeCheck(TYPE_ARRAY))) {
				std::cout<<"HUHHH???"<<std::endl;
				std::cout << lVal->getName() << " should be an array but it is not. Instead, it's a(n) " << lVal->getType() << "." << std::endl;
				std::cout << *lVal << std::endl;
				std::cout << *(lVal->type) << std::endl;
				exit(0);
			}
			else if (!(expr->typeCheck(TYPE_INTEGER))) yyerror("Expected integer");

			type = lVal->type->getArrayType();
		}

		virtual void igen() override {
			lVal->igen();
			expr->igen();

			place = "$" + std::to_string(quadNEWTEMP());
			quadGENQUAD("array", lVal->place, expr->place, place);
			place = "[" + place + "]";
		}
};


class Formal : public AST {
	private:
		std::unique_ptr<IdList> idList;
		std::shared_ptr<Type> type;
		bool byRef;
	public:
		Formal(std::unique_ptr<IdList> iL, std::shared_ptr<Type> t, bool b=false) : idList(std::move(iL)), type(t), byRef(b) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Formal(";
			idList->printAST(out);
			out << ", ";
			type->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Formal(";
			res += idList->getName();
			res += ", ";
			res += type->getName();
			res += ")";
			return res;
		}
		std::vector<std::unique_ptr<Id>> const& getIdList() { return idList->getList(); }
		std::string getType() { return type->getName(); }
        std::string quadPARAMMODE() const {
            if (byRef) return "R";
            return "V";
        }
		virtual void sem() override { for (const auto &id : idList->getList()) st.insert(id->getId(), type); }

		virtual void igen() override {  }
};


class FormalList : public AST {
	private:
		std::vector<std::unique_ptr<Formal>> formalList;
	public:
		FormalList() : formalList() {}

		std::vector<std::unique_ptr<Formal>> const& getList() { return formalList; }
		virtual bool isEmpty() { return formalList.empty(); }

		void append(std::unique_ptr<Formal> f) { formalList.push_back(std::move(f)); }
		void appendAtStart(std::unique_ptr<Formal> f) { formalList.insert(formalList.begin(), std::move(f)); }
		virtual void printAST(std::ostream &out) const override {
			out << "FormalList(";
			bool start = true;

			for (const auto &f : formalList) {
				if (!start) out << ", ";
				start = false;
				f->printAST(out);
			}
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "FormalList(";
			bool start = true;

			for (const auto &f : formalList) {
				if (!start) res += ", ";
				start = false;
				res += f->getName();
			}
			res += ")";
			return res;
		}
		virtual void sem() override { for (auto &f : formalList) f->sem(); }

		virtual void igen() override { for (auto &f : formalList) f->igen(); }
};


class Call : public Stmt {
	private:
		std::unique_ptr<Id> id;
		std::unique_ptr<ExprList> func;
		std::shared_ptr<FormalList> fL;
		bool isProc;
	public:
		Call(std::unique_ptr<Id> i, std::unique_ptr<ExprList> eL=std::move(std::make_unique<ExprList>())) : id(std::move(i)), func(std::move(eL)), fL(nullptr), isProc(false) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Call(";
			id->printAST(out);
			out << ", ";
			func->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Call(";
			res += id->getName();
			res += ", ";
			res += func->getName();
			res += ")";
			return res;
		}
		virtual void sem() override {
			if (func) func->sem();

			std::string funcName = id->getId();
			st.lookup(funcName); /* Will throw error if it doesn't exist */
			if (!st.isFormal(funcName)) yyerror("Can't be called.");

			fL = st.getParams(funcName);

			int i = 0, expArgs = 0, actArgs = 0;
			if (!(fL->isEmpty())) {
				for (const auto &f : fL->getList()) expArgs += f->getIdList().size();
			}
			if (func) actArgs = func->getList().size();
			if (expArgs != actArgs) yyerror("Incorrect number of arguments given.");

			if (!(fL->isEmpty())) {
				for (const auto &f : fL->getList()) {
					int k = f->getIdList().size();
					for (int j=0; j<k; j++) {
						if ((f->getType()).compare(func->getList()[i]->type->getNameNoSize()) && (!(((f->getType()).compare(func->getList()[i]->type->getNameNoSize()) == 0) || (((f->getType().compare("Real()")==0) && (func->getList()[i]->typeCheck(TYPE_INTEGER))))))) yyerror("Type mismatch.");
					}
				}
			}

			isProc = (st.lookup(id->getId())->type->getType() == TYPE_PROC);

			st.refreshFormals(funcName, fL);
		}

		virtual void igen() override {
			int i = 0;
			if (func) func->igen();

			if (!fL->isEmpty()) {
				for (const auto &f : fL->getList()) {
					if (func->getList()[i]->typeCheck(TYPE_BOOLEAN)) {
						std::string W = "$" + std::to_string(quadNEWTEMP());
						quadBACKPATCH(func->getList()[i]->quadTRUE, std::to_string(quadNEXTQUAD()));
						quadGENQUAD(":=", "true", "-", W);
						quadGENQUAD("jump", "-", "-", std::to_string(quadNEXTQUAD() + 2));

						quadBACKPATCH(func->getList()[i]->quadFALSE, std::to_string(quadNEXTQUAD()));
						quadGENQUAD(":=", "false", "-", W);
						func->getList()[i]->place = W;
					}

					if ((f->quadPARAMMODE().compare("V")==0) && (f->getType().compare(func->getList()[i]->type->getName()) && (((f->getType()).compare(func->getList()[i]->type->getNameNoSize()) == 0) || (((f->getType().compare("Real()")==0) && (func->getList()[i]->typeCheck(TYPE_INTEGER))))))){
						std::string W = "$" + std::to_string(quadNEWTEMP());
						quadGENQUAD(":=", func->getList()[i]->place, "-", W);
						quadGENQUAD("par", W, "V", "-");
					} else quadGENQUAD("par", func->getList()[i]->place, f->quadPARAMMODE(), "-");

					++i;
				}
			}

			if (!isProc) {
                std::string W = "$" + std::to_string(quadNEWTEMP());
                quadGENQUAD("par", "RET", W, "-");
            }

			quadGENQUAD("call", "-", "-", id->getId());
			quadNEXT = quadEMPTYLIST();
		}
};


class CallRVal : public RVal {
	private:
		std::unique_ptr<Id> id;
		std::unique_ptr<ExprList> func;
		std::shared_ptr<FormalList> fL;

	public:
		CallRVal(std::unique_ptr<Id> i, std::unique_ptr<ExprList> eL=std::move(std::make_unique<ExprList>())) : id(std::move(i)), func(std::move(eL)), fL(nullptr) {}

		virtual void printAST(std::ostream &out) const override {
			out << "CallRVal(";
			id->printAST(out);
			out << ", ";
			func->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "CallRVal(";
			res += id->getName();
			res += ", ";
			res += func->getName();
			res += ")";
			return res;
		}
		virtual void sem() override {
			if (func) func->sem();

			std::string funcName = id->getId();
			st.lookup(funcName); /* Will throw error if it doesn't exist */
			if (!st.isFormal(funcName)) yyerror("Can't be called.");

			fL = st.getParams(funcName);

			int i = 0, expArgs = 0, actArgs = 0;
			if (!(fL->isEmpty())) {
				for (const auto &f : fL->getList()) expArgs += f->getIdList().size();
			}
			if (func) actArgs = func->getList().size();
			if (expArgs != actArgs) yyerror("Incorrect number of arguments given.");

			if (!(fL->isEmpty())) {
				for (const auto &f : fL->getList()) {
					int k = f->getIdList().size();
					for (int j=0; j<k; j++) {
						if ((f->getType()).compare(func->getList()[i]->type->getName()) || ((f->getType().compare("Real()")==0) && (func->getList()[i]->typeCheck(TYPE_INTEGER)))) yyerror("Type mismatch.");
					}
				}
			}
			type = std::move(st.lookup(funcName)->type);

			st.refreshFormals(funcName, fL);
		}

		virtual void igen() override {
			int i = 0;
			if (func) func->igen();

			if (!fL->isEmpty()) {
				for (const auto &f : fL->getList()) {
					if (func->getList()[i]->typeCheck(TYPE_BOOLEAN)) {
						std::string W = "$" + std::to_string(quadNEWTEMP());
						quadBACKPATCH(func->getList()[i]->quadTRUE, std::to_string(quadNEXTQUAD()));
						quadGENQUAD(":=", "true", "-", W);
						quadGENQUAD("jump", "-", "-", std::to_string(quadNEXTQUAD() + 2));

						quadBACKPATCH(func->getList()[i]->quadFALSE, std::to_string(quadNEXTQUAD()));
						quadGENQUAD(":=", "false", "-", W);
						func->getList()[i]->place = W;
					}

					if ((f->quadPARAMMODE().compare("V")==0) && (f->getType().compare(func->getList()[i]->type->getName()) && (((f->getType()).compare(func->getList()[i]->type->getNameNoSize()) == 0) || (((f->getType().compare("Real()")==0) && (func->getList()[i]->typeCheck(TYPE_INTEGER))))))){
						std::string W = "$" + std::to_string(quadNEWTEMP());
						quadGENQUAD(":=", func->getList()[i]->place, "-", W);
						quadGENQUAD("par", W, "V", "-");
					} else quadGENQUAD("par", func->getList()[i]->place, f->quadPARAMMODE(), "-");

					++i;
				}
			}

			quadGENQUAD("call", "-", "-", id->getId());
			place = "$" + std::to_string(quadNEWTEMP());
			quadGENQUAD("par", "RET", place, "-");
			quadGENQUAD("call", "-", "-", id->getId());
		}
};


class Dispose: public Stmt {
	private:
		std::unique_ptr<LVal> lVal;
		bool bracket;
	public:
		Dispose(std::unique_ptr<LVal> l, bool b) : lVal(std::move(l)), bracket(b) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Dispose(";
			lVal->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Dispose(";
			res += lVal->getName();
			res += ")";
			return res;
		}
		virtual void sem() override {
			lVal->sem();
			if (lVal->typeCheck(TYPE_RES)) lVal->type = st.lookup("result")->type;
			if (!(lVal->typeCheck(TYPE_POINTER))) yyerror("Expected pointer.");
			if (!st.isNew(lVal->getName())) yyerror("Cannot dispose of non-new value.");

			if (bracket && (lVal->type->getPointerType()->getType() != TYPE_ARRAY)) yyerror("Something something pointer to array.");

			lVal = std::move(std::make_unique<NilLVal>());
		}

		virtual void igen() override {
			// lVal->igen(); // FIXME lVal is Nil at this point

			quadGENQUAD("par", lVal->place, "R", "-");
			quadGENQUAD("call", "-", "-", "dispose");
			quadNEXT = quadEMPTYLIST();
		}
};


class Label: public Stmt {
	private:
		std::unique_ptr<IdList> idList;
	public:
		Label(std::unique_ptr<IdList> iL) : idList(std::move(iL)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Label(";
			idList->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Label(";
			res += idList->getName();
			res += ")";
			return res;
		}
		virtual void sem() override { for (const auto &id : idList->getList()) st.insertLabel(id->getId(), std::make_unique<TypeLbl>()); }
};


class Assign: public Stmt {
	private:
		std::unique_ptr<LVal> lval;
		std::unique_ptr<Expr> expr;
	public:
		Assign(std::unique_ptr<LVal> l, std::unique_ptr<Expr> e) : lval(std::move(l)), expr(std::move(e)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Assign(";
			if (lval) lval->printAST(out);
			out << ", ";
			expr->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Assign(";
			res += lval->getName();
			res += ", ";
			res += expr->getName();
			res += ")";
			return res;
		}
		virtual void sem() override {
			lval->sem();
			expr->sem();

			if (lval->isRes()) {
				if (!(st.hasRes())) st.insert("result", expr->type);

				std::shared_ptr<Type> resType = st.lookup(st.getParent())->type;
				if (resType->getType() == TYPE_PROC) yyerror("Procedure shouldn't have a result statement.");

				if (expr->type->getType() == TYPE_ARRAY) yyerror("Function cannot return an array.");
				if (expr->type->getType() != resType->getType()) yyerror("Type mismatch.");

				lval->type = expr->type;
			} else if (lval->type->getName().compare(expr->type->getName())) {
				if (!((lval->typeCheck(TYPE_REAL)) && (expr->typeCheck(TYPE_INTEGER)))) {
					std::string err = lval->getName() + " is of type " + lval->type->getName() + ".\n" + expr->getName() + " is of type " + expr->type->getName() + ".\n";
					yyerror(err);
				}
			}
		}

		virtual void igen() override {
			expr->igen();
			if (expr->typeCheck(TYPE_BOOLEAN)) {
				quadBACKPATCH(expr->quadTRUE, std::to_string(quadNEXTQUAD()));
				// for (const auto &v : expr->quadTRUE) std::cout << v << std::endl;
				quadGENQUAD(":=", "true", "-", lval->place);
				quadGENQUAD("jump", "-", "-", std::to_string(quadNEXTQUAD() + 2));
				quadBACKPATCH(expr->quadFALSE, std::to_string(quadNEXTQUAD()));
				quadGENQUAD(":=", "false", "-", lval->place);
			} else quadGENQUAD(":=", expr->place, "-", lval->place);
			quadNEXT = quadEMPTYLIST();
		}
};


class Return: public Stmt {
	public:
		Return() {};

		virtual void printAST(std::ostream &out) const override { out << "Return()"; }
		virtual std::string getName() const override { return "Return()"; }
		virtual void sem() override {  }
		virtual void igen() override { quadGENQUAD("ret", "-", "-", "-"); }
};


class New: public Stmt {
	private:
		std::unique_ptr<LVal> lVal;
		std::unique_ptr<Expr> expr;
	public:
		New(std::unique_ptr<LVal> l, std::unique_ptr<Expr> e=nullptr) : lVal(std::move(l)), expr(std::move(e)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "New(";
			if (lVal) lVal->printAST(out);
			if (expr) {
				out << ", ";
				expr->printAST(out);
			}
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "New(";
			res += lVal->getName();
			if (expr) {
				res += ", ";
				res += expr->getName();
			}
			res += ")";
			return res;
		}
		virtual void sem() override {
			lVal->sem();

			if (lVal->type->getType() == TYPE_RES) lVal->type = std::move(st.lookup("result")->type);
			if (lVal->type->getType() != TYPE_POINTER) yyerror("Expected pointer for new.");

			if (expr) {
				if (lVal->type->getPointerType()->getType() != TYPE_ARRAY) yyerror("Expected pointer to array");

				expr->sem();

				if (expr->type->getType() == TYPE_RES) expr->type = std::move(st.lookup("result")->type);
				if (expr->type->getType() != TYPE_INTEGER) yyerror("Expected integer.");	
			}

			st.makeNew(lVal->getName());
		}

		void igen() override {
			// lVal->igen(); // FIXME ??? idk if we want this or not
			if (expr) {
				expr->igen();
				int s = lVal->type->getSize();
				std::string W = "$" + std::to_string(quadNEWTEMP());
				quadGENQUAD("*", expr->place, std::to_string(s), W);
				quadGENQUAD("par", W, "V", "-");
			} else quadGENQUAD("par", std::to_string(lVal->type->getSize()), "V", "-");
			quadGENQUAD("par", lVal->place, "RET", "-");
			quadGENQUAD("call", "-", "-", "new");
			quadNEXT = quadEMPTYLIST();
		}
};


class Goto: public Stmt {
	private:
		std::unique_ptr<Id> id;
	public:
		Goto(std::unique_ptr<Id> c) : id(std::move(c)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Goto(";
			id->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Goto(";
			res += id->getName();
			res += ")";
			return res;
		}
		virtual void sem() override {
			std::string label = id->getId();
			if (!st.isLabel(label)) yyerror("Label Not Found.\n");
			else if (!st.validLabel(label)) yyerror("Label has no Statement.\n");
		}

		virtual void igen() override {
			quadGENQUAD("jumpl", "-", "-", id->getId());
			quadNEXT = quadEMPTYLIST();
		}
};


class Decl : public AST {
	private:
		std::unique_ptr<IdList> idList;
		std::shared_ptr<Type> type;
	public:
		Decl(std::unique_ptr<IdList> iL, std::shared_ptr<Type> t) : idList(std::move(iL)), type(t) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Declaration(";
			idList->printAST(out);
			out << ", ";
			type->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Declaration(";
			res += idList->getName();
			res += ", ";
			res += type->getName();
			res += ")";
			return res;
		}
		virtual void sem() override { for (const auto &id : idList->getList()) st.insert(id->getId(), type); }

		virtual void igen() override {  }
};


class DeclList : public AST {
	private:
		std::vector<std::unique_ptr<Decl>> decList;
	public:
		DeclList() : decList() {}

		void append(std::unique_ptr<Decl> d) { decList.push_back(std::move(d)); }
		virtual void printAST(std::ostream &out) const override {
			out << "DeclList(";
			bool start = true;

			for (const auto &d : decList) {
				if (!start) out << ", ";
				start = false;
				d->printAST(out);
			}
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "DeclList(";
			bool start = true;

			for (const auto &d : decList) {
				if (!start) res += ", ";
				start = false;
				res += d->getName();
			}
			res += ")";
			return res;
		}
		virtual void sem() override { for (auto &d : decList) d->sem(); }

		virtual void igen() override { for (auto &d : decList) d->igen(); /* just in case I change Decl */ }
};


class Header : public AST {
	public:
		virtual void semForward() {}
};


class Procedure : public Header {
	private:
		std::unique_ptr<Id> id;
		std::shared_ptr<FormalList> formalList;
	public:
		Procedure(std::unique_ptr<Id> i, std::shared_ptr<FormalList> fL = std::move(std::make_unique<FormalList>())) : id(std::move(i)), formalList(fL) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Procedure(";
			id->printAST(out);
			out << ", ";
			formalList->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Procedure(";
			res += id->getName();
			if (formalList) {
				res += ", ";
				res += formalList->getName();
			}
			res += ")";
			return res;
		}
		virtual void sem() override {
			if (st.forwarded(id->getId())) {
				std::string oldName="", newName="";
				std::shared_ptr<FormalList> oldFormals = st.getParams(id->getId());
				if (oldFormals) oldName = oldFormals->getName();
				if (!(formalList->getList().empty())) newName = formalList->getName();

				if(oldName.compare(newName)) yyerror("Procedure has two different declarations.");

				st.backward(id->getId());
				st.refreshFormals(id->getId(), oldFormals);
				st.insertParent(id->getId());
			} else st.insertFormal(id->getId(), std::make_unique<TypeProc>(), formalList);
			st.enterScope();
			formalList->sem();
		}
		virtual void semForward() override { st.insertFormalForward(id->getId(), std::make_unique<TypeProc>(), std::move(formalList)); }

		virtual void igen() {  }
};


class Function : public Header {
	private:
		std::unique_ptr<Id> id;
		std::shared_ptr<Type> type;
		std::shared_ptr<FormalList> formalList;
	public:
		Function(std::unique_ptr<Id> i, std::shared_ptr<Type> t, std::shared_ptr<FormalList> fL = std::move(std::make_unique<FormalList>())) : id(std::move(i)), type(t), formalList(std::move(fL)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Function(";
			id->printAST(out);
			out << ", type=";
			type->printAST(out);
			out << ", ";
			formalList->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Function(";
			res += id->getName();
			res += ", type=";
			res += type->getName();
			res += ", ";
			res += formalList->getName();
			res += ")";
			return res;
		}
		virtual void sem() override {
			formalList->sem();
			if (st.forwarded(id->getId())) {
				std::string oldName="", newName="";
				std::shared_ptr<FormalList> oldFormals = st.getParams(id->getId());
				if (oldFormals) oldName = oldFormals->getName();
				if (!(formalList->getList().empty())) newName = formalList->getName();

				if(oldName.compare(newName)) yyerror("Function has two different declarations.");

				st.backward(id->getId());
				st.refreshFormals(id->getId(), oldFormals);
				st.insertParent(id->getId());
			} else st.insertFormal(id->getId(), type, formalList);
		}
		virtual void semForward() override { st.insertFormalForward(id->getId(), type, formalList); }

		virtual void igen() override {  }
};


class Local : public AST {
	private:
		std::unique_ptr<DeclList> dList;
		std::unique_ptr<Label> lbl;
		std::unique_ptr<Header> hdr;
		std::unique_ptr<AST> body;
		std::string localType;
	public:
		Local(std::unique_ptr<DeclList> dL) : dList(std::move(dL)), localType("var") {}
		Local(std::unique_ptr<Label> l) : lbl(std::move(l)), localType("label") {}
		Local(std::unique_ptr<Header> h, std::unique_ptr<AST> b) : hdr(std::move(h)), body(std::move(b)), localType("forp") {}
		Local(std::unique_ptr<Header> h) : hdr(std::move(h)), localType("forward") {}

		virtual void printAST(std::ostream &out) const override {
			out << "Local(";
			if (localType.compare("var") == 0) dList->printAST(out);
			else if (localType.compare("label") == 0) lbl->printAST(out);
			else if (localType.compare("forp") == 0) {
				hdr->printAST(out);
				out << ", ";
				body->printAST(out);
			}
			else if (localType.compare("forward") == 0) hdr->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			if (localType.compare("var") == 0) return dList->getName();
			else if (localType.compare("label") == 0) return lbl->getName();
			else if (localType.compare("forp") == 0) return hdr->getName() + ", " + body->getName();
			else if (localType.compare("forward") == 0) return hdr->getName();
			return "";
		}
		virtual void sem() override {
			if (localType.compare("var") == 0) dList->sem();
			else if (localType.compare("label") == 0) lbl->sem();
			else if (localType.compare("forp") == 0) {
				// if(st.lookup("x"))  std::cout << st.lookup("x")->type->getType() << std::endl;
				// st.enterScope();
				hdr->sem();
				// std::cout << st.lookup("x")->type->getType() << std::endl;
				// st.printTopScope(std::cout);
				body->sem();
				// std::cout << st.lookup("x")->type->getType() << std::endl;
				st.exitScope();
			} else if (localType.compare("forward") == 0) hdr->semForward();
		}

		virtual void igen() override {
			if (localType.compare("var") == 0) dList->igen();
			else if (localType.compare("label") == 0) lbl->igen();
			else if (localType.compare("forp") == 0) {
				hdr->igen();
				body->igen();
			} else if (localType.compare("forward") == 0) hdr->igen();
		}
};


class LocalList : public AST {
	private:
		std::vector<std::unique_ptr<Local>> localList;
	public:
		LocalList(): localList() {}

		void append(std::unique_ptr<Local> l) { localList.push_back(std::move(l)); }
		virtual void printAST(std::ostream &out) const override {
			out << "LocalList(";
			bool start = true;
			for (const auto &l : localList) {
				if (!start) out << ", ";
				start = false;
				l->printAST(out);
			}
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "LocalList(";
			bool start = true;

			for (const auto &l : localList) {
				if (!start) res += ", ";
				start = false;
				res += l->getName();
			}
			res += ")";
			return res;
		}
		virtual void sem() override { for (auto &l : localList) l->sem(); }

		virtual void igen() override { for (auto &l : localList) l->igen(); }
};


class Body : public Stmt {
	private:
		std::unique_ptr<LocalList> localList;
		std::unique_ptr<Block> block;
		std::string name;
	public:
		Body(std::unique_ptr<LocalList> l, std::unique_ptr<Block> b) : localList(std::move(l)), block(std::move(b)), name("") {}
		Body(std::unique_ptr<LocalList> l, std::unique_ptr<Block> b, std::string s) : localList(std::move(l)), block(std::move(b)), name(s) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Body(";
			localList->printAST(out);
			out << ", ";
			block->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Body(";
			res += localList->getName();
			res += ", ";
			res += block->getName();
			res +=")";
			return res;
		}
		void setName(std::string s) { name = s; }
		std::string getBodyName() { return name; }
		virtual void sem() override {
			if (name.compare("") == 0) name = st.getParent(); 

			// st.enterScope();
			localList->sem();
			block->sem();
			// st.exitScope();
		}

		virtual void igen() override {
			localList->igen();
			quadGENQUAD("unit", name, "-", "-");
			block->igen();
			quadBACKPATCH(block->quadNEXT, std::to_string(quadNEXTQUAD()));
			quadGENQUAD("endu", name, "-", "-");
		}
};


class Reference : public RVal {
	private:
		std::unique_ptr<LVal> lVal;
	public:
		Reference(std::unique_ptr<LVal> l) : lVal(std::move(l)) {}

		virtual void printAST(std::ostream &out) const override {
			out << "Reference(";
			lVal->printAST(out);
			out << ")";
		}
		virtual std::string getName() const override {
			std::string res = "Reference(";
			res += lVal->getName();
			res +=")";
			return res;
		}
		virtual void sem() override {
			lVal->sem();
			if (lVal->typeCheck(TYPE_RES)) lVal->type = std::move(st.lookup("result")->type);
			type = std::make_shared<Pointer>(std::move(lVal->type));

			place = quadADDRESSOF(lVal->place);
		}

		virtual void igen() override {  }
};


class Result : public LVal {
	private:
	public:
		Result() {}

		virtual bool isRes() override { return true; }

		virtual void printAST(std::ostream &out) const override { out << "Result()"; }
		virtual std::string getName() const override { return "Result()"; }
		virtual void sem() override {
			type = std::make_shared<TypeRes>();

			place = "$$";
		}

		virtual void igen() override {  }
};


class Deref : public LVal {
	private:
		std::unique_ptr<Expr> expr;
	public:
		Deref(std::unique_ptr<Expr> e) : expr(std::move(e)) {}

		virtual void printAST(std::ostream &out) const override { out << "Deref(" << *expr << ")"; }
		virtual std::string getName() const override { return "Deref(" + expr->getName() + ")"; }
		virtual void sem() override {
			expr->sem();
			if (expr->typeCheck(TYPE_RES)) expr->type = std::move(st.lookup("result")->type);
			if (!(expr->typeCheck(TYPE_POINTER))) yyerror("Expression is not a pointer.");
			type = expr->type->getPointerType();

			std::string W = "$" + quadNEWTEMP();
			quadGENQUAD(":=", expr->place, "-", W);
			place ="[" + W + "]";
		}

		virtual void igen() override {
			expr->igen();
			std::string W ="$" + quadNEWTEMP();
			quadGENQUAD(":=", expr->place, "-", W);
			place = "[" + W + "]";
		}
};


#endif
