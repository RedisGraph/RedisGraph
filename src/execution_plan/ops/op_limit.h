/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

typedef struct {
	OpBase op;
	unsigned int limit;     // Max number of records to consume.
	unsigned int consumed;  // Number of records consumed so far.
} OpLimit;

OpBase *NewLimitOp(const ExecutionPlan *plan);
