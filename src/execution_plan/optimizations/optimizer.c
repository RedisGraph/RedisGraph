/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./optimizer.h"
#include "./optimizations.h"

void optimizePlan(GraphContext *gc, ExecutionPlan *plan) {
    // Try to reduce SCAN + FILTER to a node seek operation.
    seekByID(plan);

    /* When possible, replace label scan and filter ops
     * with index scans. */
    utilizeIndices(gc, plan);

    /* Try to reduce a number of filters into a single filter op. */
    reduceFilters(plan);

    /* Remove redundant SCAN operations. */
    // reduceScans(plan);

    /* Relocate sort, skip, limit operations. */
    relocateOperations(plan);
    
    /* Try to reduce distinct if it follows aggregation. */
    reduceDistinct(plan);

    /* Try to reduce execution plan incase it perform node counting. */
    reduceCount(plan);
}
