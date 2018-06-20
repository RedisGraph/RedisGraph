#ifndef _CLAUSE_MATCH_H
#define _CLAUSE_MATCH_H

#include "../ast_common.h"
#include "../../rmutil/vector.h"
#include "../../util/triemap/triemap.h"

typedef struct {
	Vector *graphEntities;
} AST_MatchNode;

AST_MatchNode* New_AST_MatchNode(Vector *elements);
void MatchClause_ReferredNodes(const AST_MatchNode *match_node, TrieMap *referred_nodes);
AST_GraphEntity* MatchClause_GetEntity(const AST_MatchNode *matchNode, const char* alias);
void Free_AST_MatchNode(AST_MatchNode *matchNode);

#endif