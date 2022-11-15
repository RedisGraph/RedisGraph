/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
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
