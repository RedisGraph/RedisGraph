%token_type {char*}
%default_type {char*}

%type relationship {RelationshipNode*}
%type node {ItemNode*}
%type link {LinkNode*}
%type filter {FilterNode*}

%include {
	#include <stdio.h>

	#include "AST_Node.h"
	#include "grammar.h"
}

%syntax_error {
	printf("Syntax error!\n");
}


main ::= expr(A).

expr ::= MATCH relationship whereClause returnClause.

relationship(A) ::= node(B) link(C) node(D). { printf("relationship\n"); A = CreateRelationshipNode(B, C, D); }

node(A) ::= LEFT_PARENTHESIS STRING(B) COLON STRING(C) RIGHT_PARENTHESIS. { A = CreateItemNode(B, C); }

node(A) ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS. { A = CreateItemNode("", ""); }

link(A) ::= DASH LEFT_BRACKET RIGHT_BRACKET DASH RIGHT_ANGLE_BRACKET. { printf("empty link\n"); A = CreateLinkNode("", ""); }

link(A) ::= DASH LEFT_BRACKET STRING(B) COLON STRING(C) RIGHT_BRACKET DASH RIGHT_ANGLE_BRACKET. { 
	A = CreateLinkNode(B, C); }


whereClause(A) ::= . { printf("no where clause\n"); }
whereClause(A) ::= WHERE filters(B). { printf("where clause\n"); }
filters(A) ::= filter(B) AND filters(C). { printf("two or more filters\n"); }
filters(A) ::= filter(B). { printf("a single filter\n"); }
filter(A) ::= STRING(B) DOT STRING(C) EQUALS STRING(D). { printf("filter %s.%s = %s\n", B, C, D); }

filter(A) ::= STRING(B) DOT STRING(C) EQUALS NUMBER(D). { printf("filter %s.%s = %s\n", B, C, D); }
filter(A) ::= STRING(B) DOT STRING(C) RIGHT_ANGLE_BRACKET NUMBER(D). { printf("filter %s.%s > %s\n", B, C, D); }
filter(A) ::= STRING(B) DOT STRING(C) LEFT_ANGLE_BRACKET NUMBER(D). { printf("filter %s.%s < %s\n", B, C, D); }
filter(A) ::= STRING(B) DOT STRING(C) RIGHT_ANGLE_BRACKET EQUALS NUMBER(D). { printf("filter %s.%s >= %s\n", B, C, D); }
filter(A) ::= STRING(B) DOT STRING(C) LEFT_ANGLE_BRACKET EQUALS NUMBER(D). { printf("filter %s.%s <= %s\n", B, C, D); }


returnClause ::= RETURN variables. { printf("return clause\n"); }
variables(A) ::= variable(B). { printf("a single variable\n"); }
variables(A) ::= variable(B) COMMA variables(C). { printf("multi variables %s, %s\n", B, C); }
variable(A) ::= STRING(B). { printf("variable: %s\n", B); }
variable(A) ::= STRING(B) DOT STRING(C). { printf("variable: %s prop: %s\n", B, C); }