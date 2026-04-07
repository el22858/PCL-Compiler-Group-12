#ifndef TYPE_HPP
#define TYPE_HPP

#include <iostream>
#include <memory>
#include "tree.hpp"

enum Types { TYPE_INTEGER, TYPE_BOOLEAN, TYPE_REAL, TYPE_NIL, TYPE_ARRAY, TYPE_IARRAY, TYPE_CHAR, TYPE_STRING, TYPE_POINTER, TYPE_LABEL, TYPE_RES, TYPE_PROC };

inline std::ostream &operator<<(std::ostream &out, Types t) {
    switch(t) {
    case TYPE_INTEGER:
        out << "int";
        break;
    case TYPE_BOOLEAN:
        out << "boolean";
        break;
    case TYPE_REAL:
        out << "real";
        break;
    case TYPE_NIL:
        out << "nil";
        break;
    case TYPE_ARRAY:
        out << "array";
        break;
    case TYPE_IARRAY:
        out << "array item";
        break;
    case TYPE_CHAR:
        out << "char";
        break;
    default:
        out << "not yet printable, shut up";
        break;
    }
    return out;
}

class Type : public AST {
    protected:
        Types val;
    public:
        virtual void sem() override { /* ... */ }
        virtual Types getType() { return val; }
        virtual std::unique_ptr<Type> getPointerType() { return nullptr; }
};


class Integer : public Type {
    private:
    public:
        Integer() { val = TYPE_INTEGER; }

        virtual void printAST(std::ostream &out) const override { out << "Integer()"; }
        virtual void sem() override { /* ... */ }
};


class String : public Type {
    public:
        String() { val = TYPE_STRING; }

        virtual void printAST(std::ostream &out) const override { out << "String()"; }
        virtual std::string getName() { return "string"; }
        virtual void sem() override { /* ... */ }
};


class Char : public Type {
    public:
        Char() { val = TYPE_CHAR; }

        virtual void printAST(std::ostream &out) const override { out << "Const()"; }
        virtual void sem() override { /* ... */ }
};


class Real : public Type {
    public:
        Real() { val = TYPE_REAL; }

        virtual void printAST(std::ostream &out) const override { out << "Real()"; }
        virtual void sem() override { /* ... */ }
};


class Boolean : public Type {
    public:
        Boolean() { val = TYPE_BOOLEAN; }

        virtual void printAST(std::ostream &out) const override { out << "Boolean()"; }
        virtual void sem() override { /* ... */ }
};


class Array : public Type {
    private:
        std::unique_ptr<Type> arType;
        int size;
    public:
        Array(std::unique_ptr<Type> t, int s = -1) : arType(std::move(t)) { val = TYPE_ARRAY; size = (s > 0) ? s : -1; }

        virtual void printAST(std::ostream &out) const override {
            out << "Array(";
            if (size > 0) out << "size=" << size << ", ";
            out << "type=" << *arType << ")";
        }
        virtual void sem() override { /* ... */ }
};


class Pointer : public Type {
    private:
        std::unique_ptr<Type> pType;
    public:
        Pointer(std::unique_ptr<Type> t) : pType(std::move(t)) { val = TYPE_POINTER; }

        virtual std::unique_ptr<Type> getPointerType() override { return std::move(pType); }
        virtual void printAST(std::ostream &out) const override { out << "Pointer(type=" << *pType << ")"; }
        virtual void sem() override { /* ... */ }
};

class TypeNil : public Type {    public:
        TypeNil() { val = TYPE_NIL; }

        virtual void printAST(std::ostream &out) const override { out << "TypeNil()"; }
        virtual void sem() override { /* ... */ }
};

class TypeRes : public Type {    public:
        TypeRes() { val = TYPE_RES; }

        virtual void printAST(std::ostream &out) const override { out << "TypeRes()"; }
        virtual void sem() override { /* ... */ }
};

class TypeLbl : public Type {    public:
        TypeLbl() { val = TYPE_LABEL; }

        virtual void printAST(std::ostream &out) const override { out << "TypeLabel()"; }
        virtual void sem() override { /* ... */ }
};

class TypeProc : public Type {    public:
        TypeProc() { val = TYPE_PROC; }

        virtual void printAST(std::ostream &out) const override { out << "TypeProcedure()"; }
        virtual void sem() override { /* ... */ }
};

#endif