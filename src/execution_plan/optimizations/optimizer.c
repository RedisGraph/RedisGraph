#include "./optimizer.h"
#include "./optimizations.h"

void optimizePlan(ExecutionPlan *plan) {
    /* Try convert filters into matrices
     * and incorporate them with traversal operations. */
    // translateFilters(plan);

    /* Try to reduce a number of filters into a single filter op. */
    reduceFilters(plan);
}
