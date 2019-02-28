/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _WITH_H_
#define _WITH_H_

#include "../ast_arithmetic_expression.h"

typedef struct {
	char *alias;
	AST_ArithmeticExpressionNode *exp;
} AST_WithElementNode;

typedef struct {    
    AST_WithElementNode **exps;
} AST_WithNode;

AST_WithNode* New_AST_WithNode(AST_WithElementNode **exps);

AST_WithElementNode* New_AST_WithElementNode(AST_ArithmeticExpressionNode *exp, char* alias);

void WithClause_ReferredEntities(const AST_WithNode *withNode, TrieMap *referred_nodes);

void WithClause_DefinedEntities(const AST_WithNode *withNode, TrieMap *definedEntities);

char** WithClause_GetAliases(const AST_WithNode *withNode);

int WithClause_ContainsAggregation(const AST_WithNode *withNode);

void Free_AST_WithNode(AST_WithNode *withNode);

#endif
