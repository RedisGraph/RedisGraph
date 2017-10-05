%left OR.
%left AND.
%nonassoc EQ GT GE LT LE.

%token_type {Token}

%include {
	#include <stdlib.h>
	#include <stdio.h>
	#include <assert.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"
	#include "parse.h"
	#include "../value.h"

	void yyerror(char *s);
} // END %include

%syntax_error {
	char buf[256];
	snprintf(buf, 256, "Syntax error at offset %d near '%s'\n", TOKEN.pos, TOKEN.s);

	ctx->ok = 0;
	ctx->errorMsg = strdup(buf);
}


%extra_argument { parseCtx *ctx }

%type expr {AST_QueryExpressionNode*}

query ::= expr(A). { ctx->root = A; }

expr(A) ::= matchClause(B) whereClause(C) creatClause(D) returnClause(E) orderClause(F) limitClause(G). {
	A = New_AST_QueryExpressionNode(B, C, D, NULL, E, F, G);
}

expr(A) ::= matchClause(B) whereClause(C) creatClause(D). {
	A = New_AST_QueryExpressionNode(B, C, D, NULL, NULL, NULL, NULL);
}

expr(A) ::= matchClause(B) whereClause(C) deleteClause(D). {
	A = New_AST_QueryExpressionNode(B, C, NULL, D, NULL, NULL, NULL);
}

expr(A) ::= creatClause(B). {
	A = New_AST_QueryExpressionNode(NULL, NULL, B, NULL, NULL, NULL, NULL);
}

%type matchClause { AST_MatchNode* }

matchClause(A) ::= MATCH chain(B). {
	A = New_AST_MatchNode(B);
}


%type chain {Vector*}

chain(A) ::= node(B). {
	A = NewVector(AST_GraphEntity*, 1);
	Vector_Push(A, B);
} 

chain(A) ::= chain(B) link(C) node(D). {
	Vector_Push(B, C);
	Vector_Push(B, D);
	A = B;
}


%type creatClause { AST_CreateNode *}

creatClause(A) ::= . { 
	A = NULL;
}

creatClause(A) ::= CREATE createExpression(B). {
	A = New_AST_CreateNode(B);
}

%type createExpression {Vector*}

createExpression(A) ::= chain(B). {
	A = NewVector(Vector*, 1);
	Vector_Push(A, B);
}

createExpression(A) ::= createExpression(B) COMMA chain(C). {
	Vector_Push(B, C);
	A = B;
}


%type deleteClause { AST_DeleteNode *}

deleteClause(A) ::= DELETE deleteExpression(B). {
	A = New_AST_DeleteNode(B);
}

%type deleteExpression {Vector*}

deleteExpression(A) ::= STRING(B). {
	A = NewVector(char*, 1);
	Vector_Push(A, B.strval);
}

deleteExpression(A) ::= deleteExpression(B) COMMA STRING(C). {
	Vector_Push(B, C.strval);
	A = B;
}

%type node {AST_NodeEntity*}

// Node with alias and label
node(A) ::= LEFT_PARENTHESIS STRING(B) COLON STRING(C) properties(D) RIGHT_PARENTHESIS. {
	A = New_AST_NodeEntity(B.strval, C.strval, D);
}

// node with label
node(A) ::= LEFT_PARENTHESIS COLON STRING(B) properties(D) RIGHT_PARENTHESIS. {
	A = New_AST_NodeEntity(NULL, B.strval, D);
}

// node with alias
node(A) ::= LEFT_PARENTHESIS STRING(B) properties(D) RIGHT_PARENTHESIS. {
	A = New_AST_NodeEntity(B.strval, NULL, D);
}

// empty node
node(A) ::= LEFT_PARENTHESIS properties(D) RIGHT_PARENTHESIS. {
	A = New_AST_NodeEntity(NULL, NULL, D);
}

%type link {AST_LinkEntity*}

// left to right edge
link(A) ::= DASH edge(B) RIGHT_ARROW . {
	A = B;
	A->direction = N_LEFT_TO_RIGHT;
}

