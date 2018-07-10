#ifndef _CLAUSE_CREATE_H
#define _CLAUSE_CREATE_H

#include "../../rmutil/vector.h"
#include "../../util/triemap/triemap.h"

typedef struct {
	Vector *graphEntities; /* Vector of AST_GraphEntity pointers. */
} AST_CreateNode;

AST_CreateNode* New_AST_CreateNode(Vector *patterns);
void CreateClause_ReferredNodes(const AST_CreateNode *createNode, TrieMap *referredNodes);
void CreateClause_NameAnonymousNodes(AST_CreateNode *createNode, int *entityID);
void Free_AST_CreateNode(AST_CreateNode *createNode);

#endif