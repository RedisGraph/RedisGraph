%.c: %.y
parser: lemon
	lex -i lexer.l;\
    ./lemon -s grammar.y

.PHONY: parser

lemon:
	gcc -o lemon lemon.c

all: lemon parser