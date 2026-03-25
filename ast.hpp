#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <cstring>
#include <vector>

#include "tree.hpp"
#include "type.hpp"
#include "symbol.hpp"


class Stmt : public AST {
    private:
    public:
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
                    if (x) out << *x;
                }
            }
            out << ")";
        }
        virtual void sem() override { for (auto &s : stmt_list) s->sem(); }

        void append(std::unique_ptr<Stmt> s) { stmt_list.insert(stmt_list.begin(), std::move(s)); }
};


class Expr : public AST {
    private:
    public:
        Types type;
        bool typeCheck(Types t) { return type == t; }
};


class ExprList : public Expr {
    private:
        std::vector<std::unique_ptr<Expr>> expr_list;
    public:
        ExprList() : expr_list() {}

        void append(std::unique_ptr<Expr> e) { expr_list.insert(expr_list.begin(), std::move(e)); }
        virtual void printAST(std::ostream &out) const override {
            out << "ExpressionList(";
            bool start = true;

            for (const auto &x : expr_list) {
                if (!start) out << ", ";
                start = false;
                out << *x;
            }
            out << ")";
        }

        virtual void sem() override { for (auto &e : expr_list) e->sem(); }
};


class Block : public Stmt {
    private:
        std::unique_ptr<StmtList> stmt_list;
    public:
        Block(std::unique_ptr<StmtList> sL = nullptr) : stmt_list(std::move(sL)) {}

        virtual void printAST(std::ostream &out) const override { out << "Block(" << *stmt_list << ")"; }
        virtual void sem() override { stmt_list->sem(); }
};


class ITE : public Stmt {
    private:
        std::unique_ptr<Expr> expr;
        std::unique_ptr<Stmt> stmt1, stmt2;
    public:
        ITE(std::unique_ptr<Expr> e, std::unique_ptr<Stmt> s1, std::unique_ptr<Stmt> s2 = nullptr) : expr(std::move(e)), stmt1(std::move(s1)), stmt2(std::move(s2)) {}

        virtual void printAST(std::ostream &out) const override {
            out << "If(" << *expr << ", " << *stmt1;
            if (stmt2 != nullptr) out << ", " << *stmt2;
            out << ")";
        }
        virtual void sem() override {
            expr->sem();
            if (expr->typeCheck(TYPE_BOOLEAN)) {
                stmt1->sem();
                if (stmt2) stmt2->sem();
            } // else yyerror("Expected boolean.");
        }
};


class While : public Stmt {
    private:
        std::unique_ptr<Expr> expr;
        std::unique_ptr<Stmt> stmt;
    public:
        While(std::unique_ptr<Expr> e, std::unique_ptr<Stmt> s) : expr(std::move(e)), stmt(std::move(s)) {}

        virtual void printAST(std::ostream &out) const override { out << "While(" << *expr << ", " << *stmt << ")"; }
        virtual void sem() override {
            expr->sem();

            if (expr->typeCheck(TYPE_BOOLEAN)) stmt->sem();
            // else yyerror("Expected boolean.");
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
        char *op;
        std::unique_ptr<Expr> expr;
    public:
        UnOp(char *s, std::unique_ptr<Expr> e) : op(s), expr(std::move(e)) {}

        virtual void printAST(std::ostream &out) const override { out << "UnOp(" << op << ", " << *expr << ")"; }
        virtual void sem() override {
            expr->sem();

            if ((strcmp(op, "+") == 0) || (strcmp(op, "-") == 0)) {
                if (expr->typeCheck(TYPE_INTEGER)) type = TYPE_INTEGER;
                else if (expr->typeCheck(TYPE_REAL)) type = TYPE_REAL;
                // else yyerror("Expected number");
            } else if (strcmp(op, "not") == 0) {
                if (expr->typeCheck(TYPE_BOOLEAN)) type = TYPE_BOOLEAN;
                // else yyerror("Expected boolean.");
            } // else yyerror("Unary Operator not recognized"); // Trash
        }
};


class BinOp : public RVal {
    private:
        std::unique_ptr<Expr> expr1, expr2;
        char *op;
    public:
        BinOp(std::unique_ptr<Expr> e1, char *s, std::unique_ptr<Expr> e2) : expr1(std::move(e1)), op(s), expr2(std::move(e2)) {}

