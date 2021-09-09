/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../RG.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../runtimes/interpreted/ops/op_filter.h"
#include "../../filter_tree/filter_tree.h"
#include "../runtimes/interpreted/execution_plan_build/runtime_execution_plan_modify.h"

/* The compact filters optimizer scans an execution plan for filters that can be
 * compressed. In case the filter is compressed into a final constant 'true' value,
 * the filter operation will be removed from the execution plan. */

// Try to compact a filter.
static inline bool _compactFilter(RT_OpBase *op) {
	ASSERT(op->op_desc->type == OPType_FILTER);
	RT_OpFilter *filter_op = (RT_OpFilter *)op;
	return FilterTree_Compact(filter_op->filterTree);
}

// In case the compacted filter resolved to 'true', remove it from the execution plan.
static void _removeTrueFilter(RT_ExecutionPlan *plan, RT_OpBase *op) {
	ASSERT(op->op_desc->type == OPType_FILTER);
	RT_OpFilter *filter_op = (RT_OpFilter *)op;
	FT_FilterNode *root = filter_op->filterTree;
	// We can only have a contant expression in this point (after compaction).
	ASSERT(root->t == FT_N_EXP);
	// Evaluate the expression, and check if it is a 'true' value.
	SIValue bool_val = AR_EXP_Evaluate(root->exp.exp, NULL);
	if(SI_TYPE(bool_val) != T_BOOL && SI_TYPE(bool_val) != T_NULL) {
		// Value did not resolve to boolean, emit an error.
		Error_SITypeMismatch(bool_val, T_BOOL);
		SIValue_Free(bool_val);
		return;
	}
	if(SIValue_IsTrue(bool_val)) {
		RT_ExecutionPlan_RemoveOp(plan, op);
		RT_OpBase_Free(op);
	}
}

static void _compactFilters(RT_ExecutionPlan *plan, RT_OpBase *op) {
	if(op == NULL) return;

	// Try to compact the filter.
	bool compact = false;
	if(op->op_desc->type == OPType_FILTER) {
		compact = _compactFilter(op);
	}

	// Try to compact children.
	for(int i = 0; i < op->childCount; i++) {
		_compactFilters(plan, op->children[i]);
	}

	// If there was a compaction, try to remove 'true' filters.
	if(compact) _removeTrueFilter(plan, op);
}

void compactFilters(RT_ExecutionPlan *plan) {
	_compactFilters(plan, plan->root);
}

