/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "merge.h"
#include "../ast_common.h"

AST_MergeNode* New_AST_MergeNode(Vector *graphEntities) {
	AST_MergeNode *mergeNode = malloc(sizeof(AST_MergeNode));
	mergeNode->graphEntities = graphEntities;
	return mergeNode;
}

void MergeClause_ReferredEntities(const AST_MergeNode *merge_node, TrieMap *referred_entities) {
    if(!merge_node) return;

    int merge_element_count = Vector_Size(merge_node->graphEntities);

    for(int i = 0; i < merge_element_count; i++) {
        AST_NodeEntity *entity;
        Vector_Get(merge_node->graphEntities, i, &entity);
        TrieMap_Add(referred_entities, entity->alias, strlen(entity->alias), entity, TrieMap_DONT_CARE_REPLACE);
    }
}

void Free_AST_MergeNode(AST_MergeNode *mergeNode) {
	if(!mergeNode) return;

	/* There's no need in freeing entities vector
	 * as it is being used by match clause,
	 * which will free it. */
	free(mergeNode);
}
