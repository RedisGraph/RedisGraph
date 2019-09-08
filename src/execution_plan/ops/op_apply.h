/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"

typedef struct {
	OpBase op;
	bool init;
	Record lhs_record;
	Record *rhs_records;
	uint rhs_idx;
} Apply;

// OpBase* NewApplyOp(uint *modifies);
OpBase *NewApplyOp(void);
