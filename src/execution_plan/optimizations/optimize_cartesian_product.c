/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "optimize_cartesian_product.h"
#include "../ops/op_filter.h"
#include "../ops/op_cartesian_product.h"
#include "../../util/rax_extensions.h"
#include "../../util/qsort.h"

#define RAX_LT(a ,b) ((raxSize((a)->entities)) < (raxSize((b)->entities)))

// This struct is an auxilary struct for sorting filters according to their referenced entities count.
typedef struct {
	OpFilter *filter;   // Filter operation
	rax *entities;      // Contains the entities that the filter references.
} Filter_And_Entities;

// Collects all consecutive filters beneath given op, sort them by the number of referenced entities.
static Filter_And_Entities *_locate_filters_and_entities(OpBase *cp) {
	OpBase *parent = cp->parent;
	Filter_And_Entities *filters = array_new(Filter_And_Entities, 0);

	while(parent && parent->type == OPType_FILTER) {
		Filter_And_Entities fae;
		OpFilter *filter_op = (OpFilter *)parent;
		fae.filter = filter_op;
		// Collect referenced entities.
		fae.entities = FilterTree_CollectModified(filter_op->filterTree);
		filters = array_append(filters, fae);
		parent = parent->parent;
	}
	// Sort by the number of referenced entities.
	QSORT(Filter_And_Entities, filters, array_len(filters), RAX_LT);
	return filters;
}

// Finds all the cartesian product's children which solve a specific filter entities.
static OpBase **_find_entities_solving_branches(rax *entities, OpBase *cp) {
	int entities_count = raxSize(entities);
	OpBase **solving_branches = array_new(OpBase *, 1);
	for(int i = 0; i < cp->childCount; i++) {
		OpBase *branch = cp->children[i];
		/* Locate references reduces the amount of entities upon each call
		 * that partially solves the references. */
		ExecutionPlan_LocateReferences(branch, NULL, entities);
		int new_entities_count = raxSize(entities);
		if(new_entities_count != entities_count) {
			// Update entity count.
			entities_count = new_entities_count;
			// Add partially solving branch into the array.
			solving_branches = array_append(solving_branches, branch);
			// No need to continue from here.
			if(entities_count == 0) break;
		}
	}
	assert(entities_count == 0);
	assert(array_len(solving_branches) > 0);
	// The rax containing the entities is now emtpy, free it.
	raxFree(entities);
	return solving_branches;
}

static void _optimize_cartesian_product(ExecutionPlan *plan, OpBase *cp) {
	// Retrieve all filter operations located upstream from the Cartesian Product.
	Filter_And_Entities *filters = _locate_filters_and_entities(cp);
	uint filter_count = array_len(filters);

	for(uint i = 0; i < filter_count; i ++) {
		// Try to create a cartesian product, followed by the current filter.
		OpFilter *filter_op = filters[i].filter;
		ExecutionPlan_RemoveOp(plan, (OpBase *)filter_op);

		OpBase **solving_branches = _find_entities_solving_branches(filters[i].entities, cp);
		// This filter is solved by a single cartesian product child and needs to be propagated up.
		if(array_len(solving_branches) == 1) {
			ExecutionPlan_PushBelow(solving_branches[0], (OpBase *)filter_op);
			array_free(solving_branches);
			continue;
		}

		// Need to create a new cartesian product and connect the solving branches to the filter.
		OpBase *new_cp = NewCartesianProductOp(cp->plan);
		ExecutionPlan_AddOp((OpBase *)filter_op, new_cp);
		uint solving_branch_count = array_len(solving_branches);
		// Detach each solving branch from the original cp, and attach them as children for the new cp.
		for(uint j = 0; j < solving_branch_count; j++) {
			OpBase *solving_branch = solving_branches[j];
			ExecutionPlan_DetachOp(solving_branch);
			ExecutionPlan_AddOp(new_cp, solving_branch);
		}

		if(cp->childCount == 0) {
			// The entire Cartesian Product can be removed.
			ExecutionPlan_ReplaceOp(plan, cp, (OpBase *)filter_op);
			OpBase_Free(cp);
			/* The optimization has depleted all of the cartesian product children, merged them and replaced the
			 * cartesian product with the filter op.
			 * Since the original cartesian product is no longer a valid operation, and there might be
			 * additional filters which are applicable to re position after the optimization is done,
			 * the following code tries to propagate up the remaining filters, and finish the loop. */
			i++;
			for(; i < filter_count; i++) {
				ExecutionPlan_RePositionFilterOp(plan, (OpBase *)filter_op, NULL, (OpBase *)filters[i].filter);
			}
		} else {
			// The Cartesian Product still has a child operation; introduce the new op as another child.
			ExecutionPlan_AddOp(cp, (OpBase *)filter_op);
		}
	}
	// Clean up.
	array_free(filters);
}

void reduceCartesianProductStreamCount(ExecutionPlan *plan) {
	OpBase **cps = ExecutionPlan_LocateOps(plan->root, OPType_CARTESIAN_PRODUCT);
	uint cp_count = array_len(cps);

	for(uint i = 0; i < cp_count ; i++) {
		OpBase *cp = cps[i];
		if(cp->childCount > 2) _optimize_cartesian_product(plan, cp);
	}
	array_free(cps);
}
