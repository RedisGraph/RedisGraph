%token_type {char*}
%default_type {char*}
%type NUMBER {int}

%type matchClause {MatchNode*}
%type relationship {RelationshipNode*}
%type node {ItemNode*}
%type link {LinkNode*}

%type whereClause {WhereNode*}
%type filters {Vector*}
%type filter {FilterNode*}

%type returnClause {ReturnNode*}
%type variables {Vector*}
%type variable {VariableNode*}

%type expr {QueryExpressionNode*}

%include {
	#include <stdio.h>

	#include "AST_Node.h"
	#include "grammar.h"
}

%syntax_error {
	printf("Syntax error!\n");
}


expr ::= matchClause(B) whereClause(C) returnClause(D). { printf("Query expression"); CreateQueryExpressionNode(B, C, D); }

matchClause(A) ::= MATCH relationship(B). { printf("match clause"); A = CreateMatchNode(B); }

relationship(A) ::= node(B) link(C) node(D). { printf("relationship\n"); A = CreateRelationshipNode(B, C, D); }

node(A) ::= LEFT_PARENTHESIS STRING(B) COLON STRING(C) RIGHT_PARENTHESIS. { A = CreateItemNode(B, C); }

node(A) ::= LEFT_PARENTHESIS STRING(B) RIGHT_PARENTHESIS. { A = CreateItemNode(B, ""); }

node(A) ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS. { A = CreateItemNode("", ""); }

link(A) ::= DASH LEFT_BRACKET RIGHT_BRACKET DASH RIGHT_ANGLE_BRACKET. { printf("empty link\n"); A = CreateLinkNode("", ""); }

link(A) ::= DASH LEFT_BRACKET STRING(B) COLON STRING(C) RIGHT_BRACKET DASH RIGHT_ANGLE_BRACKET. { 
	A = CreateLinkNode(B, C); }


whereClause(A) ::= . { printf("no where clause\n"); A = NULL; }
whereClause(A) ::= WHERE filters(B). { printf("where clause\n"); A = CreateWhereNode(B); }

filters(A) ::= filter(B) AND filters(C). { printf("two or more filters\n"); Vector_Push(C, B); A = C; }

filters(A) ::= filter(B). { printf("a single filter\n"); A = NewVector(FilterNode*, 1); Vector_Push(A, B); }

filter(A) ::= STRING(B) DOT STRING(C) EQUALS STRING(D). { printf("filter %s.%s = %s\n", B, C, D); A = CreateStringFilterNode(B, C, EQ, D); }

filter(A) ::= STRING(B) DOT STRING(C) EQUALS NUMBER(D). { printf("filter %s.%s = %s\n", B, C, D); A = CreateNumericFilterNode(B, C, EQ, D); }

filter(A) ::= STRING(B) DOT STRING(C) RIGHT_ANGLE_BRACKET NUMBER(D). { printf("filter %s.%s > %s\n", B, C, D); A = CreateNumericFilterNode(B, C, GT, D); }

filter(A) ::= STRING(B) DOT STRING(C) LEFT_ANGLE_BRACKET NUMBER(D). { printf("filter %s.%s < %s\n", B, C, D); A = CreateNumericFilterNode(B, C, LT, D); }

filter(A) ::= STRING(B) DOT STRING(C) RIGHT_ANGLE_BRACKET EQUALS NUMBER(D). { printf("filter %s.%s >= %s\n", B, C, D); A = CreateNumericFilterNode(B, C, GE, D); }

filter(A) ::= STRING(B) DOT STRING(C) LEFT_ANGLE_BRACKET EQUALS NUMBER(D). { printf("filter %s.%s <= %s\n", B, C, D); A = CreateNumericFilterNode(B, C, LE, D); }


returnClause(A) ::= RETURN variables(B). { printf("return clause\n"); A = CreateReturnNode(B); }

variables(A) ::= variable(B). { printf("a single variable\n"); A = NewVector(VariableNode*, 1); Vector_Push(A, B); }

variables(A) ::= variable(B) COMMA variables(C). { printf("multi variables\n"); Vector_Push(C, B); A = C; }

variable(A) ::= STRING(B). { printf("variable: %s\n", B); A = CreateVariableNode(B, ""); }
variable(A) ::= STRING(B) DOT STRING(C). { printf("variable: %s prop: %s\n", B, C); A = CreateVariableNode(B, C); }