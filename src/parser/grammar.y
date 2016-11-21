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

expr(A) ::= matchClause(B) whereClause(C) returnClause(D). { 
	//printf("Query expression\n");
	A = NewQueryExpressionNode(B, C, D); 
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
node(A) ::= LEFT_PARENTHESIS STRING(B) RIGHT_PARENTHESIS. {
	A = NewChainEntity(B.strval, "");
}
node(A) ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS. {
	A = NewChainEntity("", "");
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
	//printf("no where clause\n");
	A = NULL;
}
whereClause(A) ::= WHERE cond(B). {
	//printf("where clause\n");
	A = NewWhereNode(B);
}


%type cond {FilterNode*}
%destructor cond { FreeFilterNode($$); }

cond(A) ::= STRING(B) DOT STRING(C) op(D) value(E). { A = NewPredicateNode(B.strval, C.strval, D, E); }
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

returnClause(A) ::= RETURN variables(B). {
	//printf("return clause\n");
	A = NewReturnNode(B);
}


%type variables {Vector*}

variables(A) ::= variable(B). {
	A = NewVector(VariableNode*, 1);
	Vector_Push(A, B); 
}
variables(A) ::= variables(B) COMMA variable(C). {
	//printf("multi variables\n");
	Vector_Push(B, C);
	A = B;
}


%type variable {VariableNode*}

variable(A) ::= STRING(B). {
	//printf("variable: %s\n", B.strval);
	A = NewVariableNode(B.strval, NULL);
}
variable(A) ::= STRING(B) DOT STRING(C). {
	//printf("variable: %s prop: %s\n", B.strval, C.strval);
	A = NewVariableNode(B.strval, C.strval);
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