/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../../../ops/op_skip.h"
#include "../runtime_execution_plan.h"

typedef struct {
	RT_OpBase op;
	const OpSkip *op_desc;
	AR_ExpNode *skip_exp; // expression evaluated to 'skip'
	unsigned int skip;    // number of records to skip
	unsigned int skipped; // number of records already skipped
} RT_OpSkip;

// Skips 'n' records.
RT_OpBase *RT_NewSkipOp(const RT_ExecutionPlan *plan, const OpSkip *op_desc);
