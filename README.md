# PCL-Compiler-Group-12
This project was created for the Compilers class of NTUA.

The compiler was written in C++, using flex and bison for the lexer and parser respectively.

Optimizations currently supported:
* Local Optimizations
* * Algebraic Simplification
* * Constant Folding
* * Common Subexpression Elimination
* * Copy & Constant Propagation
* * Reverse Copy Propagation
* A few peephole optimizations