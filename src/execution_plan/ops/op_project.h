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
	AR_ExpNode **exps;              // Projected expressions (including order exps).
	uint *record_offsets;           // Record IDs corresponding to each projection (including order exps).
	bool singleResponse;            // When no child operations, return NULL after a first response.
	bool project_all;               // Projects all user-defined aliases (tracked for freeing logic).
	uint exp_count;                 // Number of expressions that should be freed with this op.
} OpProject;

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps, AR_ExpNode **order_exps);

