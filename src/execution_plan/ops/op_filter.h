/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../filter_tree/filter_tree.h"

/* Filter
 * filters graph according to where cluase */
typedef struct {
	OpBase op;
	FT_FilterNode *filterTree;
} OpFilter;

/* Creates a new Filter operation */
OpBase *NewFilterOp(const ExecutionPlan *plan, FT_FilterNode *filterTree);
