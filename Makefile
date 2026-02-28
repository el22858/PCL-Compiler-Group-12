lexer.cpp: lexer.l
	@flex -s -o lexer.cpp lexer.l

lexer: lexer.cpp
	@g++ -o lexer lexer.cpp

clean:
	@rm lexer.cpp

distclean: clean
	@rm lexer
