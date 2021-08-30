/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../../../ops/op_results.h"
#include "../runtime_execution_plan.h"
#include "../../../../resultset/resultset.h"

/* Results generates result set */

typedef struct {
	RT_OpBase op;
	const Results *op_desc;
	ResultSet *result_set;
	uint64_t result_set_size_limit;
} RT_Results;

/* Creates a new Results operation */
RT_OpBase *RT_NewResultsOp(const RT_ExecutionPlan *plan, const Results *op_desc);