// right to left edge
link(A) ::= LEFT_ARROW edge(B) DASH . {
	A = B;
	A->direction = N_RIGHT_TO_LEFT;
}

%type edge {AST_LinkEntity*}
// Empty edge []
edge(A) ::= LEFT_BRACKET properties(B) RIGHT_BRACKET . { 
	A = New_AST_LinkEntity(NULL, NULL, B, N_DIR_UNKNOWN);
}

// Edge with an alias [alias]
edge(A) ::= LEFT_BRACKET STRING(B) properties(C) RIGHT_BRACKET . { 
	A = New_AST_LinkEntity(B.strval, NULL, C, N_DIR_UNKNOWN);
}

// Edge with an alias [:label]
edge(A) ::= LEFT_BRACKET COLON STRING(B) properties(C) RIGHT_BRACKET . { 
	A = New_AST_LinkEntity(NULL, B.strval, C, N_DIR_UNKNOWN);
}

// Edge with an alias and label [alias:label]
edge(A) ::= LEFT_BRACKET STRING(B) COLON STRING(C) properties(D) RIGHT_BRACKET . { 
	A = New_AST_LinkEntity(B.strval, C.strval, D, N_DIR_UNKNOWN);
}

%type properties {Vector*}
// empty properties
properties(A) ::= . {
	A = NULL;
}

properties(A) ::= LEFT_CURLY_BRACKET mapLiteral(B) RIGHT_CURLY_BRACKET. {
	A = B;
}

