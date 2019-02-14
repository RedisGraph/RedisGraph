/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _CLAUSE_CREATE_H
#define _CLAUSE_CREATE_H

#include "../../util/vector.h"
#include "../../util/triemap/triemap.h"

typedef struct {
	Vector *graphEntities; /* Vector of AST_GraphEntity pointers. */
} AST_CreateNode;

AST_CreateNode* New_AST_CreateNode(Vector *patterns);
void CreateClause_ReferredEntities(const AST_CreateNode *createNode, TrieMap *referredNodes);
void CreateClause_DefinedEntities(const AST_CreateNode *createNode, TrieMap *identifiers);
void CreateClause_NameAnonymousNodes(AST_CreateNode *createNode, int *entityID);
void Free_AST_CreateNode(AST_CreateNode *createNode);

#endif