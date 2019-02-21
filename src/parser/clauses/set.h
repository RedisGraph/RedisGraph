/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _CLAUSE_SET_H
#define _CLAUSE_SET_H

#include "../ast_common.h"
#include "../../util/vector.h"
#include "../../util/triemap/triemap.h"
#include "../ast_arithmetic_expression.h"

typedef struct {
	AST_Variable *entity;				/* Destination entity to update. */
	AST_ArithmeticExpressionNode *exp;	/* Arithmetic expression, evaluated value used for update. */
} AST_SetElement;

typedef struct {
	Vector *set_elements; /* Vector of AST_SetElement pointers, each describes an entity update. */
} AST_SetNode;

/* Set clause individual elements. */
AST_SetNode* New_AST_SetNode(Vector *elements);
AST_SetElement* New_AST_SetElement(AST_Variable *updated_entity, AST_ArithmeticExpressionNode *exp);
void SetClause_ReferredEntities(const AST_SetNode *set_node, TrieMap *referred_entities);
void Free_AST_SetNode(AST_SetNode *setNode);

#endif