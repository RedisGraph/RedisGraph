%{

#include "grammar.h"
#include "token.h"
#include <string.h>
#include <math.h>

Token tok;


/* handle locations */
int yycolumn = 1;

#define YY_USER_ACTION yycolumn += yyleng; \
    tok.pos = yycolumn; \
    tok.s = strdup(yytext);
%}

%%

"AND"		    { return AND; }
"OR"        { return OR; }
"TRUE"      { return TRUE; }
"FALSE"     { return FALSE; }
"MATCH" 	  { return MATCH; }
"CREATE" 	  { return CREATE; }
"DELETE" 	  { return DELETE; }
"RETURN" 	  { return RETURN; }
"AS" 	      { return AS; }
"DISTINCT"  { return DISTINCT; }
"WHERE" 	  { return WHERE; }
"ORDER"     { return ORDER; }
"BY"        { return BY; }
"ASC"       { return ASC; }
"DESC"      { return DESC; }
"LIMIT"     { return LIMIT; }


[0-9]*\.[0-9]+    {
	tok.dval = atof(yytext);
	return FLOAT; 
}

[0-9]+    {
  tok.intval = atoi(yytext); 
  return INTEGER;
}

[_A-Za-z][A-Za-z0-9_-]* {
  	tok.strval = strdup(yytext);
  	return STRING;
}

(\"(\\.|[^\"])*\")|('(\\.|[^'])*')    {
  /* String literals, with escape sequences - enclosed by "" or '' */
  *(yytext+strlen(yytext)-1) = '\0';
  tok.strval = strdup(yytext+1);
  return STRING;
}

"," 	{ return COMMA; }
"(" 	{ return LEFT_PARENTHESIS; }
")" 	{ return RIGHT_PARENTHESIS; }
"[" 	{ return LEFT_BRACKET; }
"]" 	{ return RIGHT_BRACKET; }
"{" 	{ return LEFT_CURLY_BRACKET; }
"}" 	{ return RIGHT_CURLY_BRACKET; }
">=" 	{ return GE; }
"<=" 	{ return LE; }
"->"	{ return RIGHT_ARROW; }
"<-"	{ return LEFT_ARROW; }
"!="  {  return NE; }
"=" 	{ return EQ; }
">" 	{ return GT; }
"<" 	{ return LT; }
"-" 	{ return DASH; }
":" 	{ return COLON; }
"." 	{ return DOT; }
"/"   { return DIV; }
"*"   { return MUL; }
"+"   { return ADD; }

[ \t]+ /* ignore whitespace */
\n    { yycolumn = 1; } /* ignore whitespace */

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