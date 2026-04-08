all: distclean pcl

lexer.cpp lexer.hpp: lexer.l
	@flex $<

lexer.o: lexer.cpp parser.hpp ast.hpp quads.hpp
	@g++ -c -o lexer.o lexer.cpp

parser.hpp parser.cpp: parser.y
	@bison -dv -o parser.cpp parser.y

parser.o: parser.cpp lexer.hpp ast.hpp quads.hpp
	@g++ -c -o parser.o parser.cpp

pcl: lexer.o parser.o
	@g++ -Wall -o pcl lexer.o parser.o

clean:
	@clear
	@rm -f lexer.cpp lexer.hpp parser.cpp parser.hpp parser.output *.o location.hpp *.imm

distclean: clean
	@rm -f pcl
