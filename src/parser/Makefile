%.c: %.y
parser:
	lex -i lexer.l;\
    ./lemon -s grammar.y

.PHONY: parser
all: parser