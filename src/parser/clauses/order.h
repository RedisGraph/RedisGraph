/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _CLAUSE_ORDER_H
#define _CLAUSE_ORDER_H

#include "../ast_common.h"
#include "../../util/vector.h"
#include "../ast_arithmetic_expression.h"

typedef enum {
	ORDER_DIR_ASC,
	ORDER_DIR_DESC
} AST_OrderByDirection;

typedef struct {
	AST_ArithmeticExpressionNode
	**expressions;	// Array of arithmetic expressions to order by.
	AST_OrderByDirection direction;
} AST_OrderNode;

AST_OrderNode *New_AST_OrderNode(Vector *expressions,
								 AST_OrderByDirection direction);

void OrderClause_ReferredEntities(const AST_OrderNode *order_node,
								  TrieMap *referred_entities);

void Free_AST_OrderNode(AST_OrderNode *orderNode);

#endif
