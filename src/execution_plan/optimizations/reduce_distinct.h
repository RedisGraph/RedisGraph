/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* A distinct operation following an aggregation operation 
 * is unnecessary, as aggregation groups are guaranteed to be unique.
 * this optimization will try to look for an aggregation operation
 * followed by a distinct operation, in which case we can omit distinct
 * from the execution plan. */
void reduceDistinct(ExecutionPlanSegment *plan);
