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

	/* Remove redundant SCAN operations. */
	reduceScans(plan);

	/* Try to match disjoint entities by applying a join */
	applyJoin(plan);

	/* Try to reduce a number of filters into a single filter op. */
	reduceFilters(plan);

	/* reduce traversals where both src and dest nodes are already resolved
	 * into an expand into operation. */
	reduceTraversal(plan);

	/* Relocate sort, skip, limit operations. */
	relocateOperations(plan);

	/* Try to reduce distinct if it follows aggregation. */
	reduceDistinct(plan);

	/* Try to reduce execution plan incase it perform node or edge counting. */
	reduceCount(plan);

}
