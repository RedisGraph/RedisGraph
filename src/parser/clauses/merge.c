/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "merge.h"
#include "../ast_common.h"

AST_MergeNode *New_AST_MergeNode(Vector *graphEntities) {
	AST_MergeNode *mergeNode = malloc(sizeof(AST_MergeNode));
	mergeNode->graphEntities = graphEntities;
	return mergeNode;
}

void MergeClause_NameAnonymousNodes(const AST_MergeNode *mergeNode,
									int *entityID) {
	if(!mergeNode) return;

	int entities_count = Vector_Size(mergeNode->graphEntities);

	for(int i = 0; i < entities_count; i++) {
		AST_GraphEntity *entity;
		Vector_Get(mergeNode->graphEntities, i, &entity);
		if(entity->alias == NULL) {
			entity->anonymous = true;
			// TODO: Memory leak!
			asprintf(&entity->alias, "anon_%d", *entityID);
			(*entityID)++;
		}
	}
}

void MergeClause_DefinedEntities(const AST_MergeNode *mergeNode,
								 TrieMap *defined_entities) {
	if(!mergeNode) return;

	int merge_element_count = Vector_Size(mergeNode->graphEntities);

	for(int i = 0; i < merge_element_count; i++) {
		AST_GraphEntity *entity;
		Vector_Get(mergeNode->graphEntities, i, &entity);
		if(!entity->alias) continue;
		TrieMap_Add(defined_entities, entity->alias, strlen(entity->alias), entity,
					TrieMap_DONT_CARE_REPLACE);
	}
}

void Free_AST_MergeNode(AST_MergeNode *mergeNode) {
	if(!mergeNode) return;

	/* There's no need in freeing entities vector
	 * as it is being used by match clause,
	 * which will free it. */
	free(mergeNode);
}
