/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "op_argument.h"
#include "../execution_plan.h"

/* The Apply op has a bound left-hand branch
 * and a right-hand branch that takes records
 * from the left-hand branch as input.
 * After retrieving a record from the left-hand branch,
 * it pulls data from the right-hand branch and passes
 * the merged record upwards until the right-hand branch
 * is depleted, at which time data is pulled from the
 * left-hand branch and the process repeats.  */
typedef struct {
	OpBase op;
	Record r;                       // Bound branch record.
	OpBase *bound_branch;           // Bound branch.
	OpBase *rhs_branch;             // Right-hand branch.
	Argument *op_arg;               // Right-hand branch tap.
} Apply;

OpBase *NewApplyOp(const ExecutionPlan *plan);

