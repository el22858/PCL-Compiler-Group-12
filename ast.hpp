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
                    x->printAST(out);
                }
            }
            out << ")";
        }
        virtual void sem() override { for (auto &s : stmt_list) s->sem(); }

        void append(std::unique_ptr<Stmt> s) { stmt_list.insert(stmt_list.begin(), std::move(s)); }
};


class Expr : public AST {
    protected:
    public:
        std::unique_ptr<Type> type;
        virtual bool isRes() { return false; }
        bool typeCheck(Types t) { return type->getType() == t; }
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
                x->printAST(out);
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

        virtual void printAST(std::ostream &out) const override {
            out << "Block(";
            stmt_list->printAST(out);
            out << ")"; }
        virtual void sem() override { stmt_list->sem(); }
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
        virtual void sem() override {
            expr->sem();
            if (expr->typeCheck(TYPE_BOOLEAN)) {
                stmt1->sem();
                if (stmt2) stmt2->sem();
            } else yyerror("Expected boolean for ITE.");
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
        virtual void sem() override {
            expr->sem();

            if (expr->typeCheck(TYPE_BOOLEAN)) stmt->sem();
            else yyerror("Expected boolean for While loop.");
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

        virtual void printAST(std::ostream &out) const override {
            out << "UnOp(" << op << ", ";
            expr->printAST(out);
            out << ")";
        }
        virtual void sem() override {
            expr->sem();

            if ((strcmp(op, "+") == 0) || (strcmp(op, "-") == 0)) {
                if (expr->typeCheck(TYPE_INTEGER)) type = std::make_unique<Integer>();
                else if (expr->typeCheck(TYPE_REAL)) type = std::make_unique<Real>();
                else yyerror("Expected number");
            } else if (strcmp(op, "not") == 0) {
                if (expr->typeCheck(TYPE_BOOLEAN)) type = std::make_unique<Boolean>();
                else yyerror("Expected boolean.");
            } else yyerror("Unary Operator not recognized"); // Trash
        }
};


class BinOp : public RVal {
    private:
        std::unique_ptr<Expr> expr1, expr2;
        char *op;
    public:
        BinOp(std::unique_ptr<Expr> e1, char *s, std::unique_ptr<Expr> e2) : expr1(std::move(e1)), op(s), expr2(std::move(e2)) {}

        virtual void printAST(std::ostream &out) const override {
            out << "BinOp(";
            expr1->printAST(out);
            out << ", " << op << ", ";
            expr2->printAST(out);
            out << ")";
        }
        virtual void sem() override {
            expr1->sem();
            expr2->sem();

            if ((strcmp(op, "+") == 0) || (strcmp(op, "-") == 0) || (strcmp(op, "*") == 0)) {
                if (expr1->typeCheck(TYPE_INTEGER)) {
                    if (expr2->typeCheck(TYPE_INTEGER)) type = std::make_unique<Integer>();
                    else if (expr2->typeCheck(TYPE_REAL)) type = std::make_unique<Real>();
                    else yyerror("Expected int or real.");
                } else if (expr1->typeCheck(TYPE_REAL)) {
                    if ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL))) type = std::make_unique<Real>();
                    else yyerror("Expected int or real.");
                }
            } else if (strcmp(op, "/")) {
                if ((expr1->typeCheck(TYPE_INTEGER)) || ((expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = std::make_unique<Real>();
                else yyerror("Expected int or real.");
            } else if ((strcmp(op, "div") == 0) || (strcmp(op, "mod") == 0)) {
                if ((expr1->typeCheck(TYPE_INTEGER)) && (expr2->typeCheck(TYPE_INTEGER))) type = std::make_unique<Integer>();
                else yyerror("Expected int.");
            } else if ((strcmp(op, "and") == 0) || (strcmp(op, "or") == 0)) {
                if ((expr1->typeCheck(TYPE_BOOLEAN) && (expr2->typeCheck(TYPE_BOOLEAN)))) type = std::make_unique<Boolean>();
                else yyerror("Expected boolean.");
            } else if ((strcmp(op, "<") == 0) || (strcmp(op, ">") == 0) || (strcmp(op, "<=") == 0) || (strcmp(op, ">=") == 0)) {
                if (((expr1->typeCheck(TYPE_INTEGER)) || (expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = std::make_unique<Boolean>();
                else yyerror("Expected int or real.");
            } else if ((strcmp(op, "=") == 0) || (strcmp(op, "<>") == 0)) {
                if (expr1->type == expr2->type) type = std::make_unique<Boolean>();
                else yyerror("Type mismatch!");
            }
        }
};


class IntConst : public RVal {
    private:
        int val;
    public:
        IntConst(int n) : val(n) {}

        virtual void printAST(std::ostream &out) const override { out << "IntConst(" << val << ")"; }
        virtual void sem() override { type = std::make_unique<Integer>(); }
};


class BoolConst : public RVal {
    private:
        bool val;
    public:
        BoolConst(bool b) : val(b) {}

        virtual void printAST(std::ostream &out) const override { out << "BoolConst('" << val << "')"; }
        virtual void sem() override { type = std::make_unique<Boolean>(); }
};


class RealConst : public RVal {
    private:
        float val;
    public:
        RealConst(float f) : val(f) {}

        virtual void printAST(std::ostream &out) const override { out << "RealConst(" << val << ")"; }
        virtual void sem() override { type = std::make_unique<Real>(); }
};

class CharConst : public RVal {
    private:
        char val;
    public:
        CharConst(char c) : val(c) {}

        virtual void printAST(std::ostream &out) const override { out << "CharConst(" << val << ")"; }
        virtual void sem() override { type = std::unique_ptr<Char>(); }
};


class Nil : public RVal {
    private:
    public:
        Nil() {}

        virtual void printAST(std::ostream &out) const override { out << "Nil()";}
        virtual void sem() override { type = std::make_unique<TypeNil>(); }
};


class StringLit : public LVal {
    private:
        char *val;
    public:
        StringLit(char *s) : val(s) {}

        virtual void printAST(std::ostream &out) const override { out << "StringLit(" << val << ")"; }
        virtual void sem() override { type = std::make_unique<String>(); }
};


class Id : public LVal {
    private:
        char *id;
        int offset;
    public:
        Id(char *s) : id(s), offset(-1) {}

        virtual void printAST(std::ostream &out) const override { out << "Id(" << id << ")"; }
        virtual void sem() override {
            type = std::move(st.lookup(id)->type);
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

        std::vector<std::unique_ptr<Id>> getList() { return idList; }

        void append(std::unique_ptr<Id> id) { idList.push_back(std::move(id)); }
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
        virtual void sem() override { for (const auto &id : idList) id->sem(); }
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
        virtual void sem() override {
            char *c = id->getName();
            if (!st.isLabel(c)) yyerror("Not a label.");
            else st.insertLabelStmt(c, std::move(stmt));
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
        virtual void sem() override {
            lVal->sem();
            if (lVal->typeCheck(TYPE_RES)) lVal->type = std::move(st.lookup("result")->type);
            
            expr->sem();
            if (expr->typeCheck(TYPE_RES)) expr->type = std::move(st.lookup("result")->type);
            
            if (!(lVal->typeCheck(TYPE_ARRAY))) yyerror("Expected array.");
            else if (!(expr->typeCheck(TYPE_INTEGER))) yyerror("Expected integer");

            type = std::move(lVal->type);
        }
};


class Call : public Stmt {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<ExprList> func;
    public:
        Call(std::unique_ptr<Id> i, std::unique_ptr<ExprList> eL=nullptr) : id(std::move(i)), func(std::move(eL)) {}

        virtual void printAST(std::ostream &out) const override {
            out << "Call(";
            id->printAST(out);
            out << ", ";
            func->printAST(out);
            out << ")";
        }
        virtual void sem() override { /* ... */ }
};


class CallRVal : public RVal {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<ExprList> func;
    public:
        CallRVal(std::unique_ptr<Id> i, std::unique_ptr<ExprList> eL=nullptr) : id(std::move(i)), func(std::move(eL)) {}

        virtual void printAST(std::ostream &out) const override {
            out << "CallRVal(";
            id->printAST(out);
            out << ", ";
            func->printAST(out);
            out << ")";
        }
        virtual void sem() override { /* ... */ }
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
            if (lVal) lVal->printAST(out);
            else if (exprPtr) exprPtr->printAST(out);
            out << ")";
        }
        virtual void sem() override { /* ... */ }
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
        virtual void sem() override { for (const auto &id : idList->getList()) st.insertLabel(id->getName(), std::make_unique<TypeLbl>()); }
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
            if (lval) lval->printAST(out);
            else if (exprPtr) exprPtr->printAST(out);
            out << ", ";
            expr->printAST(out);
            out << ")";
        }
        virtual void sem() override { /* ... */ }
};


class Return: public Stmt {
    public:
        Return() {};

        virtual void printAST(std::ostream &out) const override { out << "Return()"; }
        virtual void sem() override {}
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
            if (lVal) lVal->printAST(out);
            else if (exprPtr) exprPtr->printAST(out);
            if (expr) {
                out << ", ";
                expr->printAST(out);
            }
            out << ")";
        }
        virtual void sem() override { /* ... */ }
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
        virtual void sem() override {
            char *label = id->getName();
            if (!st.isLabel(label)) yyerror("Label Not Found.\n");
            else if (!st.validLabel(label)) yyerror("Label has no Statement.\n");
        }
};


class Decl : public AST {
    private:
        std::unique_ptr<IdList> idList;
        std::unique_ptr<Type> type;
    public:
        Decl(std::unique_ptr<IdList> iL, std::unique_ptr<Type> t) : idList(std::move(iL)), type(std::move(t)) {}

        virtual void printAST(std::ostream &out) const override {
            out << "Declaration(";
            idList->printAST(out);
            out << ", ";
            type->printAST(out);
            out << ")";
        }
        virtual void sem() override { for (const auto &id : idList->getList()) st.insert(id->getName(), std::move(type)); }
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
                d->printAST(out);
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

    void printAST(std::ostream &out) const override {
        out << "Formal(";
        idList->printAST(out);
        out << ", ";
        type->printAST(out);
        out << ")";
    }
        virtual void sem() override { /* ... */ }
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
                f->printAST(out);
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

        virtual void printAST(std::ostream &out) const override {
            out << "Procedure(";
            id->printAST(out);
            out << ", ";
            formalList->printAST(out);
            out << ")";
        }
        virtual void sem() override { /* ... */ }
};


class Function : public Header {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<Type> type;
        std::unique_ptr<FormalList> formalList;
    public:
        Function(std::unique_ptr<Id> i, std::unique_ptr<Type> t, std::unique_ptr<FormalList> fL=nullptr) : id(std::move(i)), type(std::move(t)), formalList(std::move(fL)) {}

        virtual void printAST(std::ostream &out) const {
            out << "Function(";
            id->printAST(out);
            out << ", type=";
            type->printAST(out);
            out << ", ";
            formalList->printAST(out);
            out << ")";
        }
        virtual void sem() override { /* ... */ }
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
                l->printAST(out);
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

        virtual void printAST(std::ostream &out) const override {
            out << "Body(";
            localList->printAST(out);
            out << ", ";
            block->printAST(out);
            out << ")";
        }
        virtual void sem() override { /* ... */ }
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
        virtual void sem() override {
            lVal->sem();
            if (lVal->typeCheck(TYPE_RES)) lVal->type = std::move(st.lookup("result")->type);
            type = std::make_unique<Pointer>(std::move(lVal->type));
        }
};


class Result : public LVal {
    private:
    public:
        Result() {}

        virtual bool isRes() override { return true; }

        virtual void printAST(std::ostream &out) const override { out << "Result()"; }
        virtual void sem() override { type = std::make_unique<TypeRes>(); }
};

class Deref : public LVal {
    private:
        std::unique_ptr<Expr> expr;
    public:
        Deref(std::unique_ptr<Expr> e) : expr(std::move(e)) {}

        virtual void printAST(ostream &out) const override { out << "Deref(" << *expr << ")"; }
        virtual void sem() override {
            expr->sem();
            if (expr->typeCheck(TYPE_RES)) expr->type = std::move(st.lookup("result")->type);
            if (!(expr->typeCheck(TYPE_POINTER))) yyerror("Expression is not a pointer.");
            type = expr->type->getPointerType();
        }
};

#endif