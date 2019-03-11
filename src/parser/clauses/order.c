/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./order.h"
#include "../../util/arr.h"

AST_OrderNode* New_AST_OrderNode(Vector *expressions, AST_OrderByDirection direction) {
	AST_OrderNode *orderNode = (AST_OrderNode*)malloc(sizeof(AST_OrderNode));
	size_t expCount = Vector_Size(expressions);
	
	// TODO: Remove this logic once arithmetic_expression migrates from vector to array.
	orderNode->expressions = array_new(AST_ArithmeticExpressionNode*, expCount);
	for(int i = 0; i < expCount; i++) {
		AST_ArithmeticExpressionNode *exp;
		Vector_Get(expressions, i, &exp);
		orderNode->expressions = array_append(orderNode->expressions, exp);
	}
	Vector_Free(expressions);

	orderNode->direction = direction;
	return orderNode;
}

void OrderClause_ReferredEntities(const AST_OrderNode *order_node, TrieMap *referred_entities) {
    if(!order_node) return;

    int order_element_count = array_len(order_node->expressions);
    for(int i = 0; i < order_element_count; i++) {
        AST_ArithmeticExpressionNode *order_element = order_node->expressions[i];
        AST_AR_EXP_GetAliases(order_element, referred_entities);
    }
}

void Free_AST_OrderNode(AST_OrderNode *orderNode) {
	if(orderNode != NULL) {
		size_t expCount = array_len(orderNode->expressions);
		for(int i = 0; i < expCount; i++) {
			Free_AST_ArithmeticExpressionNode(orderNode->expressions[i]);
		}

		array_free(orderNode->expressions);
		free(orderNode);
	}
}
