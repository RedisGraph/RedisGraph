/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/op_value_hash_join.h"
#include "../../../../arithmetic/arithmetic_expression.h"

typedef struct {
	RT_OpBase op;
	const OpValueHashJoin *op_desc;
	Record rhs_rec;                     // Right hand side record.
	int64_t intersect_idx;              // Current intersection, < number_of_intersections
	Record *cached_records;             // Cached left hand side records.
	uint join_value_rec_idx;            // position on joined expression within record.
	int64_t number_of_intersections;    // Number of intersections located.
} RT_OpValueHashJoin;

// Creates a new ValueHashJoin operation
RT_OpBase *RT_NewValueHashJoin(const RT_ExecutionPlan *plan, const OpValueHashJoin *op_desc);
