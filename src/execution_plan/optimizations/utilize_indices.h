/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __UTILIZE_INDICES_H__
#define __UTILIZE_INDICES_H__

#include "../execution_plan.h"
#include "../../index/index.h"
#include "../ops/ops.h"

/* The utilizeIndices optimization finds Label Scan operations with Filter parents and, if
 * any constant predicate filter matches a viable index, replaces the Label Scan with an Index Scan.
 * This allows for the consideration of fewer candidate nodes and significantly increases the speed
 * of the operation.
 */
void utilizeIndices(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g);

void updateScanOps(RedisModuleCtx *ctx, const char *graph_name, ExecutionPlan *plan);

#endif