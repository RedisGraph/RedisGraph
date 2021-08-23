/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
	OpBase op;
	AR_ExpNode **key_exps;        // array of expressions used to calculate the group key
	AR_ExpNode **aggregate_exps;  // array of expressions that aggregate data for each key
	uint key_count;               // number of key expressions
	uint aggregate_count;         // number of aggregating expressions
	bool should_cache_records;    // records should be cached if we're sorting after aggregation
} OpAggregate;

OpBase *NewAggregateOp(const ExecutionPlan *plan, AR_ExpNode **exps, bool should_cache_records);
