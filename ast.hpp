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
        virtual void sem() override {
            for (auto &s : stmt_list) {
                if (s) s->sem();
            }
        }

        void append(std::unique_ptr<Stmt> s) { /*stmt_list.insert(stmt_list.begin(), std::move(s));*/ stmt_list.push_back(std::move(s)); }
        void appendAtStart(std::unique_ptr<Stmt> s) { stmt_list.insert(stmt_list.begin(), std::move(s)); }
};


class Expr : public AST {
    protected:
    public:
        std::shared_ptr<Type> type;
        std::string place;
        virtual bool isRes() { return false; }
        Types getType() { return type->getType(); }
        bool typeCheck(Types t) { return type->getType() == t; }
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
        virtual std::string getName() const override {
            std::string res = "While(";
            res += expr->getName();
            res += ", ";
            res += stmt->getName();
            res += ")";
            return res;
        }
        virtual void sem() override {
            int whileLine = st.quadNEXTQUAD();
            expr->sem();
            
            if (expr->typeCheck(TYPE_BOOLEAN)) {
                st.quadBACKPATCH(whileLine, std::to_string(st.quadNEXTQUAD()));

                stmt->sem();
            }
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
        char* op;
        std::unique_ptr<Expr> expr;
    public:
        UnOp(char* s, std::unique_ptr<Expr> e) : op(s), expr(std::move(e)) {}

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
        virtual void sem() override {
            expr->sem();

            if (strcmp(op, "+") == 0) {
                if (expr->typeCheck(TYPE_INTEGER)) type = std::make_shared<Integer>();
                else if (expr->typeCheck(TYPE_REAL)) type = std::make_shared<Real>();
                else yyerror("Expected number");

                place = expr->place;
            } else if (strcmp(op, "-") == 0) {
                if (expr->typeCheck(TYPE_INTEGER)) type = std::make_shared<Integer>();
                else if (expr->typeCheck(TYPE_REAL)) type = std::make_shared<Real>();
                else yyerror("Expected number");

                place = "$" + std::to_string(quadNEWTEMP());
                st.quadGENQUAD(op, expr->place, "-", place);
            } else if (strcmp(op, "not") == 0) {
                if (expr->typeCheck(TYPE_BOOLEAN)) type = std::make_shared<Boolean>();
                else yyerror("Expected boolean.");

                place = "$" + std::to_string(quadNEWTEMP());
                st.quadGENQUAD(op, expr->place, "-", std::to_string(st.quadNEXTQUAD()+2));
                st.quadGENQUAD("jump", "-", "-", "*");
            } else yyerror("Unary Operator not recognized"); // Trash
        }
};


class BinOp : public RVal {
    private:
        std::unique_ptr<Expr> expr1, expr2;
        char* op;
    public:
        BinOp(std::unique_ptr<Expr> e1, char* s, std::unique_ptr<Expr> e2) : expr1(std::move(e1)), op(s), expr2(std::move(e2)) {}

        virtual void printAST(std::ostream &out) const override {
            out << "BinOp(";
            expr1->printAST(out);
            out << ", " << op << ", ";
            expr2->printAST(out);
            out << ")";
        }
        virtual std::string getName() const override {
            std::string res = "BinOp(";
            res += expr1->getName();
            res += ", ";
            res += op;
            res += ", ";
            res += expr2->getName();
            res += ")";
            return res;
        } 
        virtual void sem() override {
            expr1->sem();
            expr2->sem();

            if ((strcmp(op, "+") == 0) || (strcmp(op, "-") == 0) || (strcmp(op, "*") == 0)) {
                if (expr1->typeCheck(TYPE_INTEGER)) {
                    if (expr2->typeCheck(TYPE_INTEGER)) type = std::make_shared<Integer>();
                    else if (expr2->typeCheck(TYPE_REAL)) type = std::make_shared<Real>();
                    else yyerror("Expected int or real.");
                } else if (expr1->typeCheck(TYPE_REAL)) {
                    if ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL))) type = std::make_shared<Real>();
                    else yyerror("Expected int or real.");
                }

