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

class Scope {
    private:
        std::map<char *, STEntry> locals;
        int offset;
    public:
        Scope() : locals() {}
        Scope(int o) : locals(), offset(o) {}

        void insert(char *id, Types type) {
            // if (locals.find(id) != locals.end()) yyerror("Duplicate variable declaration");
            locals[id] = STEntry(type, offset++);
        }
        STEntry* lookup(char *id) {
            if (locals.find(id) == locals.end()) return nullptr;
            return &(locals[id]);
        }
        int get_offset() { return offset; }
};

class SymbolTable {
    private:
        std::vector<Scope> scopes;
    public:
        SymbolTable() : scopes() {}

        void insert(char *id, Types t) { scopes.back().insert(id, t); }
        STEntry* lookup(char* id) {
            for (auto s = scopes.rbegin(); s != scopes.rend(); ++s) {
                STEntry *e = s->lookup(id);
                if (e) return e;
            }

            // yyerror("Variable not found.");
            return nullptr;
        }

        void enterScope() {
            int o = scopes.empty() ? 0 : scopes.back().get_offset();
            scopes.push_back(Scope(o));
        }
        void exitScope() { scopes.pop_back(); }
};

extern SymbolTable st;

#endif