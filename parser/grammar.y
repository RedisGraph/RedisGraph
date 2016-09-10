%token_type {char*}
%default_type {char*}

%type relationship {RelationshipNode*}
%type node {ItemNode*}
%type link {LinkNode*}
%type filter {FilterNode*}

%include {
	#include <stdio.h>
	#include <iostream>

	#include "AST_Node.h"
	#include "grammar.h"
}

%syntax_error {
	printf("Syntax error!\n");
}

relationship(A) ::= node(B) link(C) node(D). { printf("relationship\n"); A = CreateRelationshipNode(B, C, D); }

node(A) ::= LEFT_PARENTHESIS STRING(B) COLON STRING(C) filter(D) RIGHT_PARENTHESIS. { A = CreateItemNode(B, C); }

node(A) ::= LEFT_PARENTHESIS RIGHT_PARENTHESIS. { A = CreateItemNode("", ""); }

link(A) ::= DASH LEFT_BRACKET RIGHT_BRACKET DASH ARROW_RIGHT. { printf("empty link\n"); A = CreateLinkNode("", ""); }

link(A) ::= DASH LEFT_BRACKET STRING(B) COLON STRING(C) RIGHT_BRACKET DASH ARROW_RIGHT. { 
	A = CreateLinkNode(B, C); }


filter(A) ::= LEFT_CURLY_BRACKET STRING(B) COLON STRING(C) RIGHT_CURLY_BRACKET. { printf("filter number\n"); A = CreateFilterNode(B, C); }

filter(A) ::= LEFT_CURLY_BRACKET STRING(B) COLON NUMBER(C) RIGHT_CURLY_BRACKET. { printf("filter number\n"); }

filter(A) ::= . { printf("empty filters\n"); A = NULL; }