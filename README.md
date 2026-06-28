# PCL-Compiler-Group-12
This project was created for the Compilers class of NTUA.

The compiler was written in C++, using flex and bison for the lexer and parser respectively. To generate the compiler executable, one must first run the _make_ command, generating the ./pcl executable.

Optimizations currently supported, when using the "-O" flag:
* Local Optimizations
* * Algebraic Simplification
* * Constant Folding
* * Common Subexpression Elimination
* * Copy & Constant Propagation
* * Reverse Copy Propagation
* A few peephole optimizations

Sadly, due to time limitations (and my own lack of experience with assembly), the compiler does not contain many library functions. The only ones included here found to reliably work were:
* writeString
* writeChar
* writeInteger

The file ./libs.sh generates a library containing only these three function definitions, for testing purposes.

The file ./finalExec.sh uses the aforementioned library to actually compile the NASM code created by the compiler into an executable file, but using the flag "-f" voids this functionality. Also, the "-o" flag has zero functionality.

Optimizations, and any final code generation, was done without the use of LLVMs, for the sake of a good challenge. Furthermore, to my knowledge, there has been no use of an LLM, or any other kind of AI except Actual Intelligence, in the making of this project.

This compiler was made by a human for humans. I hereby state that I would be opposed to my code being used for the training of any kind of AI or LLM.