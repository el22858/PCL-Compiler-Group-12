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
        std::vector<Stmt *> stmt_list;
    public:
        StmtList() : stmt_list() {}
        ~StmtList() { for (Stmt *s : stmt_list) delete s; }

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
        virtual void sem() override { for (Stmt *s : stmt_list) s->sem(); }

        void append(Stmt* s) { stmt_list.push_back(s); }
};


class Expr : public AST {
    private:
    public:
        Types type;
        bool typeCheck(Types t) { return type == t; }
};


class ExprList : public Expr {
    private:
        std::vector<Expr *> expr_list;
    public:
        ExprList() : expr_list() {}
        ~ExprList() { for (Expr *e : expr_list) delete e; }

        void append(Expr *e) {expr_list.push_back(e);}
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

        virtual void sem() override { for (Expr *e : expr_list) e->sem(); }
};


class Block : public Stmt {
    private:
        StmtList *stmt_list;
    public:
        Block(StmtList *sL = nullptr) : stmt_list(sL) {}
        ~Block() { delete stmt_list; }

        virtual void printAST(std::ostream &out) const override { out << "Block(" << *stmt_list << ")"; }
        virtual void sem() override { stmt_list->sem(); }
};


class ITE : public Stmt {
    private:
        Expr *expr;
        Stmt *stmt1, *stmt2;
    public:
        ITE(Expr *e, Stmt *s1, Stmt *s2 = nullptr) : expr(e), stmt1(s1), stmt2(s2) {}

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
            } else yyerror("Expected boolean.");
        }
};


class While : public Stmt {
    private:
        Expr *expr;
        Stmt *stmt;
    public:
        While(Expr *e, Stmt *s) : expr(e), stmt(s) {}

        virtual void printAST(std::ostream &out) const override { out << "While(" << *expr << ", " << *stmt << ")"; }
        virtual void sem() override {
            expr->sem();

            if (expr->typeCheck(TYPE_BOOLEAN)) stmt->sem();
            else yyerror("Expected boolean.");
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
        Expr *expr;
    public:
        UnOp(char *s, Expr *e) : op(s), expr(e) {}
        ~UnOp() {
            delete op;
            delete expr;
        }

        virtual void printAST(std::ostream &out) const override { out << "UnOp(" << op << ", " << *expr << ")"; }
        virtual void sem() override {
            expr->sem();

            if ((strcmp(op, "+") == 0) || (strcmp(op, "-") == 0)) {
                if (expr->typeCheck(TYPE_INTEGER)) type = TYPE_INTEGER;
                else if (expr->typeCheck(TYPE_REAL)) type = TYPE_REAL;
                else yyerror("Expected number");
            } else if (strcmp(op, "not") == 0) {
                if (expr->typeCheck(TYPE_BOOLEAN)) type = TYPE_BOOLEAN;
                else yyerror("Expected boolean.");
            } else yyerror("Unary Operator not recognized"); // Trash
        }
};


class BinOp : public RVal {
    private:
        Expr *expr1, *expr2;
        char *op;
    public:
        BinOp(Expr *e1, char *s, Expr *e2) : expr1(e1), op(s), expr2(e2) {}
        ~BinOp() {
            delete expr1;
            delete op;
            delete expr2;
        }

