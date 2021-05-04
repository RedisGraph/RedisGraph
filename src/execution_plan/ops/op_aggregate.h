/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../grouping/group_cache.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
	OpBase op;
	uint *record_offsets;               /* Record IDs for key and aggregate exps. */
	AR_ExpNode **key_exps;              /* Array of expressions used to calculate the group key. */
	AR_ExpNode **aggregate_exps;        /* Array of expressions that aggregate data for each key. */
	rax *groups;                        /* Map of all groups built by this operation. */
	Group *group;                       /* Last accessed group. */
	SIValue *group_keys;                /* Array of values that represent a key associated with a Group of aggregations. */
	CacheGroupIterator *group_iter;     /* Iterator for walking all groups. */
	uint key_count;                     /* Number of key expressions. */
	uint aggregate_count;               /* Number of aggregating expressions. */
	bool should_cache_records;          /* Records should be cached if we're sorting after aggregation. */
} OpAggregate;

OpBase *NewAggregateOp(const ExecutionPlan *plan, AR_ExpNode **exps, bool should_cache_records);

