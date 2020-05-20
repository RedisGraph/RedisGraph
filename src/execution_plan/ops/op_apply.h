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
	bool init;
	uint rhs_idx;
	Record lhs_record;
	Record *rhs_records;
} Apply;

// OpBase* NewApplyOp(uint *modifies);
OpBase *NewApplyOp(const ExecutionPlan *plan);
