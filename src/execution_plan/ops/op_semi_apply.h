/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "op_argument.h"
#include "../execution_plan.h"

/* SemiApply operation tests for the presence of a pattern
 * Normal Semi Apply: Starts by pulling on the main execution plan branch,
 * for each record received it tries to get a record from the match branch
 * if no data is produced it will try to fetch a new data point from the main execution plan branch,
 * otherwise the main execution plan branch record is passed onward.
 * Anti Semi Apply: Starts by pulling on the main execution plan branch,
 * for each record received it tries to get a record from the match branch
 * if no data is produced the main execution plan branch record is passed onward
 * otherwise it will try to fetch a new data point from the main execution plan branch. */

typedef struct OpSemiApply {
	OpBase op;
	Record r;                       // Bound branch record.
	OpBase *bound_branch;           // Bound branch root;
	OpBase *match_branch;           // Match branch root;
	Argument *op_arg;               // Match branch tap.
} OpSemiApply;

OpBase *NewSemiApplyOp(const ExecutionPlan *plan, bool anti);
