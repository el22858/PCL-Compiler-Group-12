all: distclean pcl

lexer.cpp: lexer.l
	@flex -s -o lexer.cpp lexer.l

lexer.o: lexer.cpp lexer.hpp parser.hpp ast.hpp
	@g++ -c -o lexer.o lexer.cpp

parser.hpp parser.cpp: parser.y
	@bison -dv -o parser.cpp parser.y

parser.o: parser.cpp lexer.hpp ast.hpp
	@g++ -c -o parser.o parser.cpp

pcl: lexer.o parser.o
	@g++ -Wall -o pcl lexer.o parser.o

clean:
	@rm -f lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	@rm -f pcl
