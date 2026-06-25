#ifndef TREE_HPP
#define TREE_HPP

#include <iostream>

class AST {
    protected:
		int size, depth;
    public:
        virtual ~AST() = default;
        virtual void sem() = 0;
        virtual void printAST(std::ostream &out) const = 0;
        
        virtual std::string getName() const { return "AST()"; }

        virtual void igen() {}
        
		void setSize(int n) { size = n; }
		void setDepth(int n) { depth = n; }
};


inline std::ostream &operator<<(std::ostream &out, const AST &ast) {
    ast.printAST(out);
    return out;
}

#endif