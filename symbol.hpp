#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <iostream>
#include <map>
#include <vector>

#include "basic.hpp"
#include "type.hpp"

struct STEntry {
    std::unique_ptr<Type> type;
    int offset;
    STEntry() {}
    STEntry(std::unique_ptr<Type> t, int o) : type(std::move(t)), offset(o) {}
};

class FormalList;
class Stmt;

class Scope {
    private:
        std::map<char *, STEntry> locals;
        std::map<char *, std::unique_ptr<FormalList>> params;
        std::map<char *, std::unique_ptr<Stmt>> lblStmt;
        std::map<char *, bool> isForm, label, isForward;
        int offset;
    public:
        Scope() : locals() {}
        Scope(int o) : locals(), offset(o) {}

        void insert(char *id, std::unique_ptr<Type> type) {
            if (locals.find(id) != locals.end()) yyerror("Duplicate variable declaration");
            locals[id] = STEntry(std::move(type), offset++);
            isForm[id] = false;
            isForward[id] = false;
            label[id] = false;
            params[id] = nullptr;
        }

        void insertFormal(char *id, std::unique_ptr<Type> type, std::unique_ptr<FormalList> fL) {
            if (locals.find(id) != locals.end()) yyerror("Dafuq");
            locals[id] = STEntry(std::move(type), offset++);
            isForm[id] = true;
            isForward[id] = false;
            label[id] = false;
            params[id] = std::move(fL);
        }
        void insertFormalForward(char *id, std::unique_ptr<Type> type, std::unique_ptr<FormalList> fL) {
            if (locals.find(id) != locals.end()) yyerror("Dafuq");
            locals[id] = STEntry(std::move(type), offset++);
            isForm[id] = true;
            isForward[id] = true;
            label[id] = false;
            params[id] = std::move(fL);
        }

        void insertLabel(char *id, std::unique_ptr<Type> type) {
            if (locals.find(id) != locals.end()) yyerror("Dafuq");
            locals[id] = STEntry(std::move(type), offset++);
            isForm[id] = false;
            isForward[id] = false;
            label[id] = true;
            params[id] = nullptr;
        }

        STEntry* lookup(char *id) {
            if (locals.find(id) == locals.end()) return nullptr;
            return &(locals[id]);
        }

        std::unique_ptr<FormalList> getParams(char *id) { return std::move(params[id]); }
        bool isFormal(char *id) {
            if (lookup(id)) return isForm[id];
            return false;
        }

        int get_offset() { return offset; }

        bool forwarded(char *c) { return isForward[c]; }
        void backward(char *c) { isForward[c] = false; }

        bool isLabel(char *c) { return label[c]; }
        bool validLabel(char *c) { return (lblStmt.find(c) != lblStmt.end()); }
        void insertLabelStmt(char *c, std::unique_ptr<Stmt> s) { lblStmt[c] = std::move(s); }
};

class SymbolTable {
    private:
        std::vector<Scope> scopes;
    public:
        SymbolTable() : scopes() {}

        void insert(char *id, std::unique_ptr<Type> t) { scopes.back().insert(id, std::move(t)); }
        void insertFormal(char *id, std::unique_ptr<Type> t, std::unique_ptr<FormalList> fL) { scopes.back().insertFormal(id, std::move(t), std::move(fL)); }
        void insertFormalForward(char *id, std::unique_ptr<Type> t, std::unique_ptr<FormalList> fL) { scopes.back().insertFormalForward(id, std::move(t), std::move(fL)); }
        void insertLabel(char *id, std::unique_ptr<Type> t) { scopes.back().insertLabel(id, std::move(t)); }

        STEntry* lookup(char* id) {
            for (auto s = scopes.rbegin(); s != scopes.rend(); ++s) {
                STEntry *e = s->lookup(id);
                if (e) return e;
            }

            // yyerror("Variable not found.");
            return nullptr;
        }

        std::unique_ptr<FormalList> getParams(char *id) {
            for (auto s = scopes.rbegin(); s!= scopes.rend(); ++s) {
                STEntry *e = s->lookup(id);
                if (e) {
                    if (s->isFormal(id)) return s->getParams(id);
                    yyerror("Variable named instead of function");
                    return nullptr;
                }
            }
            return nullptr;
        }

        void enterScope() {
            int o = scopes.empty() ? 0 : scopes.back().get_offset();
            scopes.push_back(Scope(o));
        }
        void exitScope() { scopes.pop_back(); }

        bool forwarded(char *c) { return scopes.back().forwarded(c); }
        void backward(char *c) { scopes.back().backward(c); }

        bool isLabel(char *c) { return scopes.back().isLabel(c); }
        bool validLabel(char *c) { return scopes.back().validLabel(c); }
        void insertLabelStmt(char *c, std::unique_ptr<Stmt> s) { scopes.back().insertLabelStmt(c, std::move(s)); }
};

extern SymbolTable st;

#endif