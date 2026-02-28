lexer.cpp: lexer.l
	@flex -s -o lexer.cpp lexer.l

clean:
	@rm lexer.cpp

distclean: clean
