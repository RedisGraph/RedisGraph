%{

#include "grammar.h"
#include "token.h"
#include <string.h>
#include <math.h>

Token tok;

%}

%%

"AND"		{ return AND; }
"OR"        { return OR; }
"TRUE"      { return TRUE; }
"FALSE"     { return FALSE; }
"MATCH" 	{ return MATCH; }
"RETURN" 	{ return RETURN; }
"WHERE" 	{ return WHERE; }


[\-\+]?[0-9]*\.[0-9]+    {
	tok.dval = atof(yytext);
	return FLOAT; 
}

[\-\+]?[0-9]+    {   
  tok.intval = atoi(yytext); 
  return INTEGER;
}

[A-Za-z][A-Za-z0-9]* {	
  	tok.strval = strdup(yytext);
  	return STRING;
}

"," 	{ return COMMA; }
"(" 	{ return LEFT_PARENTHESIS; }
")" 	{ return RIGHT_PARENTHESIS; }
"[" 	{ return LEFT_BRACKET; }
"]" 	{ return RIGHT_BRACKET; }
">=" 	{ return GE; }
"<=" 	{ return LE; }
"->"	{ return RIGHT_ARROW; }
"!="    {  return NE; }
"=" 	{ return EQ; }
">" 	{ return GT; }
"<" 	{ return LT; }
"-" 	{ return DASH; }
":" 	{ return COLON; }
"." 	{ return DOT; }

[ \t\n]+ /* ignore whitespace */

%%

/**
 * yyerror() is invoked when the lexer or the parser encounter
 * an error. The error message is passed via *s
 *
 *
 */
void yyerror(char *s) {
	printf("error: %s at line: %d\n",s,yylineno);
}

int yywrap(void) {
	return 1;
}