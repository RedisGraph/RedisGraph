/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _CLAUSE_RETURN_H
#define _CLAUSE_RETURN_H

#include "../ast_arithmetic_expression.h"
#include "../../util/vector.h"

typedef struct {
	char *alias; 		// Alias given to this return element (using the AS keyword)
	AST_ArithmeticExpressionNode *exp;
	int asterisks;		// Identifies if return element is '*'.
} AST_ReturnElementNode;

typedef struct {
	Vector *returnElements; // Vector of ReturnElementNode pointers
	int distinct;
} AST_ReturnNode;

AST_ReturnElementNode* New_AST_ReturnElementNode(AST_ArithmeticExpressionNode *exp, const char* alias);

// Dedicated return element which represents RETURN '*'.
AST_ReturnElementNode* New_AST_ReturnElementExpandALL();

AST_ReturnNode* New_AST_ReturnNode(Vector* returnElements, int distinct);
/* Checks to see if return clause contains a collapsed node. */
int ReturnClause_ContainsCollapsedNodes(const AST_ReturnNode *return_node);
int ReturnClause_ContainsAggregation(const AST_ReturnNode *return_node);
void ReturnClause_ReferredEntities(const AST_ReturnNode *return_node, TrieMap *referred_nodes);
void ReturnClause_ReferredFunctions(const AST_ReturnNode *return_node, TrieMap *referred_funcs);
void Free_AST_ReturnElementNode(AST_ReturnElementNode *returnElementNode);
void Free_AST_ReturnNode(AST_ReturnNode *returnNode);

#endif
