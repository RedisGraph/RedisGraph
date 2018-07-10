#include "merge.h"
#include "../ast_common.h"

AST_MergeNode* New_AST_MergeNode(Vector *graphEntities) {
	AST_MergeNode *mergeNode = malloc(sizeof(AST_MergeNode));
	mergeNode->graphEntities = graphEntities;
	return mergeNode;
}

void Free_AST_MergeNode(AST_MergeNode *mergeNode) {
	if(!mergeNode) return;

	/* There's no need in freeing entities vector
	 * as it is being used by match clause,
	 * which will free it. */
	free(mergeNode);
}
