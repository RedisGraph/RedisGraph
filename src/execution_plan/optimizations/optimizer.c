#include "./optimizer.h"
#include "./reduce_filters.h"

void optimizePlan(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g) {
    reduceFilters(plan);

    substituteIndexScans(ctx, plan, graph_name, g);
}
