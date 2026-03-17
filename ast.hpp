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


class StmtList : public Stmt {
    private:
        vector<Stmt *> stmt_list;
    public:
        StmtList() : stmt_list() {}
        ~StmtList() { for (Stmt *s : stmt_list) delete s; }

        virtual void printAST(ostream &out) const override {
            out << "StatmentList(";
            bool start = true;

            for (const auto &x : stmt_list) {
                if (!start) out << ", ";
                start = false;
                out << *x;
            }
            out << ")";
        }

        void append(Stmt *s) { stmt_list.push_back(s); }
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
        ~ExprList() { for (Expr *e : expr_list) delete e; }

        void append(Expr *e) {expr_list.push_back(e);}
        virtual void printAST(ostream &out) const override {
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


enum Types { TYPE_INTEGER, TYPE_BOOLEAN, TYPE_REAL, TYPE_ARRAY, TYPE_IARRAY, TYPE_CHAR, TYPE_STRING, TYPE_POINTER};


class Type : public AST {
    private:
        Types val;
    public:
};


class Integer : public Type {
    private:
        Types val;
    public:
        Integer() : val(TYPE_INTEGER) {}

        virtual void printAST(ostream &out) const override { out << "Integer()"; }
};


class String : public Type {
    private:
        Types val;
    public:
        String() : val(TYPE_STRING) {}

        virtual void printAST(ostream &out) const override { out << "String()"; }
};


class Char : public Type {
    private:
        Types val;
    public:
        Char() : val(TYPE_CHAR) {}

        virtual void printAST(ostream &out) const override { out << "Const()"; }
};


class Real : public Type {
    private:
        Types val;
    public:
        Real() : val(TYPE_REAL) {}

        virtual void printAST(ostream &out) const override { out << "Real()"; }
};


class Boolean : public Type {
    private:
        Types val;
    public:
        Boolean() : val(TYPE_BOOLEAN) {}

        virtual void printAST(ostream &out) const override { out << "Boolean()"; }
};


class Array : public Type {
    private:
        Types val;
        Type *arType;
        int size;
    public:
        Array(Type *t, int s = -1) : val(TYPE_ARRAY), arType(t) { size = (s > 0) ? s : -1; }

        virtual void printAST(ostream &out) const override {
            out << "Array(";
            if (size > 0) out << "size=" << size << ", ";
            out << "type=" << *arType << ")";
        }
};


class Pointer : public Type {
    private:
        Types val;
        Type *pType;
    public:
        Pointer(Type *t) : val(TYPE_POINTER), pType(t) {}

        virtual void printAST(ostream &out) const override { out << "Pointer(type=" << *pType << ")"; }
};


class Block : public Stmt {
    private:
        StmtList *stmt_list;
    public:
        Block(StmtList *sL = nullptr) : stmt_list(sL) {}
        ~Block() { delete stmt_list; }

        virtual void printAST(ostream &out) const override {
            out << "Block(" << *stmt_list << ")";
        }
};


class ITE : public Stmt {
    private:
        Expr *expr;
        Stmt *stmt1, *stmt2;
    public:
        ITE(Expr *e, Stmt *s1, Stmt *s2 = nullptr) : expr(e), stmt1(s1), stmt2(s2) {}

        virtual void printAST(ostream &out) const override {
            out << "If(" << *expr << ", " << *stmt1;
            if (stmt2 != nullptr) out << ", " << *stmt2;
            out << ")";
        }
};


class While : public Stmt {
    private:
        Expr *expr;
        Stmt *stmt;
    public:
        While(Expr *e, Stmt *s) : expr(e), stmt(s) {}

        virtual void printAST(ostream &out) const override { out << "While(" << *expr << ", " << *stmt << ")"; }
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

        virtual void printAST(ostream &out) const override { out << "UnOp(" << *op << ", " << *expr << ")"; }
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

        virtual void printAST(ostream &out) const override { out << "BinOp(" << *expr1 << ", " << *op << ", " << *expr2 << ")"; }
};


class IntConst : public RVal {
    private:
        int val;
    public:
        Type *type;
        IntConst(int n) : val(n) { type = new Integer(); }

        virtual void printAST(ostream &out) const override { out << "IntConst(" << val << ")"; }
};


class BoolConst : public RVal {
    private:
        bool val;
    public:
        Type *type;
        BoolConst(bool b) : val(b) { type = new Boolean(); }

        virtual void printAST(ostream &out) const override { out << "BoolConst('" << val << "')"; }
};


class RealConst : public RVal {
    private:
        float val;
    public:
        Type *type;
        RealConst(float f) : val(f) { type = new Real(); }

        virtual void printAST(ostream &out) const override { out << "RealConst(" << val << ")"; }
};

class CharConst : public RVal {
    private:
        char val;
    public:
        Type *type;
        CharConst(char c) : val(c) { type = new Char(); }

        virtual void printAST(ostream &out) const override { out << "CharConst(" << val << ")"; }
};


class Nil : public RVal {
    private:
    public:
        Nil() {}

        virtual void printAST(ostream &out) const override { out << "Nil()";}
};


class StringLit : public LVal {
    private:
        char *val;
    public:
        Type *type;
        StringLit(char *s) : val(s) { type = new String(); }

        virtual void printAST(ostream &out) const override { out << "StringLit(" << *val << ")"; }
};


class Id : public LVal {
    private:
        char *id;
    public:
        Id(char *s) : id(s) {}

        virtual void printAST(ostream &out) const override { out << "Id(" << *id << ")"; }
};


class IdList : public AST {
    private:
        vector<Id *> idList;
    public:
        IdList(): idList() {}
        ~IdList() { for (Id *id : idList) delete id; }

        void append(Id *id) { idList.push_back(id); }
        virtual void printAST(ostream &out) const override {
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

        virtual void printAST(ostream &out) const override { out << "IdLabel(" << *id << ", " << *stmt << ")"; }
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

        virtual void printAST(ostream &out) const override {out << "ArrayItem(" << *lVal << ", " << *expr << ")";}
};


class Call : public Stmt {
    private:
        Id *id;
        ExprList *func;
    public:
        Call(Id *i, ExprList *eL=nullptr) : id(i), func(eL) {}

        virtual void printAST(ostream &out) const override { out << "Call(" << *id << ", " << *func << ")"; }
};


class CallRVal : public RVal {
    private:
        Id *id;
        ExprList *func;
    public:
        CallRVal(Id *i, ExprList *eL=nullptr) : id(i), func(eL) {}

        virtual void printAST(ostream &out) const override { out << "CallRVal(" << *id << ", " << *func << ")"; }
};


class Dispose: public Stmt {
    private:
        LVal *lVal;
        Expr *exprPtr;
    public:
        Dispose(LVal *l) : lVal(l) { exprPtr = nullptr; }
        Dispose(Expr *e) : exprPtr(e) { lVal = nullptr; }
        ~Dispose() { delete lVal; }

        virtual void printAST(ostream &out) const override {
            out << "Dispose(";
            if (lVal) out << *lVal;
            else if (exprPtr) out << *exprPtr;
            out << ")"; }
};


class Label: public Stmt {
    private:
        IdList *idList;
    public:
        Label(IdList *iL) : idList(iL) {}
        ~Label() { delete idList; }

        virtual void printAST(ostream &out) const override { out << "Label(" << *idList << ")"; }
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

        virtual void printAST(ostream &out) const override {
            out << "Assign(";
            if (lval) out << *lval;
            else if (exprPtr) out << *exprPtr;
            out << ", " << expr << ")";
        }
};


class Return: public Stmt {
    public:
        Return() {};

        virtual void printAST(ostream &out) const override { out << "Return()"; }
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

        virtual void printAST(ostream &out) const override {
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

        virtual void printAST(ostream &out) const override { out << "Goto(" << *id << ")"; }
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

        virtual void printAST(ostream &out) const override { out << "Declaration(" << *idList << ", " << *type << ")"; }
};


class DeclList : public AST {
    private:
        vector<Decl *> decList;
    public:
        DeclList() : decList() {}
        ~DeclList() { for (Decl *d : decList) delete d; }

        void append(Decl *d) { decList.push_back(d); }
        virtual void printAST(ostream &out) const override {
            out << "DeclList(";
            bool start = true;
            for (Decl *d : decList) {
                if (!start) out << ", ";
                start = false;
                out << *d;
            }
            out << ")";
        }
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

    void printAST(ostream &out) const override { out << "Formal(" << *idList << ", " << *type << ")"; }
};


class FormalList : public AST {
    private:
        vector<Formal *> formalList;
    public:
        FormalList() : formalList() {}
        ~FormalList() { for (Formal *f : formalList) delete f; }

        void append(Formal *f) { formalList.push_back(f); }
        virtual void printAST(ostream &out) const override {
            out << "FormalList(";
            bool start = true;
            for (Formal *f : formalList) {
                if (!start) out << ", ";
                start = false;
                out << *f;
            }
            out << ")";
        }
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

        virtual void printAST(ostream &out) const override { out << "Procedure(" << *id << ", " << *formalList << ")"; }
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

        virtual void printAST(ostream &out) const { out << "Function(" << *id << ", type=" << *type << ", " << *formalList << ")"; }

};


class Local : public AST {
    private:
        DeclList *dList;
        Label *lbl;
        Header *hdr;
        AST *body;
        string localType;
    public:
        Local(DeclList *dL) : dList(dL), localType("var") {}
        Local(Label *l) : lbl(l), localType("label") {}
        Local(Header *h, AST *b) : hdr(h), body(b), localType("forp") {}
        Local(Header *h) : hdr(h), localType("forward") {}

        virtual void printAST(ostream &out) const override {
            out << "Local(";
            if (localType.compare("var") == 0) out << *dList;
            else if (localType.compare("label") == 0) out << *lbl;
            else if (localType.compare("forp") == 0) out << *hdr << ", " << *body;
            else if (localType.compare("forward") == 0) out << *hdr;
            out << ")";
        }
};


class LocalList : public AST {
    private:
        vector<Local *> localList;
    public:
        LocalList(): localList() {}
        ~LocalList() { for (Local *l : localList) delete l; }

        void append(Local *l) { localList.push_back(l); }
        virtual void printAST(ostream &out) const override {
            out << "LocalList(";
            bool start = true;
            for (Local *l : localList) {
                if (!start) out << ", ";
                start = false;
                out << *l;
            }
            out << ")";
        }
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

        virtual void printAST(ostream &out) const override { out << "Body(" << *localList << ", " << *block << ")"; }
};


class Reference : public RVal {
    private:
        LVal *lVal;
    public:
        Reference(LVal *l) : lVal(l) {}
        ~Reference() { delete lVal; }

        virtual void printAST(ostream &out) const override { out << "Reference(" << *lVal << ")"; }
};


class Result : public LVal {
    private:
    public:
        Result() {}

        virtual void printAST(ostream &out) const override { out << "Result()"; }
};