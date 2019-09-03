/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

typedef struct {
	OpBase op;
	bool init;
	Record lhs_record;
	Record *rhs_records;
	uint rhs_idx;
} Apply;

OpBase *NewApplyOp(const ExecutionPlan *plan);
OpResult ApplyInit(OpBase *opBase);
Record ApplyConsume(OpBase *opBase);
OpResult ApplyReset(OpBase *opBase);
void ApplyFree(OpBase *opBase);
