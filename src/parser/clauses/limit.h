/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _CLAUSE_LIMIT_H
#define _CLAUSE_LIMIT_H

#include "../../util/vector.h"

typedef struct {
	int limit;
} AST_LimitNode;

AST_LimitNode *New_AST_LimitNode(int limit);
void Free_AST_LimitNode(AST_LimitNode *limitNode);

#endif
