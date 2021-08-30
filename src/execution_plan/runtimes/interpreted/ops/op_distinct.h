/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "rax.h"
#include "../../../ops/op_distinct.h"
#include "../runtime_execution_plan.h"

typedef struct {
	RT_OpBase op;
	const OpDistinct *op_desc;
	rax *found;
	rax *mapping;          // record mapping
	uint *offsets;         // offsets to expression values
} RT_OpDistinct;

RT_OpBase *RT_NewDistinctOp(const RT_ExecutionPlan *plan, const OpDistinct *op_desc);
