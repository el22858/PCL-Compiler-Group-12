#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <iostream>
#include <map>
#include <vector>

#include "quads.hpp"
#include "basic.hpp"
#include "type.hpp"

struct STEntry {
	std::shared_ptr<Type> type;
	int offset, n;
	STEntry() {}
	STEntry(std::shared_ptr<Type> t, int o, int depth) : type(t), offset(o), n(depth) {}
};


class FormalList;
class Stmt;

class Scope {
	private:
		std::map<std::string , STEntry> locals;
		std::map<std::string , std::shared_ptr<FormalList>> params;
		std::map<std::string , std::shared_ptr<Stmt>> lblStmt;
		std::map<std::string , bool> isForm, label, isForward, isNewMap, lib, labHasStmt;
		std::vector<std::string> localForp;
		int offset, n;

		std::vector<quad> quadList;
	public:
		Scope() : locals(), localForp() {}
		Scope(int n) : locals(), localForp(), offset(6), n(n) {}

		void printScope(std::ostream &out) {
			for (auto i : locals) out << i.first << " : " << *(i.second.type) << std::endl;
			out << std::endl;
		}

		void insert(std::string id, std::shared_ptr<Type> type, bool isPar) {
			if (locals.find(id) != locals.end()) yyerror("Duplicate variable declaration");
			if (isPar) offset += type->getSize();
			else offset-=type->getSize();
			locals[id] = STEntry(type, offset, n);
			isForm[id] = false;
			isForward[id] = false;
			label[id] = false;
			params[id] = nullptr;
			lib[id] = false;
			labHasStmt[id] = false;
		}

		void insertFormal(std::string id, std::shared_ptr<Type> type, std::shared_ptr<FormalList> fL, bool isLib) {
			if (locals.find(id) != locals.end()) yyerror("Dafuq");
			locals[id] = STEntry(type, 0, n);
			isForm[id] = true;
			isForward[id] = false;
			label[id] = false;
			params[id] = fL;
			localForp.push_back(id);
			lib[id] = isLib;
			labHasStmt[id] = false;
		}
		void insertFormalForward(std::string id, std::shared_ptr<Type> type, std::shared_ptr<FormalList> fL) {
			if (locals.find(id) != locals.end()) yyerror("Dafuq");
			locals[id] = STEntry(type, offset, n);
			isForm[id] = true;
			isForward[id] = true;
			label[id] = false;
			params[id] = fL;
			lib[id] = false;
			labHasStmt[id] = false;
		}

		void insertLabel(std::string id, std::shared_ptr<Type> type) {
			if (locals.find(id) != locals.end()) yyerror("Dafuq");
			locals[id] = STEntry(type, offset, n);
			isForm[id] = false;
			isForward[id] = false;
			label[id] = true;
			params[id] = nullptr;
			lib[id] = false;
			labHasStmt[id] = false;
		}

		STEntry* lookup(std::string id) {
			if (locals.find(id) == locals.end()) return nullptr;
			return &(locals[id]);
		}

		std::shared_ptr<FormalList> getParams(std::string id) { return params[id]; }
		void refreshFormal(std::string id, std::shared_ptr<FormalList> fL) { params[id] = fL; }
		bool isFormal(std::string id) {
			if (lookup(id)) return isForm[id];
			return false;
		}

		int get_offset() { return offset; }

		bool forwarded(std::string c) { return isForward[c]; }
		void backward(std::string c) { isForward[c] = false; }

		bool isLabel(std::string c) { return label[c]; }
		bool validLabel(std::string c) { return (lblStmt.find(c) != lblStmt.end()); }
		void insertLabelStmt(std::string c, std::shared_ptr<Stmt> s) {
			if (!labHasStmt[c]) {
				lblStmt[c] = s;
				labHasStmt[c] = true;
			} else yyerror("Label has already been set.\n");
		}

		bool isLib(std::string c) { return lib[c]; }

		void makeNew(std::string c) { isNewMap[c] = true; }
		bool isNew(std::string c) { return (isNewMap.find(c) != isNewMap.end()); }

