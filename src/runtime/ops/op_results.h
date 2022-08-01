/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../../IR/execution_plan/execution_plan.h"
#include "../../redismodule.h"
#include "../../storage/graph/graphcontext.h"
#include "../../runtime/resultset/resultset.h"

/* Results generates result set */

typedef struct {
	OpBase op;
	ResultSet *result_set;
	uint64_t result_set_size_limit;
} Results;

/* Creates a new Results operation */
OpBase *NewResultsOp(const ExecutionPlan *plan);
