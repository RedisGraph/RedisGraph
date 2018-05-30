%.c: %.y
parser:
	lex -i lexer.l;\
    ./lemon -s grammar.y

.PHONY: parser

lemon:
	gcc -o lemon lemon.c

all: parser