/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./optimizer.h"
#include "./optimizations.h"

void optimizeSegment(GraphContext *gc, ExecutionPlanSegment *segment) {
    // Try to reduce SCAN + FILTER to a node seek operation.
    seekByID(segment);

    /* When possible, replace label scan and filter ops
     * with index scans. */
    utilizeIndices(gc, segment);

    /* Try to reduce a number of filters into a single filter op. */
    reduceFilters(segment);

    /* Remove redundant SCAN operations. */
    // reduceScans(segment);

    /* Relocate sort, skip, limit operations. */
    relocateOperations(segment);
    
    /* Try to reduce distinct if it follows aggregation. */
    reduceDistinct(segment);

    /* Try to reduce execution segment incase it perform node counting. */
    reduceCount(segment);
}
