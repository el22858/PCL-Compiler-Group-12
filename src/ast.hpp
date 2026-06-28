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
		int offset, depth;
		bool hasReal;
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

		int getOff() { return offset; }
		int getDepth() { return depth; }
		virtual bool isRVal() const { return true; }
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
			} else yyerror("Expected boolean for ITE. Instead got " + expr->getName() + ", which is a(n)" + expr->type->getName() + ".");
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
			else yyerror("Expected boolean for While loop. Instead got " + expr->getName() + ", which is a(n) " + expr->type->getName() + ".");
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
		virtual bool isRVal() const override { return false; }
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
			hasReal = false;

			if (op.compare("+") == 0) {
				if (expr->typeCheck(TYPE_INTEGER)) type = std::make_shared<Integer>();
				else if (expr->typeCheck(TYPE_REAL)) {
					type = std::make_shared<Real>();
					hasReal = true;
				}
				else yyerror("Expected number after plus sign. Instead got " + expr->getName() + ", which is a(n) " + expr->type->getName() + ".");
				place = expr->place;
			} else if (op.compare("-") == 0) {
				if (expr->typeCheck(TYPE_INTEGER)) type = std::make_shared<Integer>();
				else if (expr->typeCheck(TYPE_REAL)) {
					type = std::make_shared<Real>();
					hasReal = true;
				}
				else yyerror("Expected number after minus sign. Instead got " + expr->getName() + ", which is a(n) " + expr->type->getName());
				place = "$" + std::to_string(quadNEWTEMP());
				depth = st.getDepth();
				offset = st.addTemp(type->getSize());
			} else if (op.compare("not") == 0) {
				if (expr->typeCheck(TYPE_BOOLEAN)) type = std::make_shared<Boolean>();
				else yyerror("Expected boolean after 'not' operation. Instead got " + expr->getName() + ", which is a(n) " + expr->type->getName() + ".");
			} else yyerror("Compiler error: unreachable flow control point reached.");
		}

		virtual void igen() override {
			expr->igen();

			if (op.compare("-") == 0) quadGENQUAD(op, "0", expr->place, place, hasReal, 0, expr->getDepth(), getDepth(), 0, expr->getOff(), getOff(), 0, expr->type->getSize(), type->getSize());
			else if (op.compare("not") == 0) {
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
			hasReal = false;

			if ((op.compare("+") == 0) || (op.compare("-") == 0) || (op.compare("*") == 0)) {
				expr1->sem();
				expr2->sem();
				if (expr1->typeCheck(TYPE_INTEGER)) {
					if (expr2->typeCheck(TYPE_INTEGER)) type = std::make_shared<Integer>();
					else if (expr2->typeCheck(TYPE_REAL)) {
						type = std::make_shared<Real>();

						hasReal = true;
					}
					else yyerror("Expected two numbers. Instead got " + expr2->getName() + ", which is a(n) " + expr2->type->getName() + ".");
				} else if (expr1->typeCheck(TYPE_REAL)) {
					if ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL))) type = std::make_shared<Real>();
					else yyerror("Expected two numbers. Instead got " + expr2->getName() + ", which is a(n) " + expr2->type->getName() + ".");
					
					hasReal = true;
				} else {
					std::string opName;
					if (op.compare("+") == 0) opName = "addition";
					else if (op.compare("-") == 0) opName = "subtraction";
					else if (op.compare("*") == 0) opName = "multiplication";
					else yyerror("Compiler error: unreachable flow control point reached.");

					if ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL))) yyerror("Expected two integers for " + opName + ". Instead got " + expr1->getName() + ", which is a(n) " + expr1->type->getName() + ", and " + expr2->getName() + ", which is a(n) " + expr2->type->getName() + ".");
					else yyerror("Expected integer for " + opName + ". Instead got " + expr1->getName() + ", which is a(n) " + expr1->type->getName() + ".");
				}
				place = "$" + std::to_string(quadNEWTEMP());
				depth = st.getDepth();
				offset = st.addTemp(type->getSize());
			} else if (op.compare("/") == 0) {
				expr1->sem();
				expr2->sem();
				place = "$" + std::to_string(quadNEWTEMP());
				depth = st.getDepth();
				offset = st.addTemp(type->getSize());
				if (((expr1->typeCheck(TYPE_INTEGER)) || ((expr1->typeCheck(TYPE_REAL)))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = std::make_shared<Real>();
				else yyerror("Expected two numbers for division. Instead got " + expr1->getName() + ", which is a(n) " + expr1->type->getName() + ", and " + expr2->getName() + ", which is a(n) " + expr2->type->getName() + ".");

				if (expr1->typeCheck(TYPE_REAL) || expr2->typeCheck(TYPE_REAL)) hasReal  = true;

				place = "$" + std::to_string(quadNEWTEMP());
				depth = st.getDepth();
				offset = st.addTemp(type->getSize());
			} else if ((op.compare("div") == 0) || (op.compare("mod") == 0)) {
				expr1->sem();
				expr2->sem();
				if ((expr1->typeCheck(TYPE_INTEGER)) && (expr2->typeCheck(TYPE_INTEGER))) type = std::make_shared<Integer>();
				else yyerror("Expected two integers for division or modulo. Instead got " + expr1->getName() + ", which is a(n) " + expr1->type->getName() + ", and " + expr2->getName() + ", which is a(n) " + expr2->type->getName() + ".");
				
				place = "$" + std::to_string(quadNEWTEMP());
				depth = st.getDepth();
				offset = st.addTemp(type->getSize());
			} else if ((op.compare("and") == 0) || (op.compare("or") == 0)) {
				expr1->sem();
				if (!expr1->typeCheck(TYPE_BOOLEAN)) yyerror("Expected boolean for '" + op + "' operation. Instead got " + expr1->getName() + ", which is a(n) " + expr1->type->getName() + ".");

				expr2->sem();
				if (!expr2->typeCheck(TYPE_BOOLEAN)) yyerror("Expected boolean for '" + op + "' operation. Instead got " + expr2->getName() + ", which is a(n) " + expr2->type->getName() + ".");
				type = std::make_shared<Boolean>();
			} else if ((op.compare("<") == 0) || (op.compare(">") == 0) || (op.compare("<=") == 0) || (op.compare(">=") == 0)) {
				expr1->sem();
				expr2->sem();
				if (((expr1->typeCheck(TYPE_INTEGER)) || (expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = std::make_shared<Boolean>();
				else yyerror("Expected two numbers for comparison. Instead got " + expr1->getName() + ", which is a(n) " + expr1->type->getName() + ", and " + expr2->getName() + ", which is a(n) " + expr2->type->getName() + ".");

				if (expr1->typeCheck(TYPE_REAL) || expr2->typeCheck(TYPE_REAL)) hasReal  = true;
			} else if ((op.compare("=") == 0) || (op.compare("<>") == 0)) {
				expr1->sem();
				expr2->sem();
				if (expr1->type->getName().compare(expr2->type->getName()) == 0) type = std::make_shared<Boolean>();
				else yyerror("Type mismatch for equality/inequality check! The first operand is " + expr1->getName() + ", which is a(n) " + expr1->type->getName() + ", while the second operand is " + expr2->getName() + ", which is a(n) " + expr2->type->getName() + ".");

				if (expr1->typeCheck(TYPE_REAL) || (expr2->typeCheck(TYPE_REAL))) hasReal  = true;
			}
		}

		virtual void igen() override {
			if ((op.compare("+") == 0) || (op.compare("-") == 0) || (op.compare("*") == 0) || (op.compare("/") == 0) || (op.compare("div") == 0) || (op.compare("mod") == 0)) {
				expr1->igen();
				expr2->igen();
				quadGENQUAD(op, expr1->place, expr2->place, place, hasReal, expr1->getDepth(), expr2->getDepth(), getDepth(), expr1->getOff(), expr2->getOff(), getOff(), expr1->type->getSize(), expr2->type->getSize(), type->getSize());
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
				quadGENQUAD(op, expr1->place, expr2->place, "*", hasReal, expr1->getDepth(), expr2->getDepth(), 0, expr1->getOff(), expr2->getOff(), 0, expr1->type->getSize(), expr2->type->getSize());
				quadFALSE = quadMAKELIST(quadNEXTQUAD());
				quadGENQUAD("jump", "-", "-", "*");
			} else if ((op.compare("=") == 0) || (op.compare("<>") == 0)) {
				expr1->igen();
				expr2->igen();

				quadTRUE = quadMAKELIST(quadNEXTQUAD());
				quadGENQUAD(op, expr1->place, expr2->place, "*", hasReal, expr1->getDepth(), expr2->getDepth(), 0, expr1->getOff(), expr2->getOff(), 0, expr1->type->getSize(), expr2->type->getSize());
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
		virtual void sem() override {
			type = std::make_shared<Integer>();
			
			place = std::to_string(val);
			depth = 0;
			offset = 0;
		}

		virtual void igen() override {}
};


class BoolConst : public RVal {
	private:
		bool val;
	public:
		BoolConst(bool b) : val(b) {}

		virtual void printAST(std::ostream &out) const override { out << "BoolConst('" << val << "')"; }
		virtual std::string getName() const override { return (val) ? "BoolConst(true)" : "BoolConst(false)" ; }
		virtual void sem() override {
			type = std::make_shared<Boolean>();
			
			place = (val) ? "true" : "false";
		}

		virtual void igen() override { quadGENBOOL(); }
};


class RealConst : public RVal {
	private:
		float val;
	public:
		RealConst(float f) : val(f) {}

		virtual void printAST(std::ostream &out) const override { out << "RealConst(" << val << ")"; }
		virtual std::string getName() const override { return "RealConst(" + std::to_string(val) + ")"; }
		virtual void sem() override {
			type = std::make_shared<Real>();
		
			place = std::to_string(val);
			depth = 0;
			offset = 0;
		}

		virtual void igen() override {}
};

class CharConst : public RVal {
	private:
		char *val;
	public:
		CharConst(char *c) : val(c) {}

		virtual void printAST(std::ostream &out) const override {out << "CharConst(" << val << ")"; }
		virtual std::string getName() const override { return "CharConst(" + std::string(val) + ")"; }
		virtual void sem() override {
			type = std::make_shared<Char>();

			place = val;
			depth = 0;
			offset = 0;
		}

		virtual void igen() override {}
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
			depth = 0;
			offset = 0;
		}

		virtual void igen() override {}
};

class NilRVal : public RVal {
	private:
	public:
		NilRVal() {}

		virtual void printAST(std::ostream &out) const override { out << "Nil()";}
		virtual std::string getName() const override { return "Nil()"; }
		virtual void sem() override {
			type = std::make_shared<TypeNil>();
		
			place = "nil";
			depth = 0;
			offset = 0;
		}

		virtual void igen() override { place = "nil"; }
};


class StringLit : public LVal {
	private:
		std::string val;
	public:
		StringLit(std::string s) : val(s) {}

		virtual void printAST(std::ostream &out) const override { out << "StringLit(" << val << ")"; }
		virtual std::string getName() const override { return "StringLit(" + val + ")"; } 
		virtual void sem() override {
			type = std::make_shared<Array>(std::move(std::make_unique<Char>()), val.size());
		
			place = val;
			depth = 0;
			offset = 0;
		}

		virtual void igen() {}
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
			depth = e->n;
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
		std::shared_ptr<Stmt> stmt;
		int depth;
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
			if (!st.isLabel(c)) yyerror(c + "is used as a label, but there is no label with that name, in the scope that it's used in.");
			else {
				stmt->sem();
				st.insertLabelStmt(c, stmt);
			}

			depth = st.getDepth();
		}

		virtual void igen() override {
			quadGENQUAD("label", id->getId(), "-", "-", depth);
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

			if (!(lVal->typeCheck(TYPE_ARRAY))) yyerror(lVal->getName() + " should be an array, but it is not. Instead it's a(n) " + lVal->type->getName() + ", in the scope that it's being used in.");
			else if (!(expr->typeCheck(TYPE_INTEGER))) yyerror("Expected integer for indexing in the array " + lVal->getName() + ". Instead got " + expr->getName() + ", which is a(n) " + expr->type->getName() + ".");

			type = lVal->type->getArrayType();
			place = "$" + std::to_string(quadNEWTEMP());
			depth = st.getDepth();
			offset = st.getOff();
		}

		virtual void igen() override {
			lVal->igen();
			expr->igen();

			quadGENQUAD("array", lVal->place, expr->place, place, false, 0, 0, 0, 0, 0, 0, type->getArrayType()->getSize());
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
		virtual void sem() override { for (const auto &id : idList->getList()) st.insert(id->getId(), type, true); }

		virtual void igen() override {}
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
		bool isProc, lib, retReal;
		std::string place;
		int offset, size;
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
			if (!st.isFormal(funcName)) yyerror(funcName + " is not a callable function or procedure, in the scope that it's being used in.");

			fL = st.getParams(funcName);

			int i = 0, expArgs = 0, actArgs = 0;
			if (!(fL->isEmpty())) {
				for (const auto &f : fL->getList()) expArgs += f->getIdList().size();
			}
			if (func) actArgs = func->getList().size();
			if (expArgs != actArgs) yyerror("Incorrect number of arguments given for function or procedure '" + funcName + "'. Got " + std::to_string(actArgs) + " real parameters, but was expecting " + std::to_string(expArgs) + ".");

			if (!(fL->isEmpty())) {
				for (const auto &f : fL->getList()) {
					int k = f->getIdList().size();
					for (int j=0; j<k; j++) {
						// if ((f->getType()).compare(func->getList()[i]->type->getNameNoSize()) && (!(((f->getType()).compare(func->getList()[i]->type->getNameNoSize()) == 0) || (((f->getType().compare("Real()")==0) && (func->getList()[i]->typeCheck(TYPE_INTEGER))))))) yyerror("Type mismatch.");
						if ((f->getType()).compare(func->getList()[i]->type->getNameNoSize())) {
							if ((f->getType().compare("Real") == 0) && func->getList()[i]->typeCheck(TYPE_INTEGER));
							else {
								std::string errMsg = "Type mismatch. Expected " + f->getType() + " for " + funcName + ", but got a(n) " + func->getList()[i]->type->getNameNoSize() + " instead, in the form of " + func->getList()[i]->getName() + ".";
								yyerror(errMsg);
							}
						}
						if ((f->quadPARAMMODE().compare("R") == 0) && (func->getList()[i]->isRVal())) yyerror("Function or Procedure '" + funcName + "' expects a call-by-reference parameter, but an RVal was given instead, in the form of " + func->getList()[i]->getName() + ".");
						++i;
					}
				}
			}

			isProc = (st.lookup(id->getId())->type->getType() == TYPE_PROC);
			lib = st.isLib(id->getId());
			retReal = (st.lookup(id->getId())->type->getType() == TYPE_REAL);

			depth = st.getDepth();
			offset = st.getOff();
			if (!isProc) {
				size = st.lookup(id->getId())->type->getSize();
				st.addTemp(size);
			}

			st.refreshFormals(funcName, fL);
		}

		virtual void igen() override {
			int i = 0, initOffset = offset;
			if (func) func->igen();

			if (!fL->isEmpty()) {
				for (const auto &f : fL->getList()) {
					const auto &tmp = func->getList()[i];

					if (tmp->typeCheck(TYPE_BOOLEAN)) {
						std::string W = "$" + std::to_string(quadNEWTEMP());
						quadBACKPATCH(tmp->quadTRUE, std::to_string(quadNEXTQUAD()));
						quadGENQUAD(":=", "true", "-", W, false, 0, 0, depth, 0, 0, --offset, 0, 0, 1);
						quadGENQUAD("jump", "-", "-", std::to_string(quadNEXTQUAD() + 2));

						quadBACKPATCH(tmp->quadFALSE, std::to_string(quadNEXTQUAD()));
						quadGENQUAD(":=", "false", "-", W, false, 0, 0, depth, 0, 0, offset, 0, 0, 1);
						tmp->place = W;
					}

					if ((f->quadPARAMMODE().compare("V")==0) && (f->getType().compare(tmp->type->getName()) && (((f->getType()).compare(tmp->type->getNameNoSize()) == 0) || (((f->getType().compare("Real()")==0) && (tmp->typeCheck(TYPE_INTEGER))))))){
						std::string W = "$" + std::to_string(quadNEWTEMP());
						offset -= tmp->type->getSize();
						quadGENQUAD(":=", tmp->place, "-", W, f->getType().compare("Real()") == 0, tmp->getDepth(), 0, depth, tmp->getOff(), offset, tmp->type->getSize(), 0, (f->getType().compare("Real()") ? tmp->type->getSize() : 8));
						quadGENQUAD("par", W, "V", "-", false, tmp->getDepth(), 0, 0, tmp->getOff(), 0, 0, tmp->type->getSize());
					} else quadGENQUAD("par", tmp->place, f->quadPARAMMODE(), "-", false, tmp->getDepth(), 0, 0, tmp->getOff(), 0, 0, tmp->type->getSize());

					++i;
				}
			}

			if (!isProc) {
                std::string W = "$" + std::to_string(quadNEWTEMP());
                quadGENQUAD("par", W, "RET", "-", retReal, depth, 0, 0, initOffset + 6, 0, 0, size, 0, 0);
            }

			quadGENQUAD("call", "-", "-", id->getId(), lib); /* ??? */
			quadNEXT = quadEMPTYLIST();
		}
};


class CallRVal : public RVal {
	private:
		std::unique_ptr<Id> id;
		std::unique_ptr<ExprList> func;
		std::shared_ptr<FormalList> fL;
		bool lib, isProc, retReal;

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
			if (!st.isFormal(funcName)) yyerror(funcName + " is not a callable function or procedure, in the scope that it's being used in.");

			fL = st.getParams(funcName);

			int i = 0, expArgs = 0, actArgs = 0;
			if (!(fL->isEmpty())) {
				for (const auto &f : fL->getList()) expArgs += f->getIdList().size();
			}
			if (func) actArgs = func->getList().size();
			if (expArgs != actArgs) yyerror("Incorrect number of arguments given for function or procedure '" + funcName + "'. Got " + std::to_string(actArgs) + " real parameters, but was expecting " + std::to_string(expArgs) + ".");

			if (!(fL->isEmpty())) {
				for (const auto &f : fL->getList()) {
					int k = f->getIdList().size();
					for (int j=0; j<k; j++) {
						if ((f->getType()).compare(func->getList()[i]->type->getNameNoSize())) {
							if ((f->getType().compare("Real") == 0) && func->getList()[i]->typeCheck(TYPE_INTEGER));
							else {
								std::string errMsg = "Type mismatch. Expected " + f->getType() + " for " + funcName + ", but got a(n) " + func->getList()[i]->type->getNameNoSize() + " instead, in the form of " + func->getList()[i]->getName() + ".";
								yyerror(errMsg);
							}
						}
						if ((f->quadPARAMMODE().compare("R") == 0) && (func->getList()[i]->isRVal())) yyerror("Function or Procedure '" + funcName + "' expects a call-by-reference parameter, but an RVal was given instead, in the form of " + func->getList()[i]->getName() + ".");
						++i;
					}
				}
			}


			type = st.lookup(funcName)->type;
			isProc = (type->getType() == TYPE_PROC);
			lib = st.isLib(id->getId());
			retReal = (st.lookup(id->getId())->type->getType() == TYPE_REAL);

			depth = st.getDepth();
			offset = st.getOff();
			if (!isProc) {
				size = st.lookup(id->getId())->type->getSize();
				st.addTemp(size);
			}

			st.refreshFormals(funcName, fL);
		}

		virtual void igen() override {
			int i = 0, initOffset = offset;

			if (func) func->igen();

			if (!fL->isEmpty()) {
				for (const auto &f : fL->getList()) {
					const auto &tmp = func->getList()[i];

					if (tmp->typeCheck(TYPE_BOOLEAN)) {
						std::string W = "$" + std::to_string(quadNEWTEMP()); /* ??? */
						quadBACKPATCH(tmp->quadTRUE, std::to_string(quadNEXTQUAD()));
						quadGENQUAD(":=", "true", "-", W);
						quadGENQUAD("jump", "-", "-", std::to_string(quadNEXTQUAD() + 2));

						quadBACKPATCH(tmp->quadFALSE, std::to_string(quadNEXTQUAD()));
						quadGENQUAD(":=", "false", "-", W);
						tmp->place = W;
					}

					if ((f->quadPARAMMODE().compare("V")==0) && (f->getType().compare(tmp->type->getName()) && (((f->getType()).compare(tmp->type->getNameNoSize()) == 0) || (((f->getType().compare("Real()")==0) && (tmp->typeCheck(TYPE_INTEGER))))))){
						std::string W = "$" + std::to_string(quadNEWTEMP()); /* ??? */
						quadGENQUAD(":=", tmp->place, "-", W);
						quadGENQUAD("par", W, "V", "-", false, tmp->getDepth(), 0, 0, tmp->getOff(), 0, 0, tmp->type->getSize());
					} else quadGENQUAD("par", tmp->place, f->quadPARAMMODE(), "-", false, tmp->getDepth(), 0, 0, tmp->getOff(), 0, 0, tmp->type->getSize());

					++i;
				}
			}

			// quadGENQUAD("call", "-", "-", id->getId());
			place = "$" + std::to_string(quadNEWTEMP()); /* ??? */
			quadGENQUAD("par", place, "RET", "-", retReal, depth, 0, 0, initOffset + 6, 0, 0, size, 0, 0);
			quadGENQUAD("call", "-", "-", id->getId(), lib, id->getDepth(), 0, 0, 0, 0, 0); /* ??? */
		}
};


class Dispose: public Stmt {
	private:
		std::unique_ptr<LVal> lVal;
		std::string oldPlace;
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
			if (!(lVal->typeCheck(TYPE_POINTER))) yyerror("Expected pointer for disposal. Instead got " + lVal->getName() + ", which is a(n) " + lVal->type->getName() + ".");
			if (!st.isNew(lVal->getName())) yyerror("Cannot dispose of non-new value.");

			if (bracket && (lVal->type->getPointerType()->getType() != TYPE_ARRAY)) yyerror("Pointer " + lVal->getName() + " scheduled for disposal is used with brackets, despite not being a pointer to an array.");

			oldPlace = lVal->place;
			lVal = std::move(std::make_unique<NilLVal>());
		}

		virtual void igen() override {
			quadGENQUAD("par", oldPlace, "R", "-");
			quadGENQUAD("call", "-", "-", "dispose", true);
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
		bool withReal;
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
				if (resType->getType() == TYPE_PROC) yyerror("Procedure" + resType->getName() + "shouldn't have a result statement.");

				if (expr->type->getType() == TYPE_ARRAY) yyerror("Functions cannot return an array. " + expr->getName() + " shouldn't be used this way.");
				if (expr->type->getType() != resType->getType()) yyerror("Type mismatch for assignment, between " + expr->getName() + " and " + resType->getName() +".");

				lval->type = expr->type;
			} else if (lval->type->getName().compare(expr->type->getName())) {
				if (!((lval->typeCheck(TYPE_REAL)) && (expr->typeCheck(TYPE_INTEGER)))) {
					std::string err = lval->getName() + " is of type " + lval->type->getName() + ".\n" + expr->getName() + " is of type " + expr->type->getName() + ".\n";
					yyerror(err);
				}
			}

			if ((lval->typeCheck(TYPE_REAL)) || (expr->typeCheck(TYPE_REAL))) withReal = true;
			else withReal = false;
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
			} else quadGENQUAD(":=", expr->place, "-", lval->place, withReal, expr->getDepth(), 0, lval->getDepth(), expr->getOff(), 0, lval->getOff(), expr->type->getSize(), 0, lval->type->getSize());
			quadNEXT = quadEMPTYLIST();
		}
};


