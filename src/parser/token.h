/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <stdlib.h>

typedef struct {
	int64_t longval;
	double dval;
	char *strval;
	char *s;  // token string
	int pos;  // position in the query
} Token;

extern Token tok;

#endif
