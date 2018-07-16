/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./optimizer.h"
#include "./optimizations.h"

void optimizePlan(ExecutionPlan *plan) {
    /* Try convert filters into matrices
     * and incorporate them with traversal operations. */
    // translateFilters(plan);

    /* Try to reduce a number of filters into a single filter op. */
    reduceFilters(plan);
}
