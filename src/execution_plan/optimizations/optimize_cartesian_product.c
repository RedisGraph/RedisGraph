/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "optimize_cartesian_product.h"
#include "../ops/op_filter.h"
#include "../ops/op_cartesian_product.h"
#include "optimizations_util.h"

#define NOT_RESOLVED -1

// Tests to see if given filter can be re-positioned.
static inline bool _applicableFilter(const FT_FilterNode *f) {
	// Can only convert filters that test equality
	if(f->t != FT_N_PRED) return false;
	// TODO allowing AR_ExpNodes that refer directly to graph entities currently causes memory errors.
	// This restriction should be lifted later.
	if((f->pred.lhs->type == AR_EXP_OPERAND &&
		f->pred.lhs->operand.type == AR_EXP_VARIADIC &&
		f->pred.lhs->operand.variadic.entity_prop == NULL) ||
	   (f->pred.rhs->type == AR_EXP_OPERAND &&
		f->pred.rhs->operand.type == AR_EXP_VARIADIC &&
		f->pred.rhs->operand.variadic.entity_prop == NULL)) return false;
	return true;
}

// Collects all consecutive filters beneath given op.
static OpFilter **_locate_filters(OpBase *cp) {
	OpBase *parent = cp->parent;
	OpFilter **filters = array_new(OpFilter *, 0);

	while(parent && parent->type == OPType_FILTER) {
		OpFilter *filter_op = (OpFilter *)parent;
		if(_applicableFilter(filter_op->filterTree)) filters = array_append(filters, filter_op);
		parent = parent->parent;
	}

	return filters;
}

static OpBase *_chain_cp_and_filter(const ExecutionPlan *plan, OpBase *lhs, OpBase *rhs,
									OpBase *filter) {
	OpBase *cp = NewCartesianProductOp(plan);
	ExecutionPlan_AddOp(cp, lhs);
	ExecutionPlan_AddOp(cp, rhs);
	ExecutionPlan_AddOp(filter, cp);
	return filter;
}

static void _optimize_cartesian_product(ExecutionPlan *plan, OpBase *cp) {
	// Retrieve all filter operations located upstream from the Cartesian Product.
	OpFilter **filter_ops = _locate_filters(cp);
	uint filter_count = array_len(filter_ops);
	/* This array will hold filters which are possible to re position in the execution plan,
	 * but during the optimization construction they will not cause this optimization to execute. */
	OpFilter **pending_filters = array_new(OpFilter *, filter_count);
	OpBase *pending_filters_lookup_root = cp;

	// For each stream joined by the Cartesian product, collect all entities the stream resolves.
	int stream_count = cp->childCount;
	rax *stream_entities[stream_count];
	build_streams_from_op(cp, stream_entities, stream_count);

	for(uint i = 0; i < filter_count; i ++) {
		// Try to create a new dual-branched cartesian product, followed by the current filter.
		OpFilter *filter_op = filter_ops[i];

		/* A new cartesian product can be generated if both sides of the filter can be fully and
		 * separately resolved by exactly two child streams. */
		FT_FilterNode *f = filter_op->filterTree;

		/* Make sure LHS of the filter is resolved by a stream. */
		AR_ExpNode *lhs = f->pred.lhs;
		uint lhs_resolving_stream = relate_exp_to_stream(lhs, stream_entities, stream_count);
		if(lhs_resolving_stream == NOT_RESOLVED) continue;
		/* Make sure RHS of the filter is resolved by a stream. */
		AR_ExpNode *rhs = f->pred.rhs;
		uint rhs_resolving_stream = relate_exp_to_stream(rhs, stream_entities, stream_count);
		if(rhs_resolving_stream == NOT_RESOLVED) continue;

		// This stream is solved by a single cartesian product child and needs to be propogate later.
		if(lhs_resolving_stream == rhs_resolving_stream) {
			pending_filters = array_append(pending_filters, filter_op);
			continue;
		}

		// Retrieve the relevent branch roots.
		OpBase *right_branch = cp->children[rhs_resolving_stream];
		OpBase *left_branch = cp->children[lhs_resolving_stream];
		// Detach the streams and completely remove the filter from the execution plan.
		ExecutionPlan_DetachOp(right_branch);
		ExecutionPlan_DetachOp(left_branch);
		ExecutionPlan_RemoveOp(plan, (OpBase *)filter_op);
		// Combine the streams as cartesian product and filter them.
		OpBase *filtered_cp_branch = _chain_cp_and_filter(cp->plan, left_branch, right_branch,
														  (OpBase *)filter_op);

		if(cp->childCount == 0) {
			// The entire Cartesian Product can be replaced with the join op.
			ExecutionPlan_ReplaceOp(plan, cp, filtered_cp_branch);
			OpBase_Free(cp);
			pending_filters_lookup_root = filtered_cp_branch;
			break;
		} else {
			// The Cartesian Product still has a child operation; introduce the new op as another child.
			ExecutionPlan_AddOp(cp, filtered_cp_branch);
			// Streams are no longer valid since cartesian product changed.
			for(int j = 0; j < stream_count; j ++) raxFree(stream_entities[j]);
			// Re-collect cartesian product streams.
			stream_count = cp->childCount;
			build_streams_from_op(cp, stream_entities, stream_count);
		}
	}
	// Try to re-position the additional filters.
	uint pending_filters_count = array_len(pending_filters);
	// If there was a modification to the execution plan, there will be less pending filters.
	if(pending_filters_count < filter_count) {
		for(uint i = 0; i < pending_filters_count; i++) {
			OpFilter *additional_filter = pending_filters[i];
			re_order_filter_op(plan, pending_filters_lookup_root, additional_filter);
		}
	}
	// Clean up.
	for(int j = 0; j < stream_count; j ++) raxFree(stream_entities[j]);
	array_free(filter_ops);
	array_free(pending_filters);
}

void optimizeCartesianProduct(ExecutionPlan *plan) {
	OpBase **cps = ExecutionPlan_LocateOps(plan->root, OPType_CARTESIAN_PRODUCT);
	uint cp_count = array_len(cps);

	for(uint i = 0; i < cp_count ; i++) {
		OpBase *cp = cps[i];
		if(cp->childCount > 2) _optimize_cartesian_product(plan, cp);
	}
	array_free(cps);
}
