/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "optimize_cartesian_product.h"
#include "RG.h"
#include "../../errors.h"
#include "../ops/op_filter.h"
#include "../ops/op_cartesian_product.h"
#include "../../util/rax_extensions.h"
#include "../../util/qsort.h"
#include "../execution_plan_build/execution_plan_modify.h"
#include "../execution_plan_build/execution_plan_construct.h"

#define FilterCtx_LT(a ,b) ((raxSize((a)->entities)) < (raxSize((b)->entities)))

// This struct is an auxilary struct for sorting filters according to their referenced entities count.
typedef struct {
	OpFilter *filter;   // Filter operation
	rax *entities;      // Contains the entities that the filter references.
} FilterCtx;

// Free FilterCtx.
static inline void _FilterCtx_Free(FilterCtx *ctx) {
	raxFree(ctx->entities);
}

/* Collects all consecutive filters beneath given op, sort them by the number of referenced entities.
 * The array is soreted in order to reposition the filter that require smaller cartiesian products first. */
static FilterCtx *_locate_filters_and_entities(OpBase *cp) {
	OpBase *parent = cp->parent;
	FilterCtx *filter_ctx_arr = array_new(FilterCtx, 0);

	while(parent && parent->type == OPType_FILTER) {
		OpFilter *filter_op = (OpFilter *)parent;
		FilterCtx filter_ctx;
		filter_ctx.filter = filter_op;
		// Collect referenced entities.
		filter_ctx.entities = FilterTree_CollectModified(filter_op->filterTree);
		filter_ctx_arr = array_append(filter_ctx_arr, filter_ctx);
		parent = parent->parent;
	}
	// Sort by the number of referenced entities.
	QSORT(FilterCtx, filter_ctx_arr, array_len(filter_ctx_arr), FilterCtx_LT);
	return filter_ctx_arr;
}

// Finds all the cartesian product's children which solve a specific filter entities.
static OpBase **_find_entities_solving_branches(rax *entities, OpBase *cp) {
	int entities_count = raxSize(entities);
	if(entities_count == 0) return NULL; // No dependencies in filters.
	OpBase **solving_branches = array_new(OpBase *, 1);
	// Iterate over all the children or until all the entities are resolved.
	for(int i = 0; i < cp->childCount && entities_count > 0; i++) {
		OpBase *branch = cp->children[i];
		// Don't recurse into previous scopes when trying to resolve references.
		OpBase *recurse_limit = ExecutionPlan_LocateOpMatchingType(branch, PROJECT_OPS, PROJECT_OP_COUNT);
		/* Locate references reduces the amount of entities upon each call
		 * that partially solves the references. */
		ExecutionPlan_LocateReferences(branch, recurse_limit, entities);
		int new_entities_count = raxSize(entities);
		if(new_entities_count != entities_count) {
			// Update entity count.
			entities_count = new_entities_count;
			// Add partially solving branch into the array.
			solving_branches = array_append(solving_branches, branch);
		}
	}
	if(entities_count != 0) {
		Error_InvalidFilterPlacement(entities);
		array_free(solving_branches);
		return NULL;
	}
	return solving_branches;
}

static void _optimize_cartesian_product(ExecutionPlan *plan, OpBase *cp) {
	// Retrieve all filter operations located upstream from the Cartesian Product.
	FilterCtx *filter_ctx_arr = _locate_filters_and_entities(cp);
	uint filter_count = array_len(filter_ctx_arr);

	for(uint i = 0; i < filter_count; i ++) {
		// Try to create a cartesian product, followed by the current filter.
		OpFilter *filter_op = filter_ctx_arr[i].filter;
		OpBase **solving_branches = _find_entities_solving_branches(filter_ctx_arr[i].entities, cp);
		if(solving_branches == NULL) {
			// Filter placement failed, return early.
			array_free(filter_ctx_arr);
			return;
		}

		uint solving_branch_count = array_len(solving_branches);
		// In case this filter is solved by the entire cartesian product, it does not need to be repositioned.
		if(solving_branch_count == cp->childCount) {
			array_free(solving_branches);
			continue;
		}

		// The filter needs to be repositioned.
		ExecutionPlan_RemoveOp(plan, (OpBase *)filter_op);
		// This filter is solved by a single cartesian product child and needs to be propagated up.
		if(solving_branch_count == 1) {
			OpBase *solving_op = solving_branches[0];
			/* Single branch solving a filter that was after a cartesian product.
			 * The filter may be pushed directly onto the appropriate branch. */
			ExecutionPlan_PushBelow(solving_op, (OpBase *)filter_op);
			array_free(solving_branches);
			continue;
		}

		// Need to create a new cartesian product and connect the solving branches to the filter.
		OpBase *new_cp = NewCartesianProductOp(cp->plan);
		ExecutionPlan_AddOp((OpBase *)filter_op, new_cp);
		// Detach each solving branch from the original cp, and attach them as children for the new cp.
		for(uint j = 0; j < solving_branch_count; j++) {
			OpBase *solving_branch = solving_branches[j];
			ExecutionPlan_DetachOp(solving_branch);
			ExecutionPlan_AddOp(new_cp, solving_branch);
		}
		array_free(solving_branches);

		if(cp->childCount == 0) {
			// The entire Cartesian Product can be removed.
			ExecutionPlan_ReplaceOp(plan, cp, (OpBase *)filter_op);
			OpBase_Free(cp);
			/* The optimization has depleted all of the cartesian product children, merged them and replaced the
			 * cartesian product with the filter op.
			 * Since the original cartesian product is no longer a valid operation, and there might be
			 * additional filters which are applicable to reposition after the optimization is done,
			 * the following code tries to propagate up the remaining filters, and finish the loop. */
			i++;
			for(; i < filter_count; i++) {
				ExecutionPlan_RePositionFilterOp(plan, (OpBase *)filter_op, NULL,
												 (OpBase *)filter_ctx_arr[i].filter);
			}
		} else {
			// The Cartesian Product still has a child operation; introduce the new op as another child.
			ExecutionPlan_AddOp(cp, (OpBase *)filter_op);
		}
	}
	// Clean up.
	for(uint i = 0; i < filter_count; i++) _FilterCtx_Free(filter_ctx_arr + i);
	array_free(filter_ctx_arr);
}

void reduceCartesianProductStreamCount(ExecutionPlan *plan) {
	OpBase **cps = ExecutionPlan_CollectOps(plan->root, OPType_CARTESIAN_PRODUCT);
	uint cp_count = array_len(cps);

	for(uint i = 0; i < cp_count ; i++) {
		OpBase *cp = cps[i];
		if(cp->childCount > 2) _optimize_cartesian_product(plan, cp);
	}
	array_free(cps);
}

