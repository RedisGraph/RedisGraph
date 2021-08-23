/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "op_argument.h"
#include "../runtime_execution_plan.h"

typedef struct {
	OpBase op;
	Record r;                       // Bound branch record.
	RT_OpBase *bound_branch;        // Bound branch.
	RT_OpBase *rhs_branch;          // Right-hand branch.
	RT_Argument *op_arg;            // Right-hand branch tap.
} RT_Apply;

RT_OpBase *RT_NewApplyOp(const RT_ExecutionPlan *plan);
