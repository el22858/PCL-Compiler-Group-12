all: distclean pcl

src/lexer.cpp src/lexer.hpp: src/lexer.l
	@flex -s -o src/lexer.cpp src/lexer.l

src/lexer.o: src/lexer.cpp src/parser.hpp src/ast.hpp src/quads.hpp
	@g++ -c -o src/lexer.o src/lexer.cpp

src/parser.hpp src/parser.cpp: src/parser.y
	@bison -dv -o src/parser.cpp src/parser.y

src/parser.o: src/parser.cpp src/lexer.hpp src/ast.hpp src/quads.hpp
	@g++ -c -o src/parser.o src/parser.cpp

pcl: src/lexer.o src/parser.o
	@g++ -Wall -o pcl src/main.cpp src/lexer.o src/parser.o

clean:
	@clear
	@rm -f src/lexer.cpp src/lexer.hpp src/parser.cpp src/parser.hpp src/parser.output src/*.o src/location.hpp *.imm *asm

distclean: clean
	@rm -f pcl lib.* *.o
