/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../../ops/op_filter.h"
#include "../runtime_execution_plan.h"
#include "../../../../filter_tree/filter_tree.h"

/* Filter
 * filters graph according to where cluase */
typedef struct {
	RT_OpBase op;
	const OpFilter *op_desc;
} RT_OpFilter;

/* Creates a new Filter operation */
RT_OpBase *RT_NewFilterOp(const RT_ExecutionPlan *plan, const OpFilter *op_desc);
