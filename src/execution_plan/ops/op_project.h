/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
	OpBase op;
	AR_ExpNode **exps;              // Projected expressions.
	AR_ExpNode **order_exps;        // Order by expressions.
	uint *record_offsets;           // Record IDs corresponding to each projection (including order exps).
	bool singleResponse;            // When no child operations, return NULL after a first response.
	bool project_all;               // Projects all user-defined aliases (tracked for freeing logic).
	unsigned short exp_count;       // Number of projected expressions.
	unsigned short order_exp_count; // Number of order by expressions.
} OpProject;

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps, AR_ExpNode **order_exps);

