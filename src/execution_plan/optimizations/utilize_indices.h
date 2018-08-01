/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __UTILIZE_INDICES_H__
#define __UTILIZE_INDICES_H__

#include "../execution_plan.h"
#include "../ops/ops.h"

/* Replace LabelScans with IndexScans when viable */
void utilizeIndices(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g);

#endif