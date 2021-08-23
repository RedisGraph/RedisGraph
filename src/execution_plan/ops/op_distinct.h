/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "rax.h"
#include "../execution_plan.h"

typedef struct {
	OpBase op;
	const char **aliases;  // expression aliases to distinct by

} OpDistinct;

OpBase *NewDistinctOp(const ExecutionPlan *plan, const char **aliases, uint alias_count);
