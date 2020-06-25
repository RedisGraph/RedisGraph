/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./optimizer.h"
#include "./optimizations.h"
#include "../../query_ctx.h"

void _optimizePlan(ExecutionPlan *plan) {
	// Tries to compact filter trees, and remove redundant filters.
	compactFilters(plan);

	/* Scan optimizations order:
	 * 1. First try to use the indices. Given a label scan and an indexed property, apply index scan.
	 * 2. Given a filter which checks id condition, and full or label scan, reduce it to id scan or label with id scan.
	 *    Note: Due to the scan optimization order, label scan will be replaced with index scan when possible, so the id filter remains.
	 * 3. Remove redundant scans which checks for the same node. */

	/* When possible, replace label scan and filter ops
	 * with index scans. */
	utilizeIndices(plan);

	// Try to reduce SCAN + FILTER to a node seek operation.
	seekByID(plan);

	/* Remove redundant SCAN operations. */
	reduceScans(plan);

	/* Try to optimize cartesian product */
	reduceCartesianProductStreamCount(plan);

	/* Try to match disjoint entities by applying a join */
	applyJoin(plan);

	/* Try to reduce a number of filters into a single filter op. */
	reduceFilters(plan);

	/* reduce traversals where both src and dest nodes are already resolved
	 * into an expand into operation. */
	reduceTraversal(plan);

	/* Try to reduce distinct if it follows aggregation. */
	reduceDistinct(plan);

	/* Try to reduce execution plan incase it perform node or edge counting. */
	reduceCount(plan);
}

void optimizePlan(ExecutionPlan *plan) {
	/* Handle UNION of execution plans. */
	if(plan->is_union) {
		uint segment_count = array_len(plan->segments);
		for(uint i = 0; i < segment_count; i++) _optimizePlan(plan->segments[i]);
	} else {
		_optimizePlan(plan);
	}
}
