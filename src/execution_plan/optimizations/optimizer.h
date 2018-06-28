#ifndef __EXECUTION_PLAN_OPTIMIZER_H__
#define __EXECUTION_PLAN_OPTIMIZER_H__

#include "../execution_plan.h"
#include "../ops/ops.h"

/* Try to optimize an execution plan */
void optimizePlan(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g);

/* Replace LabelScans with IndexScans when viable */
void substituteIndexScans(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g);

#endif