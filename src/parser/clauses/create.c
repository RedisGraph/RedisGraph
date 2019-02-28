/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./create.h"
#include "../ast_common.h"

AST_CreateNode* New_AST_CreateNode(Vector *patterns) {
	AST_CreateNode *createNode = (AST_CreateNode*)malloc(sizeof(AST_CreateNode));
	createNode->graphEntities = NewVector(AST_GraphEntity*, 1);
	
	// Merge patterns into a single pattern.
	Vector *pattern;
	while(Vector_Pop(patterns, &pattern)) {
		for(int i = 0; i < Vector_Size(pattern); i++) {
			AST_GraphEntity* entity;
			Vector_Get(pattern, i, &entity);
			Vector_Push(createNode->graphEntities, entity);
		}
		Vector_Free(pattern);
	}

	Vector_Free(patterns);
	return createNode;
}

void CreateClause_ReferredEntities(const AST_CreateNode *createNode, TrieMap *referredNodes) {
	if(!createNode) return;

	int entities_count = Vector_Size(createNode->graphEntities);

    for(int i = 0; i < entities_count; i++) {
        AST_GraphEntity *entity;
        Vector_Get(createNode->graphEntities, i, &entity);
		if (!entity->alias) continue;
		TrieMap_Add(referredNodes, entity->alias, strlen(entity->alias), entity, TrieMap_DONT_CARE_REPLACE);
    }
}

/* We could do a better job here and isolate create clause defined 
 * identifiers from the once declared at earlier clauses, but for 
 * the timebeing setting the defined entities to referred entities 
 * is good enough. */
void CreateClause_DefinedEntities(const AST_CreateNode *createNode, TrieMap *identifiers) {
	return CreateClause_ReferredEntities(createNode, identifiers);
}

void CreateClause_NameAnonymousNodes(AST_CreateNode *createNode, int *entityID) {
	if(!createNode) return;

	int entities_count = Vector_Size(createNode->graphEntities);

    for(int i = 0; i < entities_count; i++) {
        AST_GraphEntity *entity;
        Vector_Get(createNode->graphEntities, i, &entity);
		if (entity->alias == NULL) {
			entity->anonymous = true;
            // TODO: Memory leak!
			asprintf(&entity->alias, "anon_%d", *entityID);
            (*entityID)++;
        }
    }
}

void Free_AST_CreateNode(AST_CreateNode *createNode) {
	if(!createNode) return;
	
	for(int i = 0; i < Vector_Size(createNode->graphEntities); i++) {
		AST_GraphEntity *ge;
		Vector_Get(createNode->graphEntities, i, &ge);
		Free_AST_GraphEntity(ge);
	}

	Vector_Free(createNode->graphEntities);
	free(createNode);
}