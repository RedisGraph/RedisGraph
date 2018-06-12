#include "./optimizer.h"
#include "./reduce_filters.h"

void optimizePlan(ExecutionPlan *plan) {
    reduceFilters(plan);
}
