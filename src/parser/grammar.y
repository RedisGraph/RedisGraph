%left OR.
%left AND.
%left ADD DASH.
%left MUL DIV.
%nonassoc EQ GT GE LT LE.

%token_type {Token}

%include {
	#include <stdlib.h>
	#include <stdio.h>
	#include <stdint.h>
	#include <assert.h>
	#include <limits.h>
	#include "token.h"	
	#include "grammar.h"
	#include "ast.h"
	#include "./clauses/clauses.h"
	#include "parse.h"
	#include "../value.h"
	#include "../util/arr.h"

	void yyerror(char *s);

	/*
	**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
	**                       zero the stack is dynamically sized using realloc()
	*/
	// Increase depth from 100 to 1000 to handel deep recursion.
	#define YYSTACKDEPTH 1000
} // END %include

%syntax_error {
	char buf[256];
	snprintf(buf, 256, "Syntax error at offset %d near '%s'", TOKEN.pos, TOKEN.s);

	ctx->ok = 0;
	ctx->errorMsg = strdup(buf);
}

%extra_argument { parseCtx *ctx }

query ::= expressions(A). { ctx->root = A; }

%type expressions {AST**}

expressions(A) ::= expr(B). {
	A = array_new(AST*, 1);
	A = array_append(A, B);
}

expressions(A) ::= expressions(B) withClause(C) singlePartQuery(D). {
	AST *ast = B[array_len(B)-1];
	ast->withNode = C;
	A = array_append(B, D);
	A=B;
}

%type singlePartQuery {AST*}
singlePartQuery(A) ::= expr(B). {
	A = B;
}

singlePartQuery(A) ::= skipClause(B) limitClause(C) returnClause(D) orderClause(E). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, D, E, B, C, NULL, NULL, NULL);
}

singlePartQuery(A) ::= limitClause(B) returnClause(C) orderClause(D). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, C, D, NULL, B, NULL, NULL, NULL);
}

singlePartQuery(A) ::= skipClause(B) returnClause(C) orderClause(D). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, C, D, B, NULL, NULL, NULL, NULL);
}

singlePartQuery(A) ::= returnClause(B) orderClause(C) skipClause(D) limitClause(E). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, B, C, D, E, NULL, NULL, NULL);
}

singlePartQuery(A) ::= orderClause(B) skipClause(C) limitClause(D) returnClause(E). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, E, B, C, D, NULL, NULL, NULL);
}

singlePartQuery(A) ::= orderClause(B) skipClause(C) limitClause(D) setClause(E). {
	A = AST_New(NULL, NULL, NULL, NULL, E, NULL, NULL, B, C, D, NULL, NULL, NULL);
}

singlePartQuery(A) ::= orderClause(B) skipClause(C) limitClause(D) deleteClause(E). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, E, NULL, B, C, D, NULL, NULL, NULL);
}

%type expr {AST*}
expr(A) ::= multipleMatchClause(B) whereClause(C) multipleCreateClause(D) returnClause(E) orderClause(F) skipClause(G) limitClause(H). {
	A = AST_New(B, C, D, NULL, NULL, NULL, E, F, G, H, NULL, NULL, NULL);
}

