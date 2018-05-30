#ifndef _CLAUSE_MATCH_H
#define _CLAUSE_MATCH_H

#include "../../rmutil/vector.h"
#include "../../util/triemap/triemap.h"

typedef struct {
	Vector *graphEntities;
} AST_MatchNode;

AST_MatchNode* New_AST_MatchNode(Vector *elements);
void MatchClause_ReferredNodes(const AST_MatchNode *match_node, TrieMap *referred_nodes);
void Free_AST_MatchNode(AST_MatchNode *matchNode);

#endif