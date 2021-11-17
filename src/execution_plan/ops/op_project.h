/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
	OpBase op;
	Record r;                       // Input Record being read from (stored to free if we encounter an error).
	Record intermidiate;
	rax   *intermidiate_rax;
	Record projection;              // Record projected by this operation (stored to free if we encounter an error).
	AR_ExpNode **exps;              // Projected expressions (including order exps).
	AR_ExpNode **order_exps;
	uint *record_offsets;           // Record IDs corresponding to each projection (including order exps).
	bool singleResponse;            // When no child operations, return NULL after a first response.
	uint exp_count;                 // Number of projected expressions.
	uint order_exp_count;
} OpProject;

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps, AR_ExpNode **order_exps);

