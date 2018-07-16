/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./order.h"

AST_OrderNode* New_AST_OrderNode(Vector *columns, AST_OrderByDirection direction) {
	AST_OrderNode *orderNode = (AST_OrderNode*)malloc(sizeof(AST_OrderNode));
	orderNode->columns = columns;
	orderNode->direction = direction;
	return orderNode;
}

AST_ColumnNode* New_AST_ColumnNode(const char *alias, const char *property, AST_ColumnNodeType type) {
	AST_ColumnNode *node = malloc(sizeof(AST_ColumnNode));
	node->type = type;
	node->alias = NULL;
	node->property = NULL;

	node->alias = malloc(sizeof(char) * (strlen(alias) + 1));
	strcpy(node->alias, alias);

	if(type == N_VARIABLE) {
		node->property = malloc(sizeof(char) * (strlen(property) + 1));
		strcpy(node->property, property);
	}

	return node;
}

AST_ColumnNode* AST_ColumnNodeFromVariable(const AST_Variable *variable) {
	return New_AST_ColumnNode(variable->alias, variable->property, N_VARIABLE);
}

AST_ColumnNode* AST_ColumnNodeFromAlias(const char *alias) {
	return New_AST_ColumnNode(alias, NULL, N_ALIAS);
}

void Free_AST_ColumnNode(AST_ColumnNode* node) {
	if(node != NULL) {
		if(node->alias != NULL) {
			free(node->alias);
		}
		if(node->type == N_VARIABLE && node->property != NULL) {
			free(node->property);
		}
		free(node);
	}
}

void Free_AST_OrderNode(AST_OrderNode *orderNode) {
	if(orderNode != NULL) {
		for(int i = 0; i < Vector_Size(orderNode->columns); i++) {
			AST_ColumnNode *c = NULL;
			Vector_Get(orderNode->columns, i , &c);
			Free_AST_ColumnNode(c);
		}

		Vector_Free(orderNode->columns);
		free(orderNode);
	}
}
