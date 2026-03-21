#ifndef TYPE_HPP
#define TYPE_HPP

#include <iostream>
#include "tree.hpp"

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

        virtual void printAST(std::ostream &out) const override { out << "Integer()"; }
};


class String : public Type {
    private:
        Types val;
    public:
        String() : val(TYPE_STRING) {}

        virtual void printAST(std::ostream &out) const override { out << "String()"; }
};


class Char : public Type {
    private:
        Types val;
    public:
        Char() : val(TYPE_CHAR) {}

        virtual void printAST(std::ostream &out) const override { out << "Const()"; }
};


class Real : public Type {
    private:
        Types val;
    public:
        Real() : val(TYPE_REAL) {}

        virtual void printAST(std::ostream &out) const override { out << "Real()"; }
};


class Boolean : public Type {
    private:
        Types val;
    public:
        Boolean() : val(TYPE_BOOLEAN) {}

        virtual void printAST(std::ostream &out) const override { out << "Boolean()"; }
};


class Array : public Type {
    private:
        Types val;
        Type *arType;
        int size;
    public:
        Array(Type *t, int s = -1) : val(TYPE_ARRAY), arType(t) { size = (s > 0) ? s : -1; }

        virtual void printAST(std::ostream &out) const override {
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

        virtual void printAST(std::ostream &out) const override { out << "Pointer(type=" << *pType << ")"; }
};

#endif