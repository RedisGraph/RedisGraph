#include "merge.h"
#include "../ast_common.h"

AST_MergeNode* New_AST_MergeNode() {
	AST_MergeNode *mergeNode = malloc(sizeof(AST_MergeNode));
	mergeNode->nodes = NewVector(AST_NodeEntity*, 1);
	return mergeNode;
}

void Free_AST_MergeNode(AST_MergeNode *mergeNode) {
	if(!mergeNode) return;

	AST_NodeEntity *node = NULL;
	while(Vector_Pop(mergeNode->nodes, node)) {
		Free_AST_GraphEntity(node);
	}

	Vector_Free(mergeNode->nodes);
	free(mergeNode);
}