/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __EXECUTION_PLAN_OPTIMIZER_H__
#define __EXECUTION_PLAN_OPTIMIZER_H__

#include "../execution_plan.h"

/* Try to optimize an execution plan */
void optimizePlan(RedisModuleCtx *ctx, const char *graph_name, ExecutionPlan *plan);

#endif