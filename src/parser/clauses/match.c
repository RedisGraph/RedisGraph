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

void MatchClause_ReferredNodes(const AST_MatchNode *matchNode, TrieMap *referred_nodes) {
	if(!matchNode) return;

	int entityCount = Vector_Size(matchNode->_mergedPatterns);
	for(int i = 0; i < entityCount; i++) {
		AST_GraphEntity *entity;
		Vector_Get(matchNode->_mergedPatterns, i, &entity);
		TrieMap_Add(referred_nodes, entity->alias, strlen(entity->alias), NULL, NULL);
	}
}

AST_GraphEntity* MatchClause_GetEntity(const AST_MatchNode *matchNode, const char* alias) {
	if(!matchNode) return NULL;

	int entityCount = Vector_Size(matchNode->_mergedPatterns);
	for(int i = 0; i < entityCount; i++) {
		AST_GraphEntity *entity;
		Vector_Get(matchNode->_mergedPatterns, i, &entity);
		if(strcmp(entity->alias, alias) == 0) return entity;
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
            asprintf(&entity->alias, "anon_%d", *entityID);
            (*entityID)++;
        }
    }
}

void Free_AST_MatchNode(AST_MatchNode *matchNode) {
	if(!matchNode) return;
	Vector *pattern;
	while(Vector_Pop(matchNode->patterns, &pattern)) {
		for(int i = 0; i < Vector_Size(pattern); i++) {
			AST_GraphEntity *ge;
			Vector_Get(pattern, i, &ge);
			Free_AST_GraphEntity(ge);
		}
		Vector_Free(pattern);
	}

	Vector_Free(matchNode->patterns);
	free(matchNode);
}
