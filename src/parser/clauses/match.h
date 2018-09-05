/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _CLAUSE_MATCH_H
#define _CLAUSE_MATCH_H

#include "../ast_common.h"
#include "../../rmutil/vector.h"
#include "../../util/triemap/triemap.h"

typedef struct {
	Vector *patterns;			// Vector of vectors, each inner vector describes a pattern to match.
	Vector *_mergedPatterns;	// Vector of all entities mentioned within patterns.
} AST_MatchNode;

/* Create a new match */
AST_MatchNode* New_AST_MatchNode(Vector *patterns);

/* Populate referred_entities triemap with entities mentioned in this MATCH clause.*/
void MatchClause_ReferredEntities(const AST_MatchNode *matchNode, TrieMap *referred_entities);

/* Get an AST_GraphEntity* aliased as given alias. */
AST_GraphEntity* MatchClause_GetEntity(const AST_MatchNode *matchNode, const char* alias);

/* Give an alias to every entity which doesn't have an alias. */
void MatchClause_NameAnonymousNodes(AST_MatchNode *matchNode, int *entityID);

void Free_AST_MatchNode(AST_MatchNode *matchNode);

#endif
