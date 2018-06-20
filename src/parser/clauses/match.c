#include "./match.h"
#include "../ast_common.h"

AST_MatchNode* New_AST_MatchNode(Vector *elements) {
	AST_MatchNode *matchNode = (AST_MatchNode*)malloc(sizeof(AST_MatchNode));
	matchNode->graphEntities = elements;
	return matchNode;
}

void MatchClause_ReferredNodes(const AST_MatchNode *match_node, TrieMap *referred_nodes) {
	if(!match_node) return;
	int match_element_count = Vector_Size(match_node->graphEntities);

    for(int i = 0; i < match_element_count; i++) {
        AST_GraphEntity *match_element;
        Vector_Get(match_node->graphEntities, i, &match_element);
		TrieMap_Add(referred_nodes, match_element->alias, strlen(match_element->alias), NULL, NULL);
    }
}

AST_GraphEntity* MatchClause_GetEntity(const AST_MatchNode *matchNode, const char* alias) {
	if(!matchNode) return NULL;
	size_t match_entity_count = Vector_Size(matchNode->graphEntities);
	for(int i = 0; i < match_entity_count; i++) {
		AST_GraphEntity *ge;
		Vector_Get(matchNode->graphEntities, i, &ge);
		if(strcmp(ge->alias, alias) == 0) return ge;
	}

	return NULL;
}

void Free_AST_MatchNode(AST_MatchNode *matchNode) {
	if(!matchNode) return;
	for(int i = 0; i < Vector_Size(matchNode->graphEntities); i++) {
		AST_GraphEntity *ge;
		Vector_Get(matchNode->graphEntities, i, &ge);
		Free_AST_GraphEntity(ge);
	}

	Vector_Free(matchNode->graphEntities);
	free(matchNode);
}
