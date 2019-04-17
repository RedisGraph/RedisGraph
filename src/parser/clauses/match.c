/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./match.h"
#include "../ast_common.h"

AST_MatchNode* New_AST_MatchNode(Vector *patterns) {
	AST_MatchNode *matchNode = (AST_MatchNode*)malloc(sizeof(AST_MatchNode));
	matchNode->patterns = patterns;
	matchNode->_mergedPatterns = NewVector(AST_GraphEntity*, 1);

	for(int i = 0; i < Vector_Size(patterns); i++) {
		Vector *pattern;
		Vector_Get(patterns, i, &pattern);

		size_t patternLength = Vector_Size(pattern);
		for(int j = 0; j < patternLength; j++) {
			AST_GraphEntity *ge;
			Vector_Get(pattern, j, &ge);
			Vector_Push(matchNode->_mergedPatterns, ge);
		}
	}
	return matchNode;
}

void MatchClause_DefinedEntities(const AST_MatchNode *matchNode, TrieMap *definedEntities) {
	if(!matchNode) return;

	int entityCount = Vector_Size(matchNode->_mergedPatterns);
	for(int i = 0; i < entityCount; i++) {
		AST_GraphEntity *entity;
		Vector_Get(matchNode->_mergedPatterns, i, &entity);
		if(!entity->alias) continue;
		TrieMap_Add(definedEntities, entity->alias, strlen(entity->alias), entity, TrieMap_DONT_CARE_REPLACE);
	}
}

AST_GraphEntity* MatchClause_GetEntity(const AST_MatchNode *matchNode, const char* alias) {
	if(!matchNode) return NULL;

	int entityCount = Vector_Size(matchNode->_mergedPatterns);
	for(int i = 0; i < entityCount; i++) {
		AST_GraphEntity *entity;
		Vector_Get(matchNode->_mergedPatterns, i, &entity);
		if(entity->alias && strcmp(entity->alias, alias) == 0) return entity;
	}

	return NULL;
}

void MatchClause_NameAnonymousNodes(AST_MatchNode *matchNode, int *entityID) {
	if(!matchNode) return;

	int entityCount = Vector_Size(matchNode->_mergedPatterns);
	for(int i = 0; i < entityCount; i++) {
		AST_GraphEntity *entity;
		Vector_Get(matchNode->_mergedPatterns, i, &entity);
        if (entity->alias == NULL) {
			entity->anonymous = true;
			// TODO: Memory leak!
            asprintf(&entity->alias, "anon_%d", *entityID);
            (*entityID)++;
        }
    }
}

void Free_AST_MatchNode(AST_MatchNode *matchNode) {
	if(!matchNode) return;
	Vector *pattern;
	while(Vector_Pop(matchNode->patterns, &pattern)) {
		Vector_Free(pattern);
	}

    AST_GraphEntity *ge;
	while(Vector_Pop(matchNode->_mergedPatterns, &ge)) {
        Free_AST_GraphEntity(ge);
    }

	Vector_Free(matchNode->_mergedPatterns);
	Vector_Free(matchNode->patterns);
	free(matchNode);
}

