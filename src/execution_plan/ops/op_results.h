/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

/* Results generates result set */

typedef struct {
	OpBase op;
} Results;

/* Creates a new Results operation */
OpBase *NewResultsOp(const ExecutionPlan *plan);
