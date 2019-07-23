/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "delete.h"

AST_DeleteNode *New_AST_DeleteNode(Vector *elements) {
	AST_DeleteNode *deleteNode = (AST_DeleteNode *)malloc(sizeof(AST_DeleteNode));
	deleteNode->graphEntities = elements;
	return deleteNode;
}

void DeleteClause_ReferredEntities(const AST_DeleteNode *delete_node,
								   TrieMap *referred_entities) {
	if(!delete_node) return;

	int delete_element_count = Vector_Size(delete_node->graphEntities);
	for(int i = 0; i < delete_element_count; i++) {
		char *delete_element;
		Vector_Get(delete_node->graphEntities, i, &delete_element);
		TrieMap_Add(referred_entities, delete_element, strlen(delete_element),
					delete_element, TrieMap_DONT_CARE_REPLACE);
	}
}

void Free_AST_DeleteNode(AST_DeleteNode *deleteNode) {
	if(!deleteNode)	return;
	Vector_Free(deleteNode->graphEntities);
	free(deleteNode);
}
