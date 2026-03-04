#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>

#include "symbol.hpp"

using namespace std;

class AST {
    private:
    public:
        virtual void printAST(ostream &out) const = 0;        
};

inline ostream &operator<<(ostream &out, const AST &ast) {
    ast.printAST(out);
    return out;
}


class Stmt : public AST {
    private:
    public:
};


class Expr : public AST {
    private:
    public:
};


class ExprList : public Expr {
    private:
        vector<Expr *> expr_list;
    public:
        ExprList() : expr_list() {}

        void append(Expr *e) {expr_list.push_back(e);}
        void printAST(ostream &out) const override {
            out << "ExpressionList(";
            bool start = true;

            for (const auto &x : expr_list) {
                if (!start) out << ", ";
                start = false;
                out << *x;
            }
            out << ")";
        }
};


class Block : public Stmt {
    private:
        vector<Stmt *> stmt_list;
    public:
        Block() : stmt_list() {}
        
        void append(Stmt *s) {stmt_list.push_back(s);}
        void printAST(ostream &out) const override {
            out << "Block(";
            bool start = true;

            for (const auto &x : stmt_list) {
                if (!start) out<<", ";
                start = false;
                out << *x;
            }
            out << ")";
        }
};


class ITE : public Stmt {
    private:
        Expr *expr;
        Stmt *stmt1, *stmt2;
    public:
        ITE(Expr *e, Stmt *s1, Stmt *s2 = nullptr) : expr(e), stmt1(s1), stmt2(s2) {}

        void printAST(ostream &out) const override {
            out << "If(" << expr << ", " << stmt1;
            if (stmt2 != nullptr) out << ", " << stmt2;
            out << ")";
        }
};


class While : public Stmt {
    private:
        Expr *expr;
        Stmt *stmt;
    public:
        While(Expr *e, Stmt *s) : expr(e), stmt(s) {}

        void printAST(ostream &out) const override { out << "While(" << expr << ", " << stmt << ")"; }
};


class UnOp : public Expr {
    private:
        string op;
        Expr *expr;
    public:
        UnOp(string s, Expr *e) : op(s), expr(e) {}

        void printAST(ostream &out) const override { out << "UnOp(" << op << ", " << expr << ")"; }
};


class CharBinOp : public Expr {
    private:
        Expr *expr1, *expr2;
        char op;
    public:
        CharBinOp(Expr *e1, char o, Expr *e2) : expr1(e1), op(o), expr2(e2) {}

        void printAST(ostream &out) const override { out << "BinOp(" << *expr1 << ", " << op << ", " << *expr2 << ")"; }
};


class StringBinOp : public Expr {
    private:
        Expr *expr1, *expr2;
        string op;
    public:
        StringBinOp(Expr *e1, string s, Expr *e2) : expr1(e1), op(s), expr2(e2) {}

        void printAST(ostream &out) const override { out << "BinOp(" << *expr1 << ", " << op << ", " << *expr2 << ")"; }
};


class IntConst : public Expr {
    private:
        int val;
    public:
        IntConst(int n) : val(n) {}

        void printAST(ostream &out) const override { out << "IntConst(" << val << ")"; }
};


class BoolConst : public Expr {
    private:
        bool val;
    public:
        BoolConst(bool b) : val(b) {}

        void printAST(ostream &out) const override { out << "BoolConst('" << val << "')"; }
};


class RealConst : public Expr {
    private:
        float val;
    public:
        RealConst(float f) : val(f) {}

        void printAST(ostream &out) const override { out << "RealConst(" << val << ")"; }
};

class CharConst : public Expr {
    private:
        char val;
    public:
        CharConst(char c) : val(c) {}

        void printAST(ostream &out) const override { out << "CharConst(" << val << ")"; }
};


class Nil : public Expr {
    private:
    public:
        Nil() {}

        void printAST(ostream &out) const override { out << "Nil";}
};


class StringLit : public Expr {
    private:
        string val;
    public:
        StringLit(string s) : val(s) {}

        void printAST(ostream &out) const override { out << "StringLit(" << val << ")"; }
};


class Id : public Expr {
    private:
        string id;
    public:
        Id(string s) : id(s) {}

        void printAST(ostream &out) const override { out << "Id(" << id << ")"; }
};


//...
#endif