/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

typedef struct {
	OpBase op;
	unsigned int skip;    // number of records to skip
	unsigned int skipped; // number of records already skipped
	AR_ExpNode *skip_exp; // expression evaluated to 'skip'
} OpSkip;

// Skips 'n' records.
OpBase *NewSkipOp(const ExecutionPlan *plan, AR_ExpNode *skip_exp);