                place = "$" + std::to_string(quadNEWTEMP());
                st.quadGENQUAD(op, expr1->place, expr2->place, place);
            } else if (strcmp(op, "/") == 0) {
                if ((expr1->typeCheck(TYPE_INTEGER)) || ((expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = std::make_shared<Real>();
                else yyerror("Expected int or real.");

                place = "$" + std::to_string(quadNEWTEMP());
                st.quadGENQUAD(op, expr1->place, expr2->place, place);
            } else if ((strcmp(op, "div") == 0) || (strcmp(op, "mod") == 0)) {
                if ((expr1->typeCheck(TYPE_INTEGER)) && (expr2->typeCheck(TYPE_INTEGER))) type = std::make_shared<Integer>();
                else yyerror("Expected int.");

                place = "$" + std::to_string(quadNEWTEMP());
                st.quadGENQUAD(op, expr1->place, expr2->place, place);
            } else if ((strcmp(op, "and") == 0) || (strcmp(op, "or") == 0)) {
                if ((expr1->typeCheck(TYPE_BOOLEAN) && (expr2->typeCheck(TYPE_BOOLEAN)))) type = std::make_shared<Boolean>();
                else yyerror("Expected boolean.");
            } else if ((strcmp(op, "<") == 0) || (strcmp(op, ">") == 0) || (strcmp(op, "<=") == 0) || (strcmp(op, ">=") == 0)) {
                if (((expr1->typeCheck(TYPE_INTEGER)) || (expr1->typeCheck(TYPE_REAL))) && ((expr2->typeCheck(TYPE_INTEGER)) || (expr2->typeCheck(TYPE_REAL)))) type = std::make_shared<Boolean>();
                else yyerror("Expected int or real.");
            } else if ((strcmp(op, "=") == 0) || (strcmp(op, "<>") == 0)) {
                if (expr1->type == expr2->type) type = std::make_shared<Boolean>();
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
        virtual std::string getName() const override { return "IntConst(" + std::to_string(val) + ")"; }
        virtual void sem() override {
            type = std::make_shared<Integer>();

            place = std::to_string(val);
        }
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
        }
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
        }
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
        }
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
        }
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
            if (lVal->typeCheck(TYPE_RES)) lVal->type = std::move(st.lookup("result")->type);
            
            expr->sem();
            if (expr->typeCheck(TYPE_RES)) expr->type = std::move(st.lookup("result")->type);
            
            if (!(lVal->typeCheck(TYPE_ARRAY))) {
                std::cout<<"BRUH"<<std::endl;
                exit(0);
            }
            else if (!(expr->typeCheck(TYPE_INTEGER))) yyerror("Expected integer");

            type = std::move(lVal->type);

            place = quadNEWTEMP();
            st.quadGENQUAD("array", lVal->place, expr->place, place);
        }
};


class Formal : public AST {
    private:
        std::unique_ptr<IdList> idList;
        std::shared_ptr<Type> type;
    public:
    Formal(std::unique_ptr<IdList> iL, std::shared_ptr<Type> t) : idList(std::move(iL)), type(t) {}

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
    Types getType() { return type->getType(); }
    virtual void sem() override { for (const auto &id : idList->getList()) st.insert(id->getId(), type); }
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
};


class Call : public Stmt {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<ExprList> func;
    public:
        Call(std::unique_ptr<Id> i, std::unique_ptr<ExprList> eL=std::move(std::make_unique<ExprList>())) : id(std::move(i)), func(std::move(eL)) {}

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
            
            std::unique_ptr<FormalList> fL;
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
                        if ((f->getType()) != (func->getList()[i]->type->getType())) yyerror("Type mismatch.");
                        
                        st.quadGENQUAD("par", func->getList()[i++]->place, "V", "-");
                    }
                }
            }

            st.quadGENQUAD("call", "-", "-", funcName);

            st.refreshFormals(funcName, std::move(fL));
        }
};


class CallRVal : public RVal {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<ExprList> func;
    public:
        CallRVal(std::unique_ptr<Id> i, std::unique_ptr<ExprList> eL=std::move(std::make_unique<ExprList>())) : id(std::move(i)), func(std::move(eL)) {}

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

            std::unique_ptr<FormalList> fL;
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
                        if ((f->getType()) != (func->getList()[i]->type->getType())) yyerror("Type mismatch.");
                        
