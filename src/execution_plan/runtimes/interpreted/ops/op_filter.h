/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../../filter_tree/filter_tree.h"

/* Filter
 * filters graph according to where cluase */
typedef struct {
	RT_OpBase op;
	FT_FilterNode *filterTree;
} RT_OpFilter;

/* Creates a new Filter operation */
RT_OpBase *RT_NewFilterOp(const RT_ExecutionPlan *plan, FT_FilterNode *filterTree);
