/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "delete.h"

AST_DeleteNode* New_AST_DeleteNode(Vector *elements) {
	AST_DeleteNode *deleteNode = (AST_DeleteNode*)malloc(sizeof(AST_DeleteNode));
	deleteNode->graphEntities = elements;
	return deleteNode;
}

void DeleteClause_ReferredNodes(const AST_DeleteNode *delete_node, TrieMap *referred_nodes) {
	if(!delete_node) return;

	int delete_element_count = Vector_Size(delete_node->graphEntities);
    for(int i = 0; i < delete_element_count; i++) {
        char *delete_element;
        Vector_Get(delete_node->graphEntities, i, &delete_element);
		TrieMap_Add(referred_nodes, delete_element, strlen(delete_element), NULL, NULL);
    }
}

void Free_AST_DeleteNode(AST_DeleteNode *deleteNode) {
	if(!deleteNode)	return;
	Vector_Free(deleteNode->graphEntities);
	free(deleteNode);
}