class Return: public Stmt {
	public:
		Return() {};

		virtual void printAST(std::ostream &out) const override { out << "Return()"; }
		virtual std::string getName() const override { return "Return()"; }
		virtual void sem() override {  }
		virtual void igen() override { quadGENQUAD("ret", "-", "-", "-"); } /* ??? */
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
			if (lVal->type->getType() != TYPE_POINTER) yyerror("Expected pointer for new. Instead got " + lVal->getName() + ", which is a(n) " + lVal->type->getName() + ".");

			if (expr) {
				if (lVal->type->getPointerType()->getType() != TYPE_ARRAY) yyerror("Expected pointer to array. Instead got " + lVal->getName() + ", which is a pointer to " + lVal->type->getPointerType()->getName() + ".");

				expr->sem();

				if (expr->type->getType() == TYPE_RES) expr->type = std::move(st.lookup("result")->type);
				if (expr->type->getType() != TYPE_INTEGER) yyerror("Expected integer for new pointer size. Instead got " + expr->getName() + ", which is a(n) " + expr->type->getName() + ".");	
			}

			st.makeNew(lVal->getName());
		}

		void igen() override {
			// lVal->igen(); // FIXME ??? idk if we want this or not
			if (expr) {
				expr->igen();
				int s = lVal->type->getSize();
				std::string W = "$" + std::to_string(quadNEWTEMP()); /* ??? */
				quadGENQUAD("*", expr->place, std::to_string(s), W);
				quadGENQUAD("par", W, "V", "-", expr->getDepth(), 0, 0, expr->getOff(), 0, 0, expr->type->getSize());
			} else quadGENQUAD("par", std::to_string(lVal->type->getSize()), "V", "-", 0, 0, 0, 0, 0, 0, 0, 4);
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
			if (!st.isLabel(label)) yyerror("Label " + label + " not found.\n");
			else if (!st.validLabel(label)) yyerror("Label " + label + " is being used with no statement to match.\n"); // ???
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
	protected:
		std::unique_ptr<Id> id;
		int size, idNum;
	public:
		virtual void semForward() {}
		void setSize(int n) { size = n; }
		void setId(int n) { idNum = n; }
};


