/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "op_argument.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/op_semi_apply.h"

/* SemiApply operation tests for the presence of a pattern
 * Normal Semi Apply: Starts by pulling on the main execution plan branch,
 * for each record received it tries to get a record from the match branch
 * if no data is produced it will try to fetch a new data point from the main execution plan branch,
 * otherwise the main execution plan branch record is passed onward.
 * Anti Semi Apply: Starts by pulling on the main execution plan branch,
 * for each record received it tries to get a record from the match branch
 * if no data is produced the main execution plan branch record is passed onward
 * otherwise it will try to fetch a new data point from the main execution plan branch. */

typedef struct RT_OpSemiApply {
	RT_OpBase op;
	const OpSemiApply *op_desc;
	Record r;                       // Bound branch record.
	RT_OpBase *bound_branch;        // Bound branch root;
	RT_OpBase *match_branch;        // Match branch root;
	RT_Argument *op_arg;            // Match branch tap.
} RT_OpSemiApply;

RT_OpBase *RT_NewSemiApplyOp(const RT_ExecutionPlan *plan, const OpSemiApply *op_desc);
