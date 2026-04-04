#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <iostream>
#include <map>
#include <vector>

#include "basic.hpp"
#include "type.hpp"

struct STEntry {
    Types type;
    int offset;
    STEntry() {}
    STEntry(Types t, int o) : type(t), offset(o) {}
};

class FormalList;
class Stmt;

class Scope {
    private:
        std::map<char *, STEntry> locals;
        std::map<char *, std::unique_ptr<FormalList>> params;
        std::map<char *, Stmt *> lblStmt;
        std::map<char *, bool> isForm, label;
        int offset;
    public:
        Scope() : locals() {}
        Scope(int o) : locals(), offset(o) {}

        void insert(char *id, Types type) {
            if (locals.find(id) != locals.end()) yyerror("Duplicate variable declaration");
            locals[id] = STEntry(type, offset++);
            isForm[id] = false;
        }

        void insertFormal(char *id, Types type, std::unique_ptr<FormalList> fL) {
            if (locals.find(id) != locals.end()) yyerror("Dafuq");
            locals[id] = STEntry(type, offset++);
            isForm[id] = true;
            params[id] = std::move(fL);
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

        bool isLabel(char *c) { return label[c]; }
        bool validLabel(char *c) { return (lblStmt.find(c) != lblStmt.end()); }
};

class SymbolTable {
    private:
        std::vector<Scope> scopes;
    public:
        SymbolTable() : scopes() {}

        void insert(char *id, Types t) { scopes.back().insert(id, t); }
        void insertFormal(char *id, Types t, std::unique_ptr<FormalList> fL) { scopes.back().insertFormal(id, t, std::move(fL)); }

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

        bool isLabel(char *c) { return scopes.back().isLabel(c); }
        bool validLabel(char *c) { return scopes.back().validLabel(c); }
};

extern SymbolTable st;

#endif