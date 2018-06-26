#include "./optimizer.h"
#include "./reduce_filters.h"

void optimizePlan(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name) {
    reduceFilters(plan);

    substituteIndexScans(ctx, plan, graph_name);
}
