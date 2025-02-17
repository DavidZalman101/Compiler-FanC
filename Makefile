.PHONY: all clean

CC = g++
CFLAGS = -std=c++17

all: clean
	flex scanner.lex
	bison -Wcounterexamples -d parser.y
	$(CC) $(CFLAGS) -o compiler *.c *.cpp
clean:
	rm -f lex.yy.* parser.tab.* compiler
