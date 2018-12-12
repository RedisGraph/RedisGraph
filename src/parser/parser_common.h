/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __PARSER_COMMON_H__
#define __PARSER_COMMON_H__
#include <stdlib.h>
#include "ast.h"

AST *Query_Parse(const char *q, size_t len, char **err);
#endif
