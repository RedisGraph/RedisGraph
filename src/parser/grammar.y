%left OR.
%left AND.
%nonassoc EQ GT GE LT LE.

%token_type {Token}

%syntax_error {
	//printf("Syntax error!\n");
}

%include {
	#include <stdlib.h>
	#include <stdio.h>
	#include <assert.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"

	void yyerror(char *s);
} // END %include

%extra_argument { QueryExpressionNode **root }

query ::= expr(A). { *root = A; }


%type expr {QueryExpressionNode*}

expr(A) ::= matchClause(B) whereClause(C) returnClause(D) orderClause(E) limitClause(F). {
	A = NewQueryExpressionNode(B, C, D, E, F);
}


%type matchClause { MatchNode* }

matchClause(A) ::= MATCH chain(B). {
	A = NewMatchNode(B);
}


%type chain {Vector*}

chain(A) ::= node(B). {
	A = NewVector(ChainElement*, 1);
	Vector_Push(A, B);
} 

chain(A) ::= chain(B) link(C) node(D). {
	Vector_Push(B, C);
	Vector_Push(B, D);
	A = B;
}


%type node {ChainElement*}

node(A) ::= LEFT_PARENTHESIS STRING(B) COLON STRING(C) RIGHT_PARENTHESIS. {
	A = NewChainEntity(B.strval, C.strval);
}
node(A) ::= LEFT_PARENTHESIS COLON STRING(B) RIGHT_PARENTHESIS. {
	A = NewChainEntity(NULL, B.strval);
}
node(A) ::= LEFT_PARENTHESIS STRING(B) RIGHT_PARENTHESIS. {
	A = NewChainEntity(B.strval, NULL);
}
node(A) ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS. {
	A = NewChainEntity(NULL, NULL);
}


%type link {ChainElement*}

link(A) ::= DASH LEFT_BRACKET RIGHT_BRACKET RIGHT_ARROW. { 
	A =  NewChainLink("", N_LEFT_TO_RIGHT); 
}
link(A) ::= DASH LEFT_BRACKET STRING(B) RIGHT_BRACKET RIGHT_ARROW. { 
	A = NewChainLink(B.strval, N_LEFT_TO_RIGHT);
}
link(A) ::= LEFT_ARROW LEFT_BRACKET RIGHT_BRACKET DASH. { 
	A = NewChainLink("", N_RIGHT_TO_LEFT); 
}
link(A) ::= LEFT_ARROW LEFT_BRACKET STRING(B) RIGHT_BRACKET DASH. { 
	A = NewChainLink(B.strval, N_RIGHT_TO_LEFT);
}

%type whereClause {WhereNode*}

whereClause(A) ::= . { 
	A = NULL;
}
whereClause(A) ::= WHERE cond(B). {
	//printf("where clause\n");
	A = NewWhereNode(B);
}


%type cond {FilterNode*}
%destructor cond { FreeFilterNode($$); }

cond(A) ::= STRING(B) DOT STRING(C) op(D) STRING(E) DOT STRING(F). { A = NewVaryingPredicateNode(B.strval, C.strval, D, E.strval, F.strval); }
cond(A) ::= STRING(B) DOT STRING(C) op(D) value(E). { A = NewConstantPredicateNode(B.strval, C.strval, D, E); }
cond(A) ::= LEFT_PARENTHESIS cond(B) RIGHT_PARENTHESIS. { A = B; }
cond(A) ::= cond(B) AND cond(C). { A = NewConditionNode(B, AND, C); }
cond(A) ::= cond(B) OR cond(C). { A = NewConditionNode(B, OR, C); }


%type op {int}
op(A) ::= EQ. { A = EQ; }
op(A) ::= GT. { A = GT; }
op(A) ::= LT. { A = LT; }
op(A) ::= LE. { A = LE; }
op(A) ::= GE. { A = GE; }
op(A) ::= NE. { A = NE; }


%type value {SIValue}

// raw value tokens - int / string / float
value(A) ::= INTEGER(B). {  A = SI_LongVal(B.intval); }
value(A) ::= STRING(B). {  A = SI_StringValC(strdup(B.strval)); }
value(A) ::= FLOAT(B). {  A = SI_DoubleVal(B.dval); }
value(A) ::= TRUE. { A = SI_BoolVal(1); }
value(A) ::= FALSE. { A = SI_BoolVal(0); }

%type returnClause {ReturnNode*}

returnClause(A) ::= RETURN returnElements(B). {
	A = NewReturnNode(B, 0);
}
returnClause(A) ::= RETURN DISTINCT returnElements(B). {
	A = NewReturnNode(B, 1);
}


%type returnElements {Vector*}

returnElements(A) ::= returnElements(B) COMMA returnElement(C). {
	Vector_Push(B, C);
	A = B;
}

returnElements(A) ::= returnElement(B). {
	A = NewVector(ReturnElementNode*, 1);
	Vector_Push(A, B);
}

%type returnElement {ReturnElementNode*}

returnElement(A) ::= variable(B). {
	A = NewReturnElementNode(N_PROP, B->alias, B->property, NULL);
	FreeVariable(B);
}

returnElement(A) ::= aggFunc(B). {
	A = B;
}

%type variableList {Vector*}
variableList(A) ::= variableList(B) COMMA variable(C). {
	Vector_Push(B, C);
	A = B;
}

variableList(A) ::= variable(B). {
	A = NewVector(Variable*, 1);
	Vector_Push(A, B);
}

%type variable {Variable*}

variable(A) ::= STRING(B) DOT STRING(C). {
	A = NewVariable(B.strval, C.strval);
}

%type aggFunc {ReturnElementNode*}

aggFunc(A) ::= STRING(B) LEFT_PARENTHESIS variable(C) RIGHT_PARENTHESIS. {
	A = NewReturnElementNode(N_AGG_FUNC, C->alias, C->property, B.strval);
}

%type orderClause {OrderNode*}

orderClause(A) ::= . {
	A = NULL;
}
orderClause(A) ::= ORDER BY variableList(B). {
	A = NewOrderNode(B, ORDER_DIR_ASC);
}
orderClause(A) ::= ORDER BY variableList(B) ASC. {
	A = NewOrderNode(B, ORDER_DIR_ASC);
}
orderClause(A) ::= ORDER BY variableList(B) DESC. {
	A = NewOrderNode(B, ORDER_DIR_DESC);
}

%type limitClause {LimitNode*}

limitClause(A) ::= . {
	A = NULL;
}
limitClause(A) ::= LIMIT INTEGER(B). {
	A = NewLimitNode(B.intval);
}

%code {

	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	YY_BUFFER_STATE yy_scan_string( const char * );
  	YY_BUFFER_STATE yy_scan_bytes( const char *, size_t );
  	extern int yylineno;
  	extern char *yytext;

	QueryExpressionNode *ParseQuery(const char *c, size_t len) {

		yy_scan_bytes(c, len);
  		void* pParser = ParseAlloc(malloc);
  		int t = 0;

  		QueryExpressionNode *ret = NULL;

  		while( (t = yylex()) != 0) {
  			////printf("Token %d\n", t);
    		Parse(pParser, t, tok, &ret);
  		}
  		Parse(pParser, 0, tok, &ret);
  		ParseFree(pParser, free);

  		return ret;
	}
	
}