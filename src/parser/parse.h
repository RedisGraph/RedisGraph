/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __QUERY_PARSER_PARSE_H__
#define __QUERY_PARSER_PARSE_H__

#include "ast.h"

typedef struct {
	AST **root;
	int ok;
	char *errorMsg;
} parseCtx;

#endif
