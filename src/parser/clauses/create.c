#include "./create.h"
#include "../ast_common.h"

AST_CreateNode* New_AST_CreateNode(Vector *elements) {
	AST_CreateNode *create_node = (AST_CreateNode*)malloc(sizeof(AST_CreateNode));
	create_node->graphEntities = elements;
	return create_node;
}

void CreateClause_ReferredNodes(const AST_CreateNode *create_node, TrieMap *referred_nodes) {
	if(!create_node) return;

	int entities_count = Vector_Size(create_node->graphEntities);

    for(int i = 0; i < entities_count; i++) {
        AST_GraphEntity *entity;
        Vector_Get(create_node->graphEntities, i, &entity);
		TrieMap_Add(referred_nodes, entity->alias, strlen(entity->alias), NULL, NULL);
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