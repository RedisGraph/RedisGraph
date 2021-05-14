/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "rax.h"
#include "../execution_plan.h"

typedef struct {
	OpBase op;
	rax *found;
	rax *mapping;       // record mapping
	uint *offsets;      // offsets to expression values
	char **aliases;     // expression aliases to distinct by
	uint offset_count;  // number of offsets

} OpDistinct;

OpBase *NewDistinctOp(const ExecutionPlan *plan);

