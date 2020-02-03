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
	AR_ExpNode **input_exps;        // Expressions to evaluate on the input Record.
	AR_ExpNode **output_exps;       // Expressions to evaluate on the projected Record.
	uint *record_offsets;           // Record IDs corresponding to each projection (including order exps).
	bool singleResponse;            // When no child operations, return NULL after a first response.
} OpProject;

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **input_exps, AR_ExpNode **output_exps);

