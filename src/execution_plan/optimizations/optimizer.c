/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./optimizer.h"
#include "./optimizations.h"

void optimizePlan(ExecutionPlan *plan) {
	/* Scan optimizations order:
	 * 1. Remove redundant scans which checks for the same node.
	 * 2. Try to use the indices. Given a label scan and an indexed property, apply index scan.
	 * 3. Given a filter which checks id condition, and full or label scan, reduce it to id scan or label with id scan.
	 *    Note: Due to the scan optimization order, label scan will be replaced with index scan when possible, so the id filter remains. */

	// Remove redundant SCAN operations.
	reduceScans(plan);

	// When possible, replace label scan and filter ops with index scans.
	utilizeIndices(plan);

	// Try to reduce SCAN + FILTER to a node seek operation.
	seekByID(plan);

	// Migrate filters on variable-length edges into the traversal operations.
	filterVariableLengthEdges(plan);

	// Try to optimize cartesian product.
	reduceCartesianProductStreamCount(plan);

	// Try to match disjoint entities by applying a join.
	applyJoin(plan);

	// Reduce traversals where both src and dest nodes are already resolved into an expand into operation.
	reduceTraversal(plan);

	// Try to reduce distinct if it follows aggregation.
	reduceDistinct(plan);
}

void optimize_RTPlan(RT_ExecutionPlan *plan) {
	// Tries to compact filter trees, and remove redundant filters.
	compactFilters(plan);

	// Try to reduce a number of filters into a single filter op.
	reduceFilters(plan);

	// Try to reduce execution plan incase it perform node or edge counting.
	reduceCount(plan);
	
	// Let operations know about specified limit(s)
	applyLimit(plan);

	// Let operations know about specified skip(s)
	applySkip(plan);
}
