/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _CLAUSE_LIMIT_H
#define _CLAUSE_LIMIT_H

#include "../../util/vector.h"

typedef struct {
	int limit;
} AST_LimitNode;

AST_LimitNode* New_AST_LimitNode(int limit);
void Free_AST_LimitNode(AST_LimitNode *limitNode);

#endif
