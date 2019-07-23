/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GROUP_H_
#define GROUP_H_

#include "../value.h"
#include "../arithmetic/arithmetic_expression.h"

typedef struct {
	int key_count;
	SIValue *keys;
	AR_ExpNode
	**aggregationFunctions;   /* Array of AR_ExpNode*, where the root is an aggregation function. */
	Record r;   /* Representative record for all aggregated records in group. */
} Group;

/* Creates a new group */
Group *NewGroup(int key_count, SIValue *keys, AR_ExpNode **funcs, Record r);

/* Compute group key string representation, it is the callers
 * responsibility to free returned string. */
void Group_KeyStr(const Group *g, char **group_key);

void FreeGroup(Group *group);

#endif
