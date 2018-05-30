#ifndef _CLAUSE_LIMIT_H
#define _CLAUSE_LIMIT_H

#include "../../rmutil/vector.h"

typedef struct {
	int limit;
} AST_LimitNode;

AST_LimitNode* New_AST_LimitNode(int limit);
void Free_AST_LimitNode(AST_LimitNode *limitNode);

#endif