		bool hasRes() { return (lookup("result") != nullptr); }

		std::string getParent() {
			if (localForp.size()) return localForp.back();
			else yyerror("Couldn't find parent function.");
			return "";
		}
		void insertParent(std::string c) { localForp.push_back(c); }

		int addTemp(int size) {
			offset -= size;
			return offset;
		}

		void resetOffset() { offset = 0; }
};

class SymbolTable {
	private:
		std::vector<Scope> scopes;
		std::vector<std::string> funcs;
		int n;
	public:
		SymbolTable() : scopes(), funcs(), n(0) {}

		void insert(std::string id, std::shared_ptr<Type> t, bool isParam = false) { scopes.back().insert(id, t, isParam); }
		void insertFormal(std::string id, std::shared_ptr<Type> t, std::shared_ptr<FormalList> fL, bool isLib = false) { scopes.back().insertFormal(id, t, fL, isLib); }
		void insertFormalForward(std::string id, std::shared_ptr<Type> t, std::shared_ptr<FormalList> fL) { scopes.back().insertFormalForward(id, t, fL); }
		void insertLabel(std::string id, std::shared_ptr<Type> t) { scopes.back().insertLabel(id, t); }

		STEntry* lookup(std::string id) {
			for (auto s = scopes.rbegin(); s != scopes.rend(); ++s) {
				STEntry *e = s->lookup(id);
				if (e) return e;
			}

			std::string err = "Variable \"" + id + "\" not found for " + getParent() + ".";
			yyerror(err);
			return nullptr;
		}

		void printTopScope(std::ostream &out) { scopes.back().printScope(out); }

		std::shared_ptr<FormalList> getParams(std::string id) {
			for (auto s = scopes.rbegin(); s!= scopes.rend(); ++s) {
				STEntry *e = s->lookup(id);
				if (e) {
					if (s->isFormal(id)) return s->getParams(id);
					yyerror("Variable named instead of function");
				}
			}
			return nullptr;
		}
		void refreshFormals(std::string c, std::shared_ptr<FormalList> fL) {
			for (auto s = scopes.rbegin(); s!=scopes.rend(); ++s) {
				STEntry *e = s->lookup(c);
				if (e) {
					if (s->isFormal(c)) s->refreshFormal(c, fL);
				}
			}
		}

		void enterScope() { scopes.push_back(Scope(n++)); }
		void exitScope() {
			scopes.pop_back();
			n--;
		}

		bool forwarded(std::string c) { return scopes.back().forwarded(c); }
		void backward(std::string c) { scopes.back().backward(c); }

		bool isLabel(std::string c) { return scopes.back().isLabel(c); }
		bool validLabel(std::string c) { return scopes.back().validLabel(c); }
		void insertLabelStmt(std::string c, std::shared_ptr<Stmt> s) { scopes.back().insertLabelStmt(c, s); }

		bool isLib(std::string c) {
			for (auto s = scopes.rbegin(); s != scopes.rend(); ++s) {
				if (s->lookup(c)) return s->isLib(c);
			}
			return false;
		}

		void makeNew(std::string c) { scopes.back().makeNew(c); }
		bool isNew(std::string c) { return scopes.back().isNew(c); }

		bool isFormal(std::string c) {
			for (auto s = scopes.rbegin(); s != scopes.rend(); ++s){
				if (s->lookup(c)) return s->isFormal(c);
			}
			return false;
		}
		void insertParent(std::string c) {}
		std::string getParent() {
			if (scopes.size() == 1) return scopes.back().getParent();
			else if (scopes.size() >= 2) return scopes[scopes.size() - 2].getParent();
			else yyerror("Couldn't find parent function");
			return "";
		}

		bool hasRes() { return scopes.back().hasRes(); }

		int getDepth() { return scopes.size(); }
		int addTemp(int size) { return scopes.back().addTemp(size); }
		void resetOffset() { scopes.back().resetOffset(); }
		int getOff() { return scopes.back().get_offset(); }
};

extern SymbolTable st;

#endif