expr(A) ::= multipleMatchClause(B) whereClause(C) multipleCreateClause(D). {
	A = AST_New(B, C, D, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

expr(A) ::= multipleMatchClause(B) whereClause(C) deleteClause(D). {
	A = AST_New(B, C, NULL, NULL, NULL, D, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

expr(A) ::= multipleMatchClause(B) whereClause(C) setClause(D). {
	A = AST_New(B, C, NULL, NULL, D, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

expr(A) ::= multipleMatchClause(B) whereClause(C) setClause(D) returnClause(E) orderClause(F) skipClause(G) limitClause(H). {
	A = AST_New(B, C, NULL, NULL, D, NULL, E, F, G, H, NULL, NULL, NULL);
}

expr(A) ::= multipleCreateClause(B). {
	A = AST_New(NULL, NULL, B, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

expr(A) ::= unwindClause(B) multipleCreateClause(C). {
	A = AST_New(NULL, NULL, C, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, B, NULL);
}

expr(A) ::= indexClause(B). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, B, NULL, NULL);
}

expr(A) ::= mergeClause(B). {
	A = AST_New(NULL, NULL, NULL, B, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

expr(A) ::= mergeClause(B) setClause(C). {
	A = AST_New(NULL, NULL, NULL, B, C, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

expr(A) ::= returnClause(B). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, B, NULL, NULL, NULL, NULL, NULL, NULL);
}

expr(A) ::= unwindClause(B) returnClause(C) skipClause(D) limitClause(E). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, C, NULL, D, E, NULL, B, NULL);
}

// Procedure call

expr(A) ::= procedureCallClause(B). {
	A = AST_New(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, B);
}

expr(A) ::= procedureCallClause(B) whereClause(C) returnClause(D) orderClause(E) skipClause(F) limitClause(G). {
	A = AST_New(NULL, C, NULL, NULL, NULL, NULL, D, E, F, G, NULL, NULL, B);
}

expr(A) ::= procedureCallClause(B) multipleMatchClause(C) whereClause(D) returnClause(E) orderClause(F) skipClause(G) limitClause(H). {
	A = AST_New(C, D, NULL, NULL, NULL, NULL, E, F, G, H, NULL, NULL, B);
}

%type procedureCallClause { AST_ProcedureCallNode* }
procedureCallClause(A) ::= CALL procedureName(B) LEFT_PARENTHESIS stringList(C) RIGHT_PARENTHESIS YIELD unquotedStringList(D). {
	A = New_AST_ProcedureCallNode(B, C, D);
}

procedureCallClause(A) ::= CALL procedureName(B) LEFT_PARENTHESIS stringList(C) RIGHT_PARENTHESIS. {	
	A = New_AST_ProcedureCallNode(B, C, NULL);
}

%type procedureName { char* }
procedureName(A) ::= unquotedStringList(B). {
	// Concatenate strings with dots.
	// Determine required string length.
	int buffLen = 0;
	for(int i = 0; i < array_len(B); i++) {
		buffLen += strlen(B[i]) + 1;
	}

	int offset = 0;
	char *procedure_name = malloc(buffLen);
	for(int i = 0; i < array_len(B); i++) {
		int n = strlen(B[i]);
		memcpy(procedure_name + offset, B[i], n);
		offset += n;
		procedure_name[offset] = '.';
		offset++;
	}

	// Discard last dot and trerminate string.
	offset--;
	procedure_name[offset] = '\0';
	A = procedure_name;
}

%type stringList {char**}
stringList(A) ::= STRING(B) . {
	A = array_new(char*, 1);
	A = array_append(A, B.strval);
}

// Multiple strings.
stringList(A) ::= stringList(B) delimiter STRING(C) . {
	B = array_append(B, C.strval);
	A = B;
}

%type unquotedStringList {char**}
unquotedStringList(A) ::= UQSTRING(B) . {
	A = array_new(char*, 1);
	A = array_append(A, B.strval);
}

// Multiple unquoted strings.
unquotedStringList(A) ::= unquotedStringList(B) delimiter UQSTRING(C) . {
	B = array_append(B, C.strval);
	A = B;
}

%type delimiter {int}
delimiter(A) ::= COMMA. { A = COMMA; }
delimiter(A) ::= DOT. { A = DOT; }

%type multipleMatchClause { AST_MatchNode* }
multipleMatchClause(A) ::= matchClauses(B). {
	A = New_AST_MatchNode(B);
}

// Vector of vectors.
%type matchClauses { Vector* }
matchClauses(A) ::= matchClause(B). {
	A = B;
}

matchClauses(A) ::= matchClauses(B) matchClause(C). {
	Vector *v;
	while(Vector_Pop(C, &v)) Vector_Push(B, v);
	Vector_Free(C);
	A = B;
}

// Vector of vectors.
%type matchClause { Vector* }
matchClause(A) ::= MATCH chains(B). {
	A = B;
}

%type multipleCreateClause { AST_CreateNode* }
multipleCreateClause(A) ::= . {
	A = NULL;
}

multipleCreateClause(A) ::= createClauses(B). {
	A = New_AST_CreateNode(B);
}

// Vector of Vectors, each representing a single chain.
%type createClauses {Vector*}

createClauses(A) ::= createClause(B). {
	A = B;
}
 createClauses(A) ::= createClauses(B) createClause(C). {
	Vector *v;
	while(Vector_Pop(C, &v)) Vector_Push(B, v);
	Vector_Free(C);
	A = B;
}

// Vector of Vectors, each representing a single chain.
%type createClause {Vector*}

createClause(A) ::= CREATE chains(B). {
	A = B;
}

%type indexClause { AST_IndexNode* }

indexClause(A) ::= indexOpToken(B) INDEX ON indexLabel(C) indexProp(D) . {
  A = New_AST_IndexNode(C.strval, D.strval, B);
}

%type indexOpToken { AST_IndexOpType }

indexOpToken(A) ::= CREATE . { A = CREATE_INDEX; }
indexOpToken(A) ::= DROP . { A = DROP_INDEX; }

indexLabel(A) ::= COLON UQSTRING(B) . {
  A = B;
}

indexProp(A) ::= LEFT_PARENTHESIS UQSTRING(B) RIGHT_PARENTHESIS . {
  A = B;
}

%type mergeClause { AST_MergeNode* }

mergeClause(A) ::= MERGE chain(B). {
	A = New_AST_MergeNode(B);
}

%type setClause { AST_SetNode* }
setClause(A) ::= SET setList(B). {
	A = New_AST_SetNode(B);
}

%type setList {Vector*}
setList(A) ::= setElement(B). {
	A = NewVector(AST_SetElement*, 1);
	Vector_Push(A, B);
}
setList(A) ::= setList(B) COMMA setElement(C). {
	Vector_Push(B, C);
	A = B;
}

%type setElement {AST_SetElement*}
setElement(A) ::= variable(B) EQ arithmetic_expression(C). {
	A = New_AST_SetElement(B, C);
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

// Vector of Vectors, each representing a single chain.
%type chains {Vector*}
chains(A) ::= chain(B). {
	A = NewVector(Vector*, 1);
	Vector_Push(A, B);
}

chains(A) ::= chains(B) COMMA chain(C). {
	Vector_Push(B, C);
	A = B;
}


%type deleteClause { AST_DeleteNode *}

deleteClause(A) ::= DELETE deleteExpression(B). {
	A = New_AST_DeleteNode(B);
}

%type deleteExpression {Vector*}

deleteExpression(A) ::= UQSTRING(B). {
	A = NewVector(char*, 1);
	Vector_Push(A, B.strval);
}

deleteExpression(A) ::= deleteExpression(B) COMMA UQSTRING(C). {
	Vector_Push(B, C.strval);
	A = B;
}

%type node {AST_NodeEntity*}

// Node with alias and label
node(A) ::= LEFT_PARENTHESIS UQSTRING(B) COLON UQSTRING(C) properties(D) RIGHT_PARENTHESIS. {
	A = New_AST_NodeEntity(B.strval, C.strval, D);
}

// node with label
node(A) ::= LEFT_PARENTHESIS COLON UQSTRING(B) properties(D) RIGHT_PARENTHESIS. {
	A = New_AST_NodeEntity(NULL, B.strval, D);
}

// node with alias
node(A) ::= LEFT_PARENTHESIS UQSTRING(B) properties(D) RIGHT_PARENTHESIS. {
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
edge(A) ::= LEFT_BRACKET properties(B) edgeLength(C) RIGHT_BRACKET . { 
	A = New_AST_LinkEntity(NULL, NULL, B, N_DIR_UNKNOWN, C);
}

// Edge with alias [alias]
edge(A) ::= LEFT_BRACKET UQSTRING(B) properties(C) RIGHT_BRACKET . { 
	A = New_AST_LinkEntity(B.strval, NULL, C, N_DIR_UNKNOWN, NULL);
}

// Edge with label [:label]
edge(A) ::= LEFT_BRACKET edgeLabels(B) edgeLength(C) properties(D) RIGHT_BRACKET . { 
	A = New_AST_LinkEntity(NULL, B, D, N_DIR_UNKNOWN, C);
}

// Edge with alias and label [alias:label]
edge(A) ::= LEFT_BRACKET UQSTRING(B) edgeLabels(C) properties(D) RIGHT_BRACKET . { 
	A = New_AST_LinkEntity(B.strval, C, D, N_DIR_UNKNOWN, NULL);
}


%type edgeLabel {char*}
// Single label
edgeLabel(A) ::= COLON UQSTRING(B) . {
	A = B.strval;
}

%type edgeLabels {char**}
edgeLabels(A) ::= edgeLabel(B) . {
	A = array_new(char*, 1);
	A = array_append(A, B);
}

// Multiple labels
edgeLabels(A) ::= edgeLabels(B) PIPE edgeLabel(C) . {
	char *label = C;
	B = array_append(B, label);
	A = B;
}

%type edgeLength {AST_LinkLength*}

// Edge length not specified.
edgeLength(A) ::= . {
	A = NULL;
}

// *minHops..maxHops
edgeLength(A) ::= MUL INTEGER(B) DOTDOT INTEGER(C). {
	A = New_AST_LinkLength(B.longval, C.longval);
}

// *minHops..
edgeLength(A) ::= MUL INTEGER(B) DOTDOT. {
	A = New_AST_LinkLength(B.longval, UINT_MAX-2);
}

// *..maxHops
edgeLength(A) ::= MUL DOTDOT INTEGER(B). {
	A = New_AST_LinkLength(1, B.longval);
}

// *hops
edgeLength(A) ::= MUL INTEGER(B). {
	A = New_AST_LinkLength(B.longval, B.longval);
}

// *
edgeLength(A) ::= MUL. {
	A = New_AST_LinkLength(1, UINT_MAX-2);
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
// key:value
mapLiteral(A) ::= UQSTRING(B) COLON value(C). {
	A = NewVector(SIValue*, 2);

	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(B.strval);
	Vector_Push(A, key);

	SIValue *val = malloc(sizeof(SIValue));
	*val = C;
	Vector_Push(A, val);
}

mapLiteral(A) ::= UQSTRING(B) COLON value(C) COMMA mapLiteral(D). {
	SIValue *key = malloc(sizeof(SIValue));
	*key = SI_ConstStringVal(B.strval);
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

// arithmetic expressions can be constant values, variables, or functions
cond(A) ::= arithmetic_expression(B) relation(C) arithmetic_expression(D). { A = New_AST_PredicateNode(B, C, D); }

cond(A) ::= LEFT_PARENTHESIS cond(B) RIGHT_PARENTHESIS. { A = B; }
cond(A) ::= cond(B) AND cond(C). { A = New_AST_ConditionNode(B, AND, C); }
cond(A) ::= cond(B) OR cond(C). { A = New_AST_ConditionNode(B, OR, C); }

%type returnClause {AST_ReturnNode*}

returnClause(A) ::= RETURN returnElements(B). {
	A = New_AST_ReturnNode(B, 0);
}
returnClause(A) ::= RETURN DISTINCT returnElements(B). {
	A = New_AST_ReturnNode(B, 1);
}
// RETURN *
returnClause(A) ::= RETURN MUL. {
	A = New_AST_ReturnNode(NULL, 0);
}
returnClause(A) ::= RETURN DISTINCT MUL. {
	A = New_AST_ReturnNode(NULL, 1);
}

%type returnElements {AST_ReturnElementNode**}

returnElements(A) ::= returnElements(B) COMMA returnElement(C). {
	A = array_append(B, C);
}

returnElements(A) ::= returnElement(B). {
	A = array_new(AST_ReturnElementNode*, 1);
	array_append(A, B);
}

%type returnElement {AST_ReturnElementNode*}

returnElement(A) ::= arithmetic_expression(B). {
	A = New_AST_ReturnElementNode(B, NULL);
}
// me.age AS my_age
returnElement(A) ::= arithmetic_expression(B) AS UQSTRING(C). {
	A = New_AST_ReturnElementNode(B, C.strval);
}

%type withClause {AST_WithNode*}
withClause(A) ::= WITH withElements(B). {
	A = New_AST_WithNode(B);
}

%type withElements {AST_WithElementNode**}
withElements(A) ::= withElement(B). {
	A = array_new(AST_WithElementNode*, 1);
	array_append(A, B);
}
withElements(A) ::= withElements(B) COMMA withElement(C). {
	A = array_append(B, C);
}

%type withElement {AST_WithElementNode*}
withElement(A) ::= arithmetic_expression(B) AS UQSTRING(C). {
	A = New_AST_WithElementNode(B, C.strval);
}

%type arithmetic_expression {AST_ArithmeticExpressionNode*}

// Highest precedence, (1+2)
arithmetic_expression(A) ::= LEFT_PARENTHESIS arithmetic_expression(B) RIGHT_PARENTHESIS. {
	A = B;
}

// Binary op (+,-,/,*)

arithmetic_expression(A) ::= arithmetic_expression(B) ADD arithmetic_expression(D). {
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, B);
	Vector_Push(args, D);
	A = New_AST_AR_EXP_OpNode("ADD", args);
}

arithmetic_expression(A) ::= arithmetic_expression(B) DASH arithmetic_expression(D). {
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, B);
	Vector_Push(args, D);
	A = New_AST_AR_EXP_OpNode("SUB", args);
}

arithmetic_expression(A) ::= arithmetic_expression(B) MUL arithmetic_expression(D). {
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, B);
	Vector_Push(args, D);
	A = New_AST_AR_EXP_OpNode("MUL", args);
}

arithmetic_expression(A) ::= arithmetic_expression(B) DIV arithmetic_expression(D). {
	Vector *args = NewVector(AST_ArithmeticExpressionNode*, 2);
	Vector_Push(args, B);
	Vector_Push(args, D);
	A = New_AST_AR_EXP_OpNode("DIV", args);
}

// concat(replace(a.first_name), ':', tostring(1 + 2))
arithmetic_expression(A) ::= UQSTRING(B) LEFT_PARENTHESIS arithmetic_expression_list(C) RIGHT_PARENTHESIS. {
	A = New_AST_AR_EXP_OpNode(B.strval, C);
}

// 4, "hello"
arithmetic_expression(A) ::= value(B). {
	A = New_AST_AR_EXP_ConstOperandNode(B);
}

// a.name
arithmetic_expression(A) ::= variable(B). {
	A = New_AST_AR_EXP_VariableOperandNode(B->alias, B->property);
	free(B->alias);
	free(B->property);
	free(B);
}

// Vector of AST_ArithmeticExpressionNode pointers.
%type arithmetic_expression_list {Vector*}
arithmetic_expression_list(A) ::= . {
	A = NewVector(AST_ArithmeticExpressionNode*, 0);
}
arithmetic_expression_list(A) ::= arithmetic_expression_list(B) COMMA arithmetic_expression(C). {
	Vector_Push(B, C);
	A = B;
}
arithmetic_expression_list(A) ::= arithmetic_expression(B). {
	A = NewVector(AST_ArithmeticExpressionNode*, 1);
	Vector_Push(A, B);
}

%type variable {AST_Variable*}
// me (collapsed node).
variable(A) ::= UQSTRING(B). {
	A = New_AST_Variable(B.strval, NULL);
}
// me.age
variable(A) ::= UQSTRING(B) DOT UQSTRING(C). {
	A = New_AST_Variable(B.strval, C.strval);
}

%type orderClause {AST_OrderNode*}

orderClause(A) ::= . {
	A = NULL;
}
orderClause(A) ::= ORDER BY arithmetic_expression_list(B). {
	A = New_AST_OrderNode(B, ORDER_DIR_ASC);
}
orderClause(A) ::= ORDER BY arithmetic_expression_list(B) ASC. {
	A = New_AST_OrderNode(B, ORDER_DIR_ASC);
}
orderClause(A) ::= ORDER BY arithmetic_expression_list(B) DESC. {
	A = New_AST_OrderNode(B, ORDER_DIR_DESC);
}

%type skipClause {AST_SkipNode*}

skipClause(A) ::= . {
	A = NULL;
}
skipClause(A) ::= SKIP INTEGER(B). {
	A = New_AST_SkipNode(B.longval);
}

%type limitClause {AST_LimitNode*}

limitClause(A) ::= . {
	A = NULL;
}
limitClause(A) ::= LIMIT INTEGER(B). {
	A = New_AST_LimitNode(B.longval);
}

%type unwindClause {AST_UnwindNode*}

unwindClause(A) ::= UNWIND LEFT_BRACKET arithmetic_expression_list(B) RIGHT_BRACKET AS UQSTRING(C). {
	A = New_AST_UnwindNode(B, C.strval);
}

%type relation {int}
relation(A) ::= EQ. { A = EQ; }
relation(A) ::= GT. { A = GT; }
relation(A) ::= LT. { A = LT; }
relation(A) ::= LE. { A = LE; }
relation(A) ::= GE. { A = GE; }
relation(A) ::= NE. { A = NE; }

%type value {SIValue}

// raw value tokens - int / string / float
value(A) ::= INTEGER(B). {  A = SI_LongVal(B.longval); }
value(A) ::= DASH INTEGER(B). {  A = SI_LongVal(-B.longval); }
value(A) ::= STRING(B). {  A = SI_ConstStringVal(B.strval); } // TODO this is probably a leak
value(A) ::= FLOAT(B). {  A = SI_DoubleVal(B.dval); }
value(A) ::= DASH FLOAT(B). {  A = SI_DoubleVal(-B.dval); }
value(A) ::= TRUE. { A = SI_BoolVal(1); }
value(A) ::= FALSE. { A = SI_BoolVal(0); }
value(A) ::= NULLVAL. { A = SI_NullVal(); }

%code {

	/* Definitions of flex stuff */
	typedef struct yy_buffer_state *YY_BUFFER_STATE;
	int             yylex( void );
	extern int             yylex_destroy (void);
	YY_BUFFER_STATE yy_scan_string( const char * );
  	YY_BUFFER_STATE yy_scan_bytes( const char *, size_t );
  	extern int yylineno;
  	extern char *yytext;
	extern int yycolumn;

	AST **Query_Parse(const char *q, size_t len, char **err) {
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
		yylex_destroy();
		return ctx.root;
	}
}
