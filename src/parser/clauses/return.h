/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "parser/ast_arithmetic_expression.h"
#include "util/vector.h"

typedef struct {
	char *alias; 		// Alias given to this return element (using the AS keyword)
	AST_ArithmeticExpressionNode *exp;
	int asterisks;		// Identifies if return element is '*'
} AST_ReturnElementNode;

typedef struct {
	AST_ReturnElementNode **returnElements; // Array of ReturnElementNode pointers
	int distinct;
} AST_ReturnNode;

AST_ReturnElementNode* New_AST_ReturnElementNode(AST_ArithmeticExpressionNode *exp, const char* alias);

// Dedicated return element which represents RETURN '*'
AST_ReturnElementNode* New_AST_ReturnElementExpandALL();

AST_ReturnNode* New_AST_ReturnNode(AST_ReturnElementNode **returnElements, int distinct);

// Checks to see if return clause contains a collapsed node
int ReturnClause_ContainsCollapsedNodes(const AST_ReturnNode *return_node);

int ReturnClause_ContainsAggregation(const AST_ReturnNode *return_node);

void ReturnClause_ReferredEntities(const AST_ReturnNode *return_node, TrieMap *referred_nodes);

void ReturnClause_ReferredFunctions(const AST_ReturnNode *return_node, TrieMap *referred_funcs);

// Return clause can (kinda) introduce "entities", consider: RETURN n.v AS Z ORDER BY Z*Z
// Z hold an expression value which can be referred to at a later stage, similar to
// MATCH (N), in which N can be referred by other clauses.
void ReturnClause_DefinedEntities(const AST_ReturnNode *return_node, TrieMap *definedEntities);

void Free_AST_ReturnElementNode(AST_ReturnElementNode *returnElementNode);

void Free_AST_ReturnNode(AST_ReturnNode *returnNode);
