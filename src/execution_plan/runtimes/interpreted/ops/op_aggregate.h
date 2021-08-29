/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/op_aggregate.h"
#include "../../../../grouping/group_cache.h"
#include "../../../../arithmetic/arithmetic_expression.h"

typedef struct {
	RT_OpBase op;
	const OpAggregate *op_desc;
	uint *record_offsets;               // record IDs for key and aggregate exps
	rax *groups;                        // map of all groups built by this operation
	Group *group;                       // last accessed group
	SIValue *group_keys;                // array of values that represent a key associated with a Group of aggregations
	CacheGroupIterator *group_iter;     // iterator for walking all groups
} RT_OpAggregate;

RT_OpBase *RT_NewAggregateOp(const RT_ExecutionPlan *plan, const OpAggregate *op_desc);
