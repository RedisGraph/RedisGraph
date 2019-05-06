/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __EXECUTION_PLAN_OPTIMIZER_H__
#define __EXECUTION_PLAN_OPTIMIZER_H__

#include "../execution_plan.h"

/* Try to optimize an execution plan segment. */
void optimizeSegment(GraphContext *gc, ExecutionPlanSegment *segment);

#endif
