/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./optimizer.h"
#include "./optimizations.h"

void optimizePlan(ExecutionPlan *plan) {
	// tries to compact filter trees, and remove redundant filters
	compactFilters(plan);

	// scan optimizations order:
	// 1. migrate argument ops that may have erroneously been made
	//    the child of a cartesian product
	// 2. remove redundant scans which checks for the same node
	// 3. try to use the indices
	//    given a label scan and an indexed property, apply index scan
	// 4. given a filter which checks id condition, and full or label scan
	//    reduce it to id scan or label with id scan
	//    note: due to the scan optimization order
	//          label scan will be replaced with index scan when possible
	//          so the id filter remains

	// migrate misplaced ARGUMENT operations
	migrateArguments(plan);

	// remove redundant SCAN operations
	reduceScans(plan);

	// when possible, replace label scan and filter ops with index scans
	utilizeIndices(plan);

	// scan label with least entities
	optimizeLabelScan(plan);

	// try to reduce SCAN + FILTER to a node seek operation
	seekByID(plan);

	// migrate filters on variable-length edges into the traversal operations
	filterVariableLengthEdges(plan);

	// try to optimize cartesian product
	reduceCartesianProduct(plan);

	// try to match disjoint entities by applying a join
	applyJoin(plan);

	// try to reduce a number of filters into a single filter op
	reduceFilters(plan);

	// reduce traversals where both src and dest nodes are already resolved
	// into an expand into operation
	reduceTraversal(plan);

	// try to reduce distinct if it follows aggregation
	reduceDistinct(plan);

	// try to reduce execution plan incase it perform node or edge counting
	reduceCount(plan);

	// let operations know about specified limit(s)
	applyLimit(plan);

	// let operations know about specified skip(s)
	applySkip(plan);
}

