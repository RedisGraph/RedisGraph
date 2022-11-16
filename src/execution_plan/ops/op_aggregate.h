/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../grouping/group_cache.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
	OpBase op;
	uint *record_offsets;               // record IDs for key and aggregate exps
	AR_ExpNode **key_exps;              // array of expressions used to calculate the group key
	AR_ExpNode **aggregate_exps;        // array of expressions that aggregate data for each key
	rax *groups;                        // map of all groups built by this operation
	Group *group;                       // last accessed group
	SIValue *group_keys;                // array of values that represent a key associated with a Group of aggregations
	CacheGroupIterator *group_iter;     // iterator for walking all groups
	uint key_count;                     // number of key expressions
	uint aggregate_count;               // number of aggregating expressions
	bool should_cache_records;          // records should be cached if we're sorting after aggregation
} OpAggregate;

OpBase *NewAggregateOp(const ExecutionPlan *plan, AR_ExpNode **exps, bool should_cache_records);

