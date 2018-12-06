/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./order.h"
#include "../../util/arr.h"

AST_OrderNode* New_AST_OrderNode(Vector *fields, AST_OrderByDirection direction) {
	AST_OrderNode *orderNode = (AST_OrderNode*)malloc(sizeof(AST_OrderNode));
	size_t fieldsCount = Vector_Size(fields);
	
	// TODO: Remove this logic once arithmetic_expression migrates from vector to array.
	orderNode->fields = array_new(AST_ArithmeticExpressionNode*, fieldsCount);
	for(int i = 0; i < fieldsCount; i++) {
		AST_ArithmeticExpressionNode *exp;
		Vector_Get(fields, i, &exp);
		orderNode->fields = array_append(orderNode->fields, exp);
	}
	Vector_Free(fields);

	orderNode->direction = direction;
	return orderNode;
}

void Free_AST_OrderNode(AST_OrderNode *orderNode) {
	if(orderNode != NULL) {
		for(int i = 0; i < array_len(orderNode->fields); i++) {
			Free_AST_ArithmeticExpressionNode(orderNode->fields[i]);
		}

		array_free(orderNode->fields);
		free(orderNode);
	}
}
