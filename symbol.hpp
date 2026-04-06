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
        std::map<std::string , STEntry> locals;
        std::map<std::string , std::unique_ptr<FormalList>> params;
        std::map<std::string , std::unique_ptr<Stmt>> lblStmt;
        std::map<std::string , bool> isForm, label, isForward, isNewMap;
        std::vector<std::string> localForp;
        int offset;
    public:
        Scope() : locals(), localForp() {}
        Scope(int o) : locals(), localForp(), offset(o) {}

        void insert(std::string id, std::unique_ptr<Type> type) {
            if (locals.find(id) != locals.end()) yyerror("Duplicate variable declaration");
            locals[id] = STEntry(std::move(type), offset++);
            isForm[id] = false;
            isForward[id] = false;
            label[id] = false;
            params[id] = nullptr;
        }

        void insertFormal(std::string id, std::unique_ptr<Type> type, std::unique_ptr<FormalList> fL) {
            if (locals.find(id) != locals.end()) yyerror("Dafuq");
            locals[id] = STEntry(std::move(type), offset++);
            isForm[id] = true;
            isForward[id] = false;
            label[id] = false;
            params[id] = std::move(fL);
            localForp.push_back(id);
        }
        void insertFormalForward(std::string id, std::unique_ptr<Type> type, std::unique_ptr<FormalList> fL) {
            if (locals.find(id) != locals.end()) yyerror("Dafuq");
            locals[id] = STEntry(std::move(type), offset++);
            isForm[id] = true;
            isForward[id] = true;
            label[id] = false;
            params[id] = std::move(fL);
        }

        void insertLabel(std::string id, std::unique_ptr<Type> type) {
            if (locals.find(id) != locals.end()) yyerror("Dafuq");
            locals[id] = STEntry(std::move(type), offset++);
            isForm[id] = false;
            isForward[id] = false;
            label[id] = true;
            params[id] = nullptr;
        }

        STEntry* lookup(std::string id) {
            if (locals.find(id) == locals.end()) return nullptr;
            return &(locals[id]);
        }

        std::unique_ptr<FormalList> getParams(std::string id) { return std::move(params[id]); }
        bool isFormal(std::string id) {
            if (lookup(id)) return isForm[id];
            return false;
        }

        int get_offset() { return offset; }

        bool forwarded(std::string c) { return isForward[c]; }
        void backward(std::string c) { isForward[c] = false; }

        bool isLabel(std::string c) { return label[c]; }
        bool validLabel(std::string c) { return (lblStmt.find(c) != lblStmt.end()); }
        void insertLabelStmt(std::string c, std::unique_ptr<Stmt> s) { lblStmt[c] = std::move(s); }

        void makeNew(std::string c) { isNewMap[c] = true; }
        bool isNew(std::string c) { return (isNewMap.find(c) != isNewMap.end()); }

        bool hasRes() { return (lookup("result") != nullptr); }

        std::string getParent() {
            if (localForp.size()) return localForp.back();
            else yyerror("Couldn't find parent function.");
        }
        void insertParent(std::string c) { localForp.push_back(c); }
};

class SymbolTable {
    private:
        std::vector<Scope> scopes;
    public:
        SymbolTable() : scopes() {}

        void insert(std::string id, std::unique_ptr<Type> t) { scopes.back().insert(id, std::move(t)); }
        void insertFormal(std::string id, std::unique_ptr<Type> t, std::unique_ptr<FormalList> fL) { scopes.back().insertFormal(id, std::move(t), std::move(fL)); }
        void insertFormalForward(std::string id, std::unique_ptr<Type> t, std::unique_ptr<FormalList> fL) { scopes.back().insertFormalForward(id, std::move(t), std::move(fL)); }
        void insertLabel(std::string id, std::unique_ptr<Type> t) { scopes.back().insertLabel(id, std::move(t)); }

        STEntry* lookup(std::string id) {
            for (auto s = scopes.rbegin(); s != scopes.rend(); ++s) {
                STEntry *e = s->lookup(id);
                if (e) return e;
            }

            std::string err = "Variable \"" + id + "\" not found.";
            yyerror(err);
            return nullptr;
        }

        std::unique_ptr<FormalList> getParams(std::string id) {
            for (auto s = scopes.rbegin(); s!= scopes.rend(); ++s) {
                STEntry *e = s->lookup(id);
                if (e) {
                    if (s->isFormal(id)) return std::move(s->getParams(id));
                    yyerror("Variable named instead of function");
                }
            }
            return nullptr;
        }

        void enterScope() {
            int o = scopes.empty() ? 0 : scopes.back().get_offset();
            scopes.push_back(Scope(o));
        }
        void exitScope() { scopes.pop_back(); }

        bool forwarded(std::string c) { return scopes.back().forwarded(c); }
        void backward(std::string c) { scopes.back().backward(c); }

        bool isLabel(std::string c) { return scopes.back().isLabel(c); }
        bool validLabel(std::string c) { return scopes.back().validLabel(c); }
        void insertLabelStmt(std::string c, std::unique_ptr<Stmt> s) { scopes.back().insertLabelStmt(c, std::move(s)); }

        void makeNew(std::string c) { scopes.back().makeNew(c); }
        bool isNew(std::string c) { return scopes.back().isNew(c); }

        bool isFormal(std::string c) {
            for (auto s = scopes.rbegin(); s != scopes.rend(); ++s){
                if (s->lookup(c)) return s->isFormal(c);
            }
            return false;
        }
        // FormalList getFormals(std::string c) { return scopes.back().; }
        void insertParent(std::string c) {}
        std::string getParent() {
            if (scopes.size() == 1) return scopes.back().getParent();
            else if (scopes.size() >= 2) return scopes[scopes.size() - 2].getParent();
            else yyerror("Couldn't find parent function");
        }

        bool hasRes() { return scopes.back().hasRes(); }
};

extern SymbolTable st;

#endif