                        st.quadGENQUAD("par", func->getList()[i++]->place, "V", "-");
                    }
                }
            }
            type = std::move(st.lookup(funcName)->type);

            place = "$" + std::to_string(quadNEWTEMP());
            st.quadGENQUAD("par", place, "RET", "-");
            st.quadGENQUAD("call", "-", "-", funcName);
            // place = "$$";

            st.refreshFormals(funcName, std::move(fL));
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
            if (lVal->typeCheck(TYPE_RES)) lVal->type = std::move(st.lookup("result")->type);
            if (!(lVal->typeCheck(TYPE_POINTER))) yyerror("Expected pointer.");
            if (!st.isNew(lVal->getName())) yyerror("Cannot dispose of non-new value.");
            
            if (bracket && (lVal->type->getPointerType()->getType() != TYPE_ARRAY)) yyerror("Something something pointer to array.");

            lVal = std::move(std::make_unique<NilLVal>());
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
            } else if (lval->type->getType() != expr->type->getType()) yyerror("Assignment error.");

            st.quadGENQUAD(":=", expr->place, "-", lval->place);
        }
};


class Return: public Stmt {
    public:
        Return() {};

        virtual void printAST(std::ostream &out) const override { out << "Return()"; }
        virtual std::string getName() const override { return "Return()"; }
        virtual void sem() override {}
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
};


class Header : public AST {
    public:
        virtual void semForward() {}
};


class Procedure : public Header {
    private:
        std::unique_ptr<Id> id;
        std::unique_ptr<FormalList> formalList;
    public:
        Procedure(std::unique_ptr<Id> i, std::unique_ptr<FormalList> fL = std::move(std::make_unique<FormalList>())) : id(std::move(i)), formalList(std::move(fL)) {}

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
            formalList->sem();
            if (st.forwarded(id->getId())) {
                std::string oldName="", newName="";
                std::unique_ptr<FormalList> oldFormals = std::move(st.getParams(id->getId()));
                if (oldFormals) oldName = oldFormals->getName();
                if (!(formalList->getList().empty())) newName = formalList->getName();

                if(oldName.compare(newName)) yyerror("Procedure has two different declarations.");

                st.backward(id->getId());
                st.refreshFormals(id->getId(), std::move(oldFormals));
                st.insertParent(id->getId());
            } else st.insertFormal(id->getId(), std::make_unique<TypeProc>(), std::move(formalList));

            // st.quadGENQUAD("unit", id->getId(), "-", "-");
            // st.quadGENQUAD("endu", id->getId(), "-", "-");
        }
        virtual void semForward() override { st.insertFormalForward(id->getId(), std::make_unique<TypeProc>(), std::move(formalList)); }
};


class Function : public Header {
    private:
        std::unique_ptr<Id> id;
        std::shared_ptr<Type> type;
        std::unique_ptr<FormalList> formalList;
    public:
        Function(std::unique_ptr<Id> i, std::shared_ptr<Type> t, std::unique_ptr<FormalList> fL = std::move(std::make_unique<FormalList>())) : id(std::move(i)), type(t), formalList(std::move(fL)) {}

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
                std::unique_ptr<FormalList> oldFormals = std::move(st.getParams(id->getId()));
                if (oldFormals) oldName = oldFormals->getName();
                if (!(formalList->getList().empty())) newName = formalList->getName();

                if(oldName.compare(newName)) yyerror("Function has two different declarations.");

                st.backward(id->getId());
                st.refreshFormals(id->getId(), std::move(oldFormals));
                st.insertParent(id->getId());
            } else st.insertFormal(id->getId(), type, std::move(formalList));
        }
        virtual void semForward() override { st.insertFormalForward(id->getId(), type, std::move(formalList)); }

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
        }
        virtual void sem() override {
            if (localType.compare("var") == 0) dList->sem();
            else if (localType.compare("label") == 0) lbl->sem();
            else if (localType.compare("forp") == 0) {
                hdr->sem();
                st.enterScope();
                body->sem();
                st.exitScope();
            } else if (localType.compare("forward") == 0) hdr->semForward();
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

            st.enterScope();
            localList->sem();
            st.quadGENQUAD("unit", name, "-", "-");
            block->sem();
            st.quadGENQUAD("endu", name, "-", "-");
            st.exitScope();
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
        }
};


class Result : public LVal {
    private:
    public:
        Result() {}

        virtual bool isRes() override { return true; }

        virtual void printAST(std::ostream &out) const override { out << "Result()"; }
        virtual std::string getName() const override { return "Result()"; }
        virtual void sem() override { type = std::make_shared<TypeRes>(); }
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
        }
};


#endif