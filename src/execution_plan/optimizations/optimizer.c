/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./optimizer.h"
#include "./optimizations.h"

void optimizePlan(GraphContext *gc, ExecutionPlan *plan) {
    /* When possible, replace label scan and filter ops
     * with index scans. */
    utilizeIndices(gc, plan);

    /* Try to reduce a number of filters into a single filter op. */
    reduceFilters(plan);

    /* Remove redundant SCAN operations. */
    // reduceScans(plan);
}
