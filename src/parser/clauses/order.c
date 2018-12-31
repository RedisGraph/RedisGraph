/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
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
