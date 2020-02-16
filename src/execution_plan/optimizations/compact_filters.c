/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "compact_filters.h"
#include "../ops/op_filter.h"
#include "../../filter_tree/filter_tree.h"

// Try to compact a filter.
static inline bool _compactFilter(OpBase *op) {
	assert(op->type == OPType_FILTER);
	OpFilter *filter_op = (OpFilter *)op;
	return FilterTree_Compact(filter_op->filterTree);
}

// In case the compacted filter resolved to true, remove it from the execution plan.
static void _removeTrueFilter(ExecutionPlan *plan, OpBase *op) {
	assert(op->type == OPType_FILTER);
	OpFilter *filter_op = (OpFilter *)op;
	FT_FilterNode *root = filter_op->filterTree;
	assert(root->t == FT_N_EXP);
	SIValue bool_val = AR_EXP_Evaluate(root->exp.exp, NULL);
	assert(SI_TYPE(bool_val) == T_BOOL);
	if(bool_val.longval) {
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
