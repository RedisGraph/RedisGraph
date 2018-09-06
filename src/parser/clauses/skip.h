/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _SKIP_H_
#define _SKIP_H_

#include <stdlib.h>

typedef struct {
	size_t skip; // Number of records to skip.
} AST_SkipNode;

AST_SkipNode* New_AST_SkipNode(size_t n);
void Free_AST_SkipNode(AST_SkipNode *skipNode);

#endif
