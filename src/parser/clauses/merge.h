#ifndef _CLAUSE_MERGE_H
#define _CLAUSE_MERGE_H

#include "../../rmutil/vector.h"

typedef struct {
	Vector *graphEntities; /* Vector of AST_NodeEntity pointers. */
} AST_MergeNode;

AST_MergeNode* New_AST_MergeNode(Vector *graphEntities);
void Free_AST_MergeNode(AST_MergeNode *mergeNode);

#endif