%type mapLiteral {Vector*}
mapLiteral(A) ::= STRING(B) COLON value(C). {
	A = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringValC(strdup(B.strval));
	Vector_Push(A, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = C;
	Vector_Push(A, val);
}

mapLiteral(A) ::= STRING(B) COLON value(C) COMMA mapLiteral(D). {
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_StringValC(strdup(B.strval));
	Vector_Push(D, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = C;
	Vector_Push(D, val);
	
	A = D;
}

%type whereClause {AST_WhereNode*}

whereClause(A) ::= . { 
	A = NULL;
}
whereClause(A) ::= WHERE cond(B). {
	A = New_AST_WhereNode(B);
}


%type cond {AST_FilterNode*}
%destructor cond { Free_AST_FilterNode($$); }

cond(A) ::= STRING(B) DOT STRING(C) op(D) STRING(E) DOT STRING(F). { A = New_AST_VaryingPredicateNode(B.strval, C.strval, D, E.strval, F.strval); }
cond(A) ::= STRING(B) DOT STRING(C) op(D) value(E). { A = New_AST_ConstantPredicateNode(B.strval, C.strval, D, E); }
cond(A) ::= LEFT_PARENTHESIS cond(B) RIGHT_PARENTHESIS. { A = B; }
cond(A) ::= cond(B) AND cond(C). { A = New_AST_ConditionNode(B, AND, C); }
cond(A) ::= cond(B) OR cond(C). { A = New_AST_ConditionNode(B, OR, C); }


%type op {int}
op(A) ::= EQ. { A = EQ; }
op(A) ::= GT. { A = GT; }
op(A) ::= LT. { A = LT; }
op(A) ::= LE. { A = LE; }
op(A) ::= GE. { A = GE; }
op(A) ::= NE. { A = NE; }


%type value {SIValue}

// raw value tokens - int / string / float
value(A) ::= INTEGER(B). {  A = SI_DoubleVal(B.intval); }
value(A) ::= STRING(B). {  A = SI_StringValC(strdup(B.strval)); }
value(A) ::= FLOAT(B). {  A = SI_DoubleVal(B.dval); }
value(A) ::= TRUE. { A = SI_BoolVal(1); }
value(A) ::= FALSE. { A = SI_BoolVal(0); }

%type returnClause {AST_ReturnNode*}

returnClause(A) ::= RETURN returnElements(B). {
	A = New_AST_ReturnNode(B, 0);
}
returnClause(A) ::= RETURN DISTINCT returnElements(B). {
	A = New_AST_ReturnNode(B, 1);
}


%type returnElements {Vector*}

returnElements(A) ::= returnElements(B) COMMA returnElement(C). {
	Vector_Push(B, C);
	A = B;
}

returnElements(A) ::= returnElement(B). {
	A = NewVector(AST_ReturnElementNode*, 1);
	Vector_Push(A, B);
}

%type returnElement {AST_ReturnElementNode*}

returnElement(A) ::= variable(B). {
	A = New_AST_ReturnElementNode(N_PROP, B, NULL, NULL);
}
returnElement(A) ::= variable(B) AS STRING(C). {
	A = New_AST_ReturnElementNode(N_PROP, B, NULL, C.strval);
}
returnElement(A) ::= aggFunc(B). {
	A = B;
}
returnElement(A) ::= STRING(B). {
	A = New_AST_ReturnElementNode(N_NODE, New_AST_Variable(B.strval, NULL), NULL, NULL);
}

%type variable {AST_Variable*}

variable(A) ::= STRING(B) DOT STRING(C). {
	A = New_AST_Variable(B.strval, C.strval);
}

%type aggFunc {AST_ReturnElementNode*}

aggFunc(A) ::= STRING(B) LEFT_PARENTHESIS variable(C) RIGHT_PARENTHESIS. {
	A = New_AST_ReturnElementNode(N_AGG_FUNC, C, B.strval, NULL);
}
aggFunc(A) ::= STRING(B) LEFT_PARENTHESIS variable(C) RIGHT_PARENTHESIS AS STRING(D). {
	A = New_AST_ReturnElementNode(N_AGG_FUNC, C, B.strval, D.strval);
}

%type orderClause {AST_OrderNode*}

orderClause(A) ::= . {
	A = NULL;
}
orderClause(A) ::= ORDER BY columnNameList(B). {
	A = New_AST_OrderNode(B, ORDER_DIR_ASC);
}
orderClause(A) ::= ORDER BY columnNameList(B) ASC. {
	A = New_AST_OrderNode(B, ORDER_DIR_ASC);
}
orderClause(A) ::= ORDER BY columnNameList(B) DESC. {
	A = New_AST_OrderNode(B, ORDER_DIR_DESC);
}

%type columnNameList {Vector*}
columnNameList(A) ::= columnNameList(B) COMMA columnName(C). {
	Vector_Push(B, C);
	A = B;
}
columnNameList(A) ::= columnName(B). {
	A = NewVector(AST_ColumnNode*, 1);
	Vector_Push(A, B);
}

%type columnName {AST_ColumnNode*}
columnName(A) ::= variable(B). {
	A = AST_ColumnNodeFromVariable(B);
	Free_AST_Variable(B);
}
columnName(A) ::= STRING(B). {
	A = AST_ColumnNodeFromAlias(B.strval);
}

%type limitClause {AST_LimitNode*}

limitClause(A) ::= . {
	A = NULL;
}
limitClause(A) ::= LIMIT INTEGER(B). {
	A = New_AST_LimitNode(B.intval);
}

%code {

	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	YY_BUFFER_STATE yy_scan_string( const char * );
  	YY_BUFFER_STATE yy_scan_bytes( const char *, size_t );
  	extern int yylineno;
  	extern char *yytext;
	extern int yycolumn;

	AST_QueryExpressionNode *Query_Parse(const char *q, size_t len, char **err) {
		yycolumn = 1;	// Reset lexer's token tracking position
		yy_scan_bytes(q, len);
  		void* pParser = ParseAlloc(malloc);
  		int t = 0;

		parseCtx ctx = {.root = NULL, .ok = 1, .errorMsg = NULL};

  		while( (t = yylex()) != 0) {
			Parse(pParser, t, tok, &ctx);
		}
		if (ctx.ok) {
			Parse(pParser, 0, tok, &ctx);
  		}
		ParseFree(pParser, free);
		if (err) {
			*err = ctx.errorMsg;
		}
		return ctx.root;
	}
}