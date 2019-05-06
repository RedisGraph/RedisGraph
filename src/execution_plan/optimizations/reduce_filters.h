/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __REDUCE_FILTERS_H__
#define __REDUCE_FILTERS_H__

#include "../execution_plan.h"

/* The reduce filters optimizer scans an execution plans for 
 * consecutive filter operations, these can be reduced down into
 * a single filter operation by ANDing their filter trees
 * Reducing the overall number of operations is expected to produce
 * faster execution time. */
void reduceFilters(ExecutionPlanSegment *plan);

#endif
