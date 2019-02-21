/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _CLAUSE_MERGE_H
#define _CLAUSE_MERGE_H

#include "../../util/vector.h"
#include "../../util/triemap/triemap.h"

typedef struct {
	Vector *graphEntities; /* Vector of AST_GraphEntity pointers. */
} AST_MergeNode;

AST_MergeNode* New_AST_MergeNode(Vector *graphEntities);
void MergeClause_NameAnonymousNodes(const AST_MergeNode *mergeNode, int *entityID);
void MergeClause_DefinedEntities(const AST_MergeNode *merge_node, TrieMap *defined_entities);
void Free_AST_MergeNode(AST_MergeNode *mergeNode);

#endif
