/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../value.h"
#include "../arithmetic/arithmetic_expression.h"

typedef struct {
	SIValue *keys;                       /* SIValues that form the key associated with each group. */
	AR_ExpNode **aggregationFunctions;   /* Nodes containing aggregate functions to be evaluated. */
	uint key_count;                      /* Number of SIValues in the key. */
	uint func_count;                     /* Number of aggregation function values. */
	Record r;   /* Representative record for all aggregated records in group. */
} Group;

/* Creates a new group */
Group *NewGroup(SIValue *keys, uint key_count, AR_ExpNode **funcs, uint func_count, Record r);

/* Compute group key string representation, it is the callers
 * responsibility to free returned string. */
void Group_KeyStr(const Group *g, char **group_key);

void FreeGroup(Group *group);

