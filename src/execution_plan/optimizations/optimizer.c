/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./optimizer.h"
#include "./optimizations.h"

void optimizePlan(ExecutionPlan *plan, AST *ast) {
    /* When possible, replace label scan and filter ops
     * with index scans. */
    utilizeIndices(plan, ast);

    /* Try to reduce a number of filters into a single filter op. */
    reduceFilters(plan);

    /* Remove redundant SCAN operations. */
    // reduceScans(plan);

    /* Relocate sort, skip, limit operations. */
    relocateOperations(plan);
    
    /* Try to reduce distinct if it follows aggregation. */
    reduceDistinct(plan);
}
