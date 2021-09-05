/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

typedef struct {
	OpBase op;
	AR_ExpNode *limit_exp;  // Expression evaluated to limit.
} OpLimit;

// Limits number of produced records
OpBase *NewLimitOp(const ExecutionPlan *plan, AR_ExpNode *limit_exp);

