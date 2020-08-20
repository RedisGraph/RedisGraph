/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_distinct.h"

void reduceDistinct(ExecutionPlan *plan) {
	// Look for Distinct operations.
	OpBase **distinct_ops = ExecutionPlan_CollectOps(plan->root, OPType_DISTINCT);

	for(uint i = 0; i < array_len(distinct_ops); i++) {
		OpBase *distinct = distinct_ops[i];
		assert(distinct->childCount == 1);
		if(distinct->children[0]->type == OPType_AGGREGATE) {
			// We can remove the Distinct op if its child is an aggregate operation,
			// as its results will inherently be unique.
			ExecutionPlan_RemoveOp(plan, distinct);
			OpBase_Free(distinct);
		}
	}

	array_free(distinct_ops);
}

