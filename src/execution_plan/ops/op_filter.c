/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_filter.h"

/* Forward declarations. */
static Record FilterConsume(OpBase *opBase);
static void FilterFree(OpBase *opBase);

OpBase *NewFilterOp(const ExecutionPlan *plan, FT_FilterNode *filterTree) {
	OpFilter *op = rm_malloc(sizeof(OpFilter));
	op->filterTree = filterTree;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_FILTER, "Filter", NULL, FilterConsume,
				NULL, NULL, FilterFree, false, plan);

	return (OpBase *)op;
}

/* FilterConsume next operation
 * returns OP_OK when graph passes filter tree. */
static Record FilterConsume(OpBase *opBase) {
	Record r;
	OpFilter *filter = (OpFilter *)opBase;
	OpBase *child = filter->op.children[0];

	while(true) {
		r = OpBase_Consume(child);
		if(!r) break;

		/* Pass graph through filter tree */
		if(FilterTree_applyFilters(filter->filterTree, r) == FILTER_PASS) break;
		else OpBase_DeleteRecord(&r);
	}

	return r;
}

/* Frees OpFilter*/
static void FilterFree(OpBase *ctx) {
	OpFilter *filter = (OpFilter *)ctx;
	if(filter->filterTree) {
		FilterTree_Free(filter->filterTree);
		filter->filterTree = NULL;
	}
}

