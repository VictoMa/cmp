sources=src/main.cpp \
	temp/parser.cpp \
	temp/token.cpp \
	src/codeGen/expression.cpp \
	src/codeGen/AstNode.cpp \
	src/codeGen/statement.cpp \
	src/codeGen/AstFunc.cpp \


headers=include/expression.h \
	include/share.h \
	include/AstNode.h \
	include/statement.h \
	include/AstFunc.h \
	temp/parser.hpp


no_clean:compile_lib parser lex compile 

all:compile_lib parser lex compile clean_temp

clean_temp:
	rm -rf temp
clean:
	rm -rf temp lib compiler a*

compile_lib:src/lib/io.c
	mkdir lib -p
	gcc -c src/lib/io.c -o lib/io.o -Igc/include -std=c99

parser:src/parser/parser.y
	mkdir temp -p
	bison -d -o temp/parser.cpp $< -r all

lex:src/parser/token.l
	mkdir temp -p
	flex -o temp/token.cpp $<

compile:$(sources) $(headers)
	g++ -o compiler $(sources) -Iinclude -Itemp `llvm-config --cxxflags \
	--ldflags --libs core native all-targets asmparser`

