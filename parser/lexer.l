%{
	#include "lexglobal.h"
	#include "grammar.h"
%}

%%

(MATCH)|(match) { return MATCH; }
(RETURN)|(return) { return RETURN; }
(WHERE)|(where) { return WHERE; }
(AND)|(and) { return AND; }

[0-9]+ { yylval.n = atoi(yytext); return NUMBER; }

[A-Za-z][A-Za-z0-9]* { yylval.str = strdup(yytext); return STRING; }

"," { return COMMA; }
"(" { return LEFT_PARENTHESIS; }
")" { return RIGHT_PARENTHESIS; }
"[" { return LEFT_BRACKET; }
"]" { return RIGHT_BRACKET; }
">" { return RIGHT_ANGLE_BRACKET; }
"<" { return LEFT_ANGLE_BRACKET; }
"-" { return DASH; }
":" { return COLON; }
"=" { return EQUALS; }
"." { return DOT; }

%%

void yyerror(char *s) {
	printf("error: %s\n", s);
}

int yywrap(void) {
	return 1;
}