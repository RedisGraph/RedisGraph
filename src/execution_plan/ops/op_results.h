/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../redismodule.h"
#include "../../graph/graphcontext.h"
#include "../../resultset/resultset.h"

/* Results generates result set */

typedef struct {
	OpBase op;
	ResultSet *result_set;
	uint64_t result_set_size_limit;
} Results;

/* Creates a new Results operation */
OpBase *NewResultsOp(const ExecutionPlan *plan);
