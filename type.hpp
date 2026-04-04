#ifndef TYPE_HPP
#define TYPE_HPP

#include <iostream>
#include <memory>
#include "tree.hpp"

enum Types { TYPE_INTEGER, TYPE_BOOLEAN, TYPE_REAL, TYPE_NIL, TYPE_ARRAY, TYPE_IARRAY, TYPE_CHAR, TYPE_STRING, TYPE_POINTER, TYPE_LABEL, TYPE_RES };

class Type : public AST {
    private:
        Types val;
    public:
        virtual void sem() override { /* ... */ }
};


class Integer : public Type {
    private:
        Types val;
    public:
        Integer() : val(TYPE_INTEGER) {}

        virtual void printAST(std::ostream &out) const override { out << "Integer()"; }
        virtual void sem() override { /* ... */ }
};


class String : public Type {
    private:
        Types val;
    public:
        String() : val(TYPE_STRING) {}

        virtual void printAST(std::ostream &out) const override { out << "String()"; }
        virtual void sem() override { /* ... */ }
};


class Char : public Type {
    private:
        Types val;
    public:
        Char() : val(TYPE_CHAR) {}

        virtual void printAST(std::ostream &out) const override { out << "Const()"; }
        virtual void sem() override { /* ... */ }
};


class Real : public Type {
    private:
        Types val;
    public:
        Real() : val(TYPE_REAL) {}

        virtual void printAST(std::ostream &out) const override { out << "Real()"; }
        virtual void sem() override { /* ... */ }
};


class Boolean : public Type {
    private:
        Types val;
    public:
        Boolean() : val(TYPE_BOOLEAN) {}

        virtual void printAST(std::ostream &out) const override { out << "Boolean()"; }
        virtual void sem() override { /* ... */ }
};


class Array : public Type {
    private:
        Types val;
        std::unique_ptr<Type> arType;
        int size;
    public:
        Array(std::unique_ptr<Type> t, int s = -1) : val(TYPE_ARRAY), arType(std::move(t)) { size = (s > 0) ? s : -1; }

        virtual void printAST(std::ostream &out) const override {
            out << "Array(";
            if (size > 0) out << "size=" << size << ", ";
            out << "type=" << *arType << ")";
        }
        virtual void sem() override { /* ... */ }
};


class Pointer : public Type {
    private:
        Types val;
        std::unique_ptr<Type> pType;
    public:
        Pointer(std::unique_ptr<Type> t) : val(TYPE_POINTER), pType(std::move(t)) {}

        virtual void printAST(std::ostream &out) const override { out << "Pointer(type=" << *pType << ")"; }
        virtual void sem() override { /* ... */ }
};

#endif