        virtual void printAST(std::ostream &out) const override { out << "BinOp(" << *expr1 << ", " << op << ", " << *expr2 << ")"; }
        virtual void sem() override {
            expr1->sem();
            expr2->sem();

            if ((strcmp(op, "+") == 0) || (strcmp(op, "-") == 0) || (strcmp(op, "*") == 0)) {
                if (expr1->typeCheck(TYPE_INTEGER)) {
                    if (expr2->typeCheck(TYPE_INTEGER)) type = TYPE_INTEGER;
                    else if (expr2->typeCheck(TYPE_REAL)) type = TYPE_REAL;
                    // else yyerror("Expected int or real.");
                } else if (expr1->typeCheck(TYPE_REAL)) {
                    if ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL))) type = TYPE_REAL;
                    // else yyerror("Expected int or real.");
                }
            } else if (strcmp(op, "/")) {
                if ((expr1->typeCheck(TYPE_INTEGER)) || ((expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = TYPE_REAL;
                // else yyerror("Expected int or real.");
            } else if ((strcmp(op, "div") == 0) || (strcmp(op, "mod") == 0)) {
                if ((expr1->typeCheck(TYPE_INTEGER)) && (expr2->typeCheck(TYPE_INTEGER))) type = TYPE_INTEGER;
                // else yyerror("Expected int.");
            } else if ((strcmp(op, "and") == 0) || (strcmp(op, "or") == 0)) {
                if ((expr1->typeCheck(TYPE_BOOLEAN) && (expr2->typeCheck(TYPE_BOOLEAN)))) type = TYPE_BOOLEAN;
                // else yyerror("Expected boolean.");
            } else if ((strcmp(op, "<") == 0) || (strcmp(op, ">") == 0) || (strcmp(op, "<=") == 0) || (strcmp(op, ">=") == 0)) {
                if (((expr1->typeCheck(TYPE_INTEGER)) || (expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = TYPE_BOOLEAN;
                // else yyerror("Expected int or real.");
            } else if ((strcmp(op, "=") == 0) || (strcmp(op, "<>") == 0)) {
                if (expr1->type == expr2->type) type = TYPE_BOOLEAN;
                // else yyerror("Type mismatch!");
            }
        }
};


class IntConst : public RVal {
    private:
        int val;
    public:
        Types type;
        IntConst(int n) : val(n), type(TYPE_INTEGER) {}

        virtual void printAST(std::ostream &out) const override { out << "IntConst(" << val << ")"; }
};


class BoolConst : public RVal {
    private:
        bool val;
    public:
        Types type;
        BoolConst(bool b) : val(b), type(TYPE_BOOLEAN) {}

        virtual void printAST(std::ostream &out) const override { out << "BoolConst('" << val << "')"; }
};


class RealConst : public RVal {
    private:
        float val;
    public:
        Types type;
        RealConst(float f) : val(f), type(TYPE_REAL) {}

        virtual void printAST(std::ostream &out) const override { out << "RealConst(" << val << ")"; }
};

class CharConst : public RVal {
    private:
        char val;
    public:
        Types type;
        CharConst(char c) : val(c), type(TYPE_CHAR) {}

        virtual void printAST(std::ostream &out) const override { out << "CharConst(" << val << ")"; }
};


class Nil : public RVal {
    private:
    public:
        Nil() {}

        virtual void printAST(std::ostream &out) const override { out << "Nil()";}
};


class StringLit : public LVal {
    private:
        char *val;
    public:
        Types type;
        StringLit(char *s) : val(s), type(TYPE_STRING) {}

        virtual void printAST(std::ostream &out) const override { out << "StringLit(" << val << ")"; }
};


class Id : public LVal {
    private:
        char *id;
        int offset;
    public:
        Id(char *s) : id(s), offset(-1) {}

        virtual void printAST(std::ostream &out) const override { out << "Id(" << id << ")"; }
        virtual void sem() override {
            type = st.lookup(id)->type;
            STEntry *e = st.lookup(id);
            offset = e->offset;
        }
        char* getName() { return id; }
};


class IdList : public AST {
    private:
        std::vector<std::unique_ptr<Id>> idList;
    public:
        IdList(): idList() {}

        void append(std::unique_ptr<Id> id) { idList.insert(idList.begin(), std::move(id)); }
        virtual void printAST(std::ostream &out) const override {
            out << "IdList(";
            bool start = true;
            for (const auto &id : idList) {
                if (!start) out << ", ";
                start = false;
                out << *id;
            }
            out << ")";
        }
};


class IdLabel : public Stmt {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<Stmt> stmt;
    public:
        IdLabel(std::unique_ptr<Id> i, std::unique_ptr<Stmt> s) : id(std::move(i)), stmt(std::move(s)) {}

        virtual void printAST(std::ostream &out) const override { out << "IdLabel(" << *id << ", " << *stmt << ")"; }
};


class ArrayItem: public LVal {
    private:
        std::unique_ptr<LVal> lVal;
        std::unique_ptr<Expr> expr;
    public:
        ArrayItem(std::unique_ptr<LVal> l, std::unique_ptr<Expr> e) : lVal(std::move(l)), expr(std::move(e)) {}

        virtual void printAST(std::ostream &out) const override {out << "ArrayItem(" << *lVal << ", " << *expr << ")";}
        virtual void sem() override {
            lVal->sem();
            expr->sem();

            
        }
};


class Call : public Stmt {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<ExprList> func;
    public:
        Call(std::unique_ptr<Id> i, std::unique_ptr<ExprList> eL=nullptr) : id(std::move(i)), func(std::move(eL)) {}

        virtual void printAST(std::ostream &out) const override { out << "Call(" << *id << ", " << *func << ")"; }
};


class CallRVal : public RVal {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<ExprList> func;
    public:
        CallRVal(std::unique_ptr<Id> i, std::unique_ptr<ExprList> eL=nullptr) : id(std::move(i)), func(std::move(eL)) {}

        virtual void printAST(std::ostream &out) const override { out << "CallRVal(" << *id << ", " << *func << ")"; }
};


class Dispose: public Stmt {
    private:
        std::unique_ptr<LVal> lVal;
        std::unique_ptr<Expr> exprPtr;
    public:
        Dispose(std::unique_ptr<LVal> l) : lVal(std::move(l)) { exprPtr = nullptr; }
        Dispose(std::unique_ptr<Expr> e) : exprPtr(std::move(e)) { lVal = nullptr; }

        virtual void printAST(std::ostream &out) const override {
            out << "Dispose(";
            if (lVal) out << *lVal;
            else if (exprPtr) out << *exprPtr;
            out << ")";
        }
        virtual void sem() override {

        }
};


class Label: public Stmt {
    private:
        std::unique_ptr<IdList> idList;
    public:
        Label(std::unique_ptr<IdList> iL) : idList(std::move(iL)) {}

        virtual void printAST(std::ostream &out) const override { out << "Label(" << *idList << ")"; }
};


class Assign: public Stmt {
    private:
        std::unique_ptr<LVal> lval;
        std::unique_ptr<Expr> expr, exprPtr;
    public:
        Assign(std::unique_ptr<LVal> l, std::unique_ptr<Expr> e) : lval(std::move(l)), expr(std::move(e)) { exprPtr = nullptr; }
        Assign(std::unique_ptr<Expr> p, std::unique_ptr<Expr> e) : exprPtr(std::move(p)), expr(std::move(e)) { lval = nullptr; }

        virtual void printAST(std::ostream &out) const override {
            out << "Assign(";
            if (lval) out << *lval;
            else if (exprPtr) out << *exprPtr;
            out << ", " << *expr << ")";
        }
};


class Return: public Stmt {
    public:
        Return() {};

        virtual void printAST(std::ostream &out) const override { out << "Return()"; }
};


class New: public Stmt {
    private:
        std::unique_ptr<LVal> lVal;
        std::unique_ptr<Expr> expr, exprPtr;
    public:
        New(std::unique_ptr<LVal> l, std::unique_ptr<Expr> e=nullptr) : lVal(std::move(l)), expr(std::move(e)) { exprPtr = nullptr; }
        New(std::unique_ptr<Expr> p, std::unique_ptr<Expr> e=nullptr) : exprPtr(std::move(p)), expr(std::move(e)) { lVal = nullptr; }

        virtual void printAST(std::ostream &out) const override {
            out << "New(";
            if (lVal) out << *lVal;
            else if (exprPtr) out << *exprPtr;
            if (expr) out << ", " << *expr;
            out << ")";
        }
};


class Goto: public Stmt {
    private:
        std::unique_ptr<Id> id;
    public:
        Goto(std::unique_ptr<Id> c) : id(std::move(c)) {}

        virtual void printAST(std::ostream &out) const override { out << "Goto(" << *id << ")"; }
};


class Decl : public AST {
    private:
        std::unique_ptr<IdList> idList;
        std::unique_ptr<Type> type;
    public:
        Decl(std::unique_ptr<IdList> iL, std::unique_ptr<Type> t) : idList(std::move(iL)), type(std::move(t)) {}

        virtual void printAST(std::ostream &out) const override { out << "Declaration(" << *idList << ", " << *type << ")"; }
        virtual void sem() override {}
};


class DeclList : public AST {
    private:
        std::vector<std::unique_ptr<Decl>> decList;
    public:
        DeclList() : decList() {}

        void append(std::unique_ptr<Decl> d) { decList.insert(decList.begin(), std::move(d)); }
        virtual void printAST(std::ostream &out) const override {
            out << "DeclList(";
            bool start = true;
            for (const auto &d : decList) {
                if (!start) out << ", ";
                start = false;
                out << *d;
            }
            out << ")";
        }
        virtual void sem() override { for (auto &d : decList) d->sem(); }
};


class Header : public AST {};


class Formal : public AST {
    private:
        std::unique_ptr<IdList> idList;
        std::unique_ptr<Type> type;
    public:
    Formal(std::unique_ptr<IdList> iL, std::unique_ptr<Type> t) : idList(std::move(iL)), type(std::move(t)) {}

    void printAST(std::ostream &out) const override { out << "Formal(" << *idList << ", " << *type << ")"; }
};


class FormalList : public AST {
    private:
        std::vector<std::unique_ptr<Formal>> formalList;
    public:
        FormalList() : formalList() {}

        void append(std::unique_ptr<Formal> f) { formalList.insert(formalList.begin(), std::move(f)); }
        virtual void printAST(std::ostream &out) const override {
            out << "FormalList(";
            bool start = true;
            for (const auto &f : formalList) {
                if (!start) out << ", ";
                start = false;
                out << *f;
            }
            out << ")";
        }
        virtual void sem() override { for (auto &f : formalList) f->sem(); }
};


class Procedure : public Header {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<FormalList> formalList;
    public:
        Procedure(std::unique_ptr<Id> i, std::unique_ptr<FormalList> fL=nullptr) : id(std::move(i)), formalList(std::move(fL)) {}

        virtual void printAST(std::ostream &out) const override { out << "Procedure(" << *id << ", " << *formalList << ")"; }
};


class Function : public Header {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<Type> type;
        std::unique_ptr<FormalList> formalList;
    public:
        Function(std::unique_ptr<Id> i, std::unique_ptr<Type> t, std::unique_ptr<FormalList> fL=nullptr) : id(std::move(i)), type(std::move(t)), formalList(std::move(fL)) {}

        virtual void printAST(std::ostream &out) const { out << "Function(" << *id << ", type=" << *type << ", " << *formalList << ")"; }
        virtual void sem() override {
            std::string s = id->getName();
        }

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
            if (localType.compare("var") == 0) out << *dList;
            else if (localType.compare("label") == 0) out << *lbl;
            else if (localType.compare("forp") == 0) out << *hdr << ", " << *body;
            else if (localType.compare("forward") == 0) out << *hdr;
            out << ")";
        }
        virtual void sem() override {
            if (localType.compare("var") == 0) dList->sem();
            else if (localType.compare("label")) lbl->sem();
            else if (localType.compare("forp")) {
                hdr->sem();
                body->sem();
            } else if (localType.compare("forward")) hdr->sem();
        }
};


class LocalList : public AST {
    private:
        std::vector<std::unique_ptr<Local>> localList;
    public:
        LocalList(): localList() {}

        void append(std::unique_ptr<Local> l) { localList.insert(localList.begin(), std::move(l)); }
        virtual void printAST(std::ostream &out) const override {
            out << "LocalList(";
            bool start = true;
            for (const auto &l : localList) {
                if (!start) out << ", ";
                start = false;
                out << *l;
            }
            out << ")";
        }
        virtual void sem() override { for (auto &l : localList) l->sem(); }
};


class Body : public Stmt {
    private:
        std::unique_ptr<LocalList> localList;
        std::unique_ptr<Block> block;
    public:
        Body(std::unique_ptr<LocalList> l, std::unique_ptr<Block> b) : localList(std::move(l)), block(std::move(b)) {}

        virtual void printAST(std::ostream &out) const override { out << "Body(" << *localList << ", " << *block << ")"; }
        virtual void sem() override {
            st.enterScope();
            localList->sem();
            block->sem();
            st.exitScope();
        }
};


class Reference : public RVal {
    private:
        std::unique_ptr<LVal> lVal;
    public:
        Reference(std::unique_ptr<LVal> l) : lVal(std::move(l)) {}

        virtual void printAST(std::ostream &out) const override { out << "Reference(" << *lVal << ")"; }
};


class Result : public LVal {
    private:
    public:
        Result() {}

        virtual void printAST(std::ostream &out) const override { out << "Result()"; }
};

#endif