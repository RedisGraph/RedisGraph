/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "op_argument.h"
#include "../execution_plan.h"

// the RollUpApply op has a bound left-hand branch
// and a right-hand branch that projects a single alias
// rollUpApply collects these expressions into an array
// that is merged into the Record from the left-hand side
// this is unlike the standard Apply op in that for every record
// retrieved from the left-hand branch, the results of the right-hand
// branch are concatenated into a list, rather than each record
// from the right-hand branch being merged into the left-hand record
// and passed upward once
typedef struct {
	OpBase op;
	OpBase *bound_branch;  // bound branch
	OpBase *rhs_branch;    // right-hand branch
	Argument *op_arg;      // right-hand branch tap
	uint alias_idx;        // index into Record of alias to be collected
	const char *alias;     // alias to be collected from right-hand branch
} RollUpApply;

OpBase *NewRollUpApplyOp(const ExecutionPlan *plan, const char *alias);