        virtual void printAST(std::ostream &out) const override { out << "BinOp(" << *expr1 << ", " << op << ", " << *expr2 << ")"; }
        virtual void sem() override {
            expr1->sem();
            expr2->sem();

            if ((strcmp(op, "+") == 0) || (strcmp(op, "-") == 0) || (strcmp(op, "*") == 0)) {
                if (expr1->typeCheck(TYPE_INTEGER)) {
                    if (expr2->typeCheck(TYPE_INTEGER)) type = TYPE_INTEGER;
                    else if (expr2->typeCheck(TYPE_REAL)) type = TYPE_REAL;
                    else yyerror("Expected int or real.");
                } else if (expr1->typeCheck(TYPE_REAL)) {
                    if ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL))) type = TYPE_REAL;
                    else yyerror("Expected int or real.");
                }
            } else if (strcmp(op, "/")) {
                if ((expr1->typeCheck(TYPE_INTEGER)) || ((expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = TYPE_REAL;
                else yyerror("Expected int or real.");
            } else if ((strcmp(op, "div") == 0) || (strcmp(op, "mod") == 0)) {
                if ((expr1->typeCheck(TYPE_INTEGER)) && (expr2->typeCheck(TYPE_INTEGER))) type = TYPE_INTEGER;
                else yyerror("Expected int.");
            } else if ((strcmp(op, "and") == 0) || (strcmp(op, "or") == 0)) {
                if ((expr1->typeCheck(TYPE_BOOLEAN) && (expr2->typeCheck(TYPE_BOOLEAN)))) type = TYPE_BOOLEAN;
                else yyerror("Expected boolean.");
            } else if ((strcmp(op, "<") == 0) || (strcmp(op, ">") == 0) || (strcmp(op, "<=") == 0) || (strcmp(op, ">=") == 0)) {
                if (((expr1->typeCheck(TYPE_INTEGER)) || (expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = TYPE_BOOLEAN;
                else yyerror("Expected int or real.");
            } else if ((strcmp(op, "=") == 0) || (strcmp(op, "<>") == 0)) {
                if (expr1->type == expr2->type) type = TYPE_BOOLEAN;
                else yyerror("Type mismatch!");
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
    public:
        Id(char *s) : id(s) {}

        virtual void printAST(std::ostream &out) const override { out << "Id(" << id << ")"; }
        char* getName() { return id; }
};


class IdList : public AST {
    private:
        std::vector<Id *> idList;
    public:
        IdList(): idList() {}
        ~IdList() { for (Id *id : idList) delete id; }

        void append(Id *id) { idList.push_back(id); }
        virtual void printAST(std::ostream &out) const override {
            out << "IdList(";
            bool start = true;
            for (Id *id : idList) {
                if (!start) out << ", ";
                start = false;
                out << *id;
            }
            out << ")";
        }
};


class IdLabel : public Stmt {
    private:
        Id *id;
        Stmt *stmt;
    public:
        IdLabel(Id *i, Stmt *s) : id(i), stmt(s) {}

        virtual void printAST(std::ostream &out) const override { out << "IdLabel(" << *id << ", " << *stmt << ")"; }
};


class ArrayItem: public LVal {
    private:
        LVal *lVal;
        Expr *expr;
    public:
        ArrayItem(LVal *l, Expr *e) : lVal(l), expr(e) {}
        ~ArrayItem() {
            delete lVal;
            delete expr;
        }

        virtual void printAST(std::ostream &out) const override {out << "ArrayItem(" << *lVal << ", " << *expr << ")";}
};


class Call : public Stmt {
    private:
        Id *id;
        ExprList *func;
    public:
        Call(Id *i, ExprList *eL=nullptr) : id(i), func(eL) {}

        virtual void printAST(std::ostream &out) const override { out << "Call(" << *id << ", " << *func << ")"; }
};


class CallRVal : public RVal {
    private:
        Id *id;
        ExprList *func;
    public:
        CallRVal(Id *i, ExprList *eL=nullptr) : id(i), func(eL) {}

        virtual void printAST(std::ostream &out) const override { out << "CallRVal(" << *id << ", " << *func << ")"; }
};


class Dispose: public Stmt {
    private:
        LVal *lVal;
        Expr *exprPtr;
    public:
        Dispose(LVal *l) : lVal(l) { exprPtr = nullptr; }
        Dispose(Expr *e) : exprPtr(e) { lVal = nullptr; }
        ~Dispose() { delete lVal; }

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
        IdList *idList;
    public:
        Label(IdList *iL) : idList(iL) {}
        ~Label() { delete idList; }

        virtual void printAST(std::ostream &out) const override { out << "Label(" << *idList << ")"; }
};


class Assign: public Stmt {
    private:
        LVal *lval;
        Expr *expr, *exprPtr;
    public:
        Assign(LVal *l, Expr *e) : lval(l), expr(e) { exprPtr = nullptr; }
        Assign(Expr *p, Expr *e) : exprPtr(p), expr(e) { lval = nullptr; }
        ~Assign() {
            delete lval;
            delete expr;
        }

        virtual void printAST(std::ostream &out) const override {
            out << "Assign(";
            if (lval) out << *lval;
            else if (exprPtr) out << *exprPtr;
            out << ", " << expr << ")";
        }
};


class Return: public Stmt {
    public:
        Return() {};

        virtual void printAST(std::ostream &out) const override { out << "Return()"; }
};


class New: public Stmt {
    private:
        LVal *lVal;
        Expr *expr, *exprPtr;
    public:
        New(LVal *l, Expr *e=nullptr) : lVal(l), expr(e) { exprPtr = nullptr; }
        New(Expr *p, Expr *e=nullptr) : exprPtr(p), expr(e) { lVal = nullptr; }
        ~New() {
            delete lVal;
            delete expr;
        }

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
        Id *id;
    public:
        Goto(Id *c) : id(c) {}
        ~Goto() { delete id; }

        virtual void printAST(std::ostream &out) const override { out << "Goto(" << *id << ")"; }
};


class Decl : public AST {
    private:
        IdList *idList;
        Type *type;
    public:
        Decl(IdList *iL, Type *t) : idList(iL), type(t) {}
        ~Decl() {
            delete idList;
            delete type;
        }

        virtual void printAST(std::ostream &out) const override { out << "Declaration(" << *idList << ", " << *type << ")"; }
        virtual void sem() override {}
};


class DeclList : public AST {
    private:
        std::vector<Decl *> decList;
    public:
        DeclList() : decList() {}
        ~DeclList() { for (Decl *d : decList) delete d; }

        void append(Decl *d) { decList.push_back(d); }
        virtual void printAST(std::ostream &out) const override {
            out << "DeclList(";
            bool start = true;
            for (Decl *d : decList) {
                if (!start) out << ", ";
                start = false;
                out << *d;
            }
            out << ")";
        }
        virtual void sem() override { for (Decl *d : decList) d->sem(); }
};


class Header : public AST {};


class Formal : public AST {
    private:
        IdList *idList;
        Type *type;
    public:
    Formal(IdList *iL, Type *t) : idList(iL), type(t) {}
    ~Formal() {
        delete idList;
        delete type;
    }

    void printAST(std::ostream &out) const override { out << "Formal(" << *idList << ", " << *type << ")"; }
};


class FormalList : public AST {
    private:
        std::vector<Formal *> formalList;
    public:
        FormalList() : formalList() {}
        ~FormalList() { for (Formal *f : formalList) delete f; }

        void append(Formal *f) { formalList.push_back(f); }
        virtual void printAST(std::ostream &out) const override {
            out << "FormalList(";
            bool start = true;
            for (Formal *f : formalList) {
                if (!start) out << ", ";
                start = false;
                out << *f;
            }
            out << ")";
        }
        virtual void sem() override { for (Formal *f : formalList) f->sem(); }
};


class Procedure : public Header {
    private:
        Id *id;
        FormalList *formalList;
    public:
        Procedure(Id *i, FormalList *fL=nullptr) : id(i), formalList(fL) {}
        ~Procedure() {
            delete id;
            delete formalList;
        }

        virtual void printAST(std::ostream &out) const override { out << "Procedure(" << *id << ", " << *formalList << ")"; }
};


class Function : public Header {
    private:
        Id *id;
        Type *type;
        FormalList *formalList;
    public:
        Function(Id *i, Type *t, FormalList *fL=nullptr) : id(i), type(t), formalList(fL) {}
        ~Function() {
            delete id;
            delete type;
            delete formalList;
        }

        virtual void printAST(std::ostream &out) const { out << "Function(" << *id << ", type=" << *type << ", " << *formalList << ")"; }
        virtual void sem() override {
            std::string s = id->getName();
        }

};


class Local : public AST {
    private:
        DeclList *dList;
        Label *lbl;
        Header *hdr;
        AST *body;
        std::string localType;
    public:
        Local(DeclList *dL) : dList(dL), localType("var") {}
        Local(Label *l) : lbl(l), localType("label") {}
        Local(Header *h, AST *b) : hdr(h), body(b), localType("forp") {}
        Local(Header *h) : hdr(h), localType("forward") {}

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
        std::vector<Local *> localList;
    public:
        LocalList(): localList() {}
        ~LocalList() { for (Local *l : localList) delete l; }

        void append(Local *l) { localList.push_back(l); }
        virtual void printAST(std::ostream &out) const override {
            out << "LocalList(";
            bool start = true;
            for (Local *l : localList) {
                if (!start) out << ", ";
                start = false;
                out << *l;
            }
            out << ")";
        }
        virtual void sem() override { for (Local *l : localList) l->sem(); }
};


class Body : public Stmt {
    private:
        LocalList *localList;
        Block *block;
    public:
        Body(LocalList *l, Block *b) : localList(l), block(b) {}
        ~Body() {
            delete localList;
            delete block;
        }

        virtual void printAST(std::ostream &out) const override { out << "Body(" << *localList << ", " << *block << ")"; }
        virtual void sem() override {
            localList->sem();
            block->sem();
        }
};


class Reference : public RVal {
    private:
        LVal *lVal;
    public:
        Reference(LVal *l) : lVal(l) {}
        ~Reference() { delete lVal; }

        virtual void printAST(std::ostream &out) const override { out << "Reference(" << *lVal << ")"; }
};


class Result : public LVal {
    private:
    public:
        Result() {}

        virtual void printAST(std::ostream &out) const override { out << "Result()"; }
};

#endif