class Procedure : public Header {
	private:
		std::shared_ptr<FormalList> formalList;
	public:
		Procedure(std::unique_ptr<Id> i, std::shared_ptr<FormalList> fL = std::move(std::make_unique<FormalList>())) : formalList(fL) { id = std::make_unique<Id>(i->getId()); }

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

				if(oldName.compare(newName)) yyerror("Procedure " + id->getId() + " has two different declarations.");

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
		std::shared_ptr<Type> type;
		std::shared_ptr<FormalList> formalList;
	public:
		Function(std::unique_ptr<Id> i, std::shared_ptr<Type> t, std::shared_ptr<FormalList> fL = std::move(std::make_unique<FormalList>())) : type(t), formalList(std::move(fL)) { id = std::make_unique<Id>(i->getId()); }

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

				if(oldName.compare(newName)) yyerror("Function " + id->getId() + " has two different declarations.");

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
				body->setSize(st.getOff());
				body->setDepth(st.getDepth());
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


extern int funcNum;

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
			quadGENQUAD("unit", name, "-", "-", false, depth, 0, 0, funcNum, 0, 0, size);
			block->igen();
			quadBACKPATCH(block->quadNEXT, std::to_string(quadNEXTQUAD()));
			quadGENQUAD("endu", name, "-", "-", false, depth, 0, 0, funcNum++, 0, 0, size);
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
			depth = st.getDepth();
			offset = 6;
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
			if (!(expr->typeCheck(TYPE_POINTER))) yyerror("Expression " + expr->getName() + " is not a pointer, and thus cannot be dereferenced.");
			type = expr->type->getPointerType();

			std::string W = "$" + quadNEWTEMP();
			depth = st.getDepth();
			offset = st.getOff();
			quadGENQUAD(":=", expr->place, "-", W);
			place ="[" + W + "]";
		}

		virtual void igen() override {
			expr->igen();
			std::string W ="$" + quadNEWTEMP(); /* ??? */
			place = "[" + W + "]";
			depth = st.getDepth();
			offset = st.addTemp(type->getSize());
			quadGENQUAD(":=", expr->place, "-", W, false, expr->getDepth(), 0, getDepth(), expr->getOff(), 0, getOff(), expr->type->getSize(), 0, type->getSize());
		}
};


#endif
