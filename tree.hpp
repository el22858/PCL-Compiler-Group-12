#ifndef TREE_HPP
#define TREE_HPP

#include <iostream>

class AST {
    private:
    public:
        virtual ~AST() = default;
        virtual void sem() = 0;
        virtual void printAST(std::ostream &out) const = 0;
        
        virtual std::string getName() const { return "AST()"; }
};


inline std::ostream &operator<<(std::ostream &out, const AST &ast) {
    ast.printAST(out);
    return out;
}

#endif