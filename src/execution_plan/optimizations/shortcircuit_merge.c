/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "../ops/op_merge.h"
#include "../ops/op_limit.h"
#include "../execution_plan.h"
#include "../execution_plan_build/execution_plan_modify.h"

// In case where a merge operation isn't required to return records i.e.
// MERGE (n:L {v:1})
// 
// there's no need to find all patterns in the graph matching the specified
// merge pattern (if such exists)
//
// this is not ture if the query is re-written as:
// MERGE (n:L {v:1}) RETURN n
// or
// MERGE (n:L {v:1}) ON MATCH SET n.v = 2
//
// in which case we do care to process each matching pattern

void shortcircuitMerge(ExecutionPlan *plan) {
	// merge require to pass its records, return
	if(OpBase_Type(plan->root) != OPType_MERGE) return;

	OpBase *op = plan->root;
	OpMerge *merge = (OpMerge*)op;

	// merge require to process each match, return
	if(merge->on_match != NULL) return;

	// merge is the last operation, and it isn't required to pass down its
	// finding, in this case we can skip pattern matching upon first match

	// locate "match" stream
	uint child_count = op->childCount;
	ASSERT(child_count == 2 || child_count == 3);
	OpBase *match_stream = NULL;

	if(child_count == 2) {
		if(ExecutionPlan_LocateOp(op->children[0], OPType_MERGE_CREATE)) {
			match_stream = op->children[1];
		} else {
			match_stream = op->children[0];
		}
	} else {
		// 3 streams, bound, create and match
		for(int i = 0; i < 3; i++) {
			OpBase *child = op->children[i];
			if(ExecutionPlan_LocateOp(child, OPType_MERGE_CREATE)) continue;

			bool child_has_merge = ExecutionPlan_LocateOp(child, OPType_MERGE);
			if(child_has_merge) continue;

			bool child_has_argument = ExecutionPlan_LocateOp(child, OPType_ARGUMENT);
			if(!child_has_argument) continue;

			match_stream = op->children[i];
			break;
		}
	}

	ASSERT(match_stream != NULL);
	OpBase *limit = NewLimitOp(plan, AR_EXP_NewConstOperandNode(SI_LongVal(1)));
	ExecutionPlan_PushBelow(match_stream, limit);
}

