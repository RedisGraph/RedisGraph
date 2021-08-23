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
	unsigned int skip;    // number of records to skip
	AR_ExpNode *skip_exp; // expression evaluated to 'skip'
} OpSkip;

// skips 'n' records
OpBase *NewSkipOp(const ExecutionPlan *plan, AR_ExpNode *skip_exp);
