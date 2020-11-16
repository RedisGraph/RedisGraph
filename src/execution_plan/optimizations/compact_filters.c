/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "compact_filters.h"
#include "../../RG.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../ops/op_filter.h"
#include "../../filter_tree/filter_tree.h"
#include "../execution_plan_build/execution_plan_modify.h"

// Try to compact a filter.
static inline bool _compactFilter(OpBase *op) {
	ASSERT(op->type == OPType_FILTER);
	OpFilter *filter_op = (OpFilter *)op;
	return FilterTree_Compact(filter_op->filterTree);
}

// In case the compacted filter resolved to 'true', remove it from the execution plan.
static void _removeTrueFilter(ExecutionPlan *plan, OpBase *op) {
	ASSERT(op->type == OPType_FILTER);
	OpFilter *filter_op = (OpFilter *)op;
	FT_FilterNode *root = filter_op->filterTree;
	// We can only have a contant expression in this point (after compaction).
	ASSERT(root->t == FT_N_EXP);
	// Evaluate the expression, and check if it is a 'true' value.
	SIValue bool_val = AR_EXP_Evaluate(root->exp.exp, NULL);
	if(SI_TYPE(bool_val) != T_BOOL) {
		// Value did not resolve to boolean, emit an error.
		Error_SITypeMismatch(bool_val, T_BOOL);
		SIValue_Free(bool_val);
		return;
	}
	if(SIValue_IsTrue(bool_val)) {
		ExecutionPlan_RemoveOp(plan, op);
		OpBase_Free(op);
	}
}

static void _compactFilters(ExecutionPlan *plan, OpBase *op) {
	if(op == NULL) return;

	// Try to compact the filter.
	bool compact = false;
	if(op->type == OPType_FILTER) {
		compact = _compactFilter(op);
	}

	// Try to compact children.
	for(int i = 0; i < op->childCount; i++) {
		_compactFilters(plan, op->children[i]);
	}

	// If there was a compaction, try to remove 'true' filters.
	if(compact) _removeTrueFilter(plan, op);
}

void compactFilters(ExecutionPlan *plan) {
	_compactFilters(plan, plan->root);
}

