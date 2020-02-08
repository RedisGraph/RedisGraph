/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "optimize_cartesian_product.h"
#include "../ops/op_filter.h"
#include "../ops/op_cartesian_product.h"
#include "optimizations_util.h"

// Collects all consecutive filters beneath given op.
static OpFilter **_locate_filters(OpBase *cp) {
	OpBase *parent = cp->parent;
	OpFilter **filters = array_new(OpFilter *, 0);

	while(parent && parent->type == OPType_FILTER) {
		OpFilter *filter_op = (OpFilter *)parent;
		filters = array_append(filters, filter_op);
		parent = parent->parent;
	}

	return filters;
}

/**
 * @brief  Creates a structure of dual branched catesian product, followed by a filter.
 *         The generated record from the cartesian product completely satisfies the filter entities.
 * @param  *plan: Execution plan to associate its mapping with the new CP operation.
 * @param  *lhs: Left side branch.
 * @param  *rhs: Right side branch.
 * @param  *filter: Trailing filter operation.
 * @retval Pointer to the filter operation.
 */
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

	// For each stream joined by the Cartesian product, collect all entities the stream resolves.
	int stream_count = cp->childCount;
	rax *stream_entities[stream_count];
	OptimizeUtils_BuildStreamFromOp(cp, stream_entities, stream_count);

	for(uint i = 0; i < filter_count; i ++) {
		// Try to create a new dual-branched cartesian product, followed by the current filter.
		OpFilter *filter_op = filter_ops[i];

		/* A new cartesian product can be generated if both sides of the filter can be fully and
		 * separately resolved by exactly two child streams. */
		FT_FilterNode *f = filter_op->filterTree;

		/* Make sure LHS of the filter is resolved by a stream. */
		AR_ExpNode *lhs = f->pred.lhs;
		uint lhs_resolving_stream = OptimizeUtils_RelateExpToStream(lhs, stream_entities, stream_count);
		if(lhs_resolving_stream == NOT_RESOLVED) continue;
		/* Make sure RHS of the filter is resolved by a stream. */
		AR_ExpNode *rhs = f->pred.rhs;
		uint rhs_resolving_stream = OptimizeUtils_RelateExpToStream(rhs, stream_entities, stream_count);
		if(rhs_resolving_stream == NOT_RESOLVED) continue;

		// This stream is solved by a single cartesian product child and needs to be propagated up.
		if(lhs_resolving_stream == rhs_resolving_stream) {
			OptimizeUtils_MigrateFilterOp(plan, cp->children[rhs_resolving_stream], filter_op);
			continue;
		}

		// Retrieve the relevant branch roots.
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
			// The entire Cartesian Product can be replaced with the new branch.
			ExecutionPlan_ReplaceOp(plan, cp, filtered_cp_branch);
			OpBase_Free(cp);
			/* The optimization has depleted all of the cartesian product children, merged them and replaced the
			 * cartesian product with the new operation.
			 * Since the original cartesian product is no longer a valid operation, and there might be
			 * additional filters which are applicable to re position after the optimization is done,
			 * the following code tries to propagate up the remaining filters, and finish the loop. */
			i++;
			for(; i < filter_count; i++) {
				OptimizeUtils_MigrateFilterOp(plan, filtered_cp_branch, filter_ops[i]);
			}
		} else {
			// The Cartesian Product still has a child operation; introduce the new op as another child.
			ExecutionPlan_AddOp(cp, filtered_cp_branch);
			// If there are remaining filters, re-collect cartesian product streams.
			if(i + 1 < filter_count) {
				// Streams are no longer valid since cartesian product changed.
				for(int j = 0; j < stream_count; j ++) raxFree(stream_entities[j]);
				stream_count = cp->childCount;
				OptimizeUtils_BuildStreamFromOp(cp, stream_entities, stream_count);
			}
		}
	}
	// Clean up.
	for(int j = 0; j < stream_count; j ++) raxFree(stream_entities[j]);
	array_free(filter_ops);
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
