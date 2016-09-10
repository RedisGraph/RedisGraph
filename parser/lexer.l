%{
	#include "lexglobal.h"
	#include "grammar.h"
%}

%%

[0-9]+ { yylval.n = atoi(yytext); return NUMBER; }

[A-Za-z][A-Za-z0-9]* { yylval.str = strdup(yytext); return STRING; }

"(" { return LEFT_PARENTHESIS; }
")" { return RIGHT_PARENTHESIS; }
"[" { return LEFT_BRACKET; }
"]" { return RIGHT_BRACKET; }
"{" { return LEFT_CURLY_BRACKET; }
"}" { return RIGHT_CURLY_BRACKET; }
">" { return ARROW_RIGHT; }
"-" { return DASH; }
":" { return COLON; }

.	{ return yytext[0]; }

%%

void yyerror(char *s) {
	printf("error: %s\n", s);
}

int yywrap(void) {
	return 1;
}