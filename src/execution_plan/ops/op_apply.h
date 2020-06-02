/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "op_argument.h"
#include "../execution_plan.h"

typedef struct {
	OpBase op;
	Record r;                       // Bound branch record.
	OpBase *bound_branch;           // Bound branch.
	OpBase *rhs_branch;             // Right-hand branch.
	Argument *op_arg;               // Right-hand branch tap.
} Apply;

// OpBase* NewApplyOp(uint *modifies);
OpBase *NewApplyOp(const ExecutionPlan *plan);

