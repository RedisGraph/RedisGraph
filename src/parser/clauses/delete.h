/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _CLAUSE_DELETE_H
#define _CLAUSE_DELETE_H

#include "../../rmutil/vector.h"
#include "../../util/triemap/triemap.h"

typedef struct {	
	Vector *graphEntities; /* Vector of char pointers. */
} AST_DeleteNode;

AST_DeleteNode* New_AST_DeleteNode(Vector *elements);
void DeleteClause_ReferredNodes(const AST_DeleteNode *delete_node, TrieMap *referred_nodes);
void Free_AST_DeleteNode(AST_DeleteNode *delete_node);

#endif