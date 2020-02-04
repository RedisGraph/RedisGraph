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
	Record intermediate_record;        // Intermediate Record in case we need to access inputs and outputs.
	intptr_t *intermediate_record_ids; // IDs for populating the intermediate Record.
	AR_ExpNode **projection_exps;      // Expressions to evaluate on the input Record.
	AR_ExpNode **order_exps;           // Expressions to evaluate on the intermediate Record.
	uint *record_offsets;              // Record IDs corresponding to all projections (including order exps).
	bool singleResponse;               // When no child operations, return NULL after a first response.
} OpProject;

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **input_exps, AR_ExpNode **output_exps);

