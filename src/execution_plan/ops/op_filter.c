/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_filter.h"

/* Forward declarations */
static void Free(OpBase *ctx);
static OpResult Reset(OpBase *ctx);
static Record Consume(OpBase *opBase);

OpBase *NewFilterOp(const ExecutionPlan *plan, FT_FilterNode *filterTree) {
	Filter *op = malloc(sizeof(Filter));
	op->filterTree = filterTree;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_FILTER, "Filter", NULL, Consume, Reset, NULL, Free, plan);

	return (OpBase *)op;
}

/* FilterConsume next operation
 * returns OP_OK when graph passes filter tree. */
static Record Consume(OpBase *opBase) {
	Record r;
	Filter *filter = (Filter *)opBase;
	OpBase *child = filter->op.children[0];

	while(true) {
		r = OpBase_Consume(child);
		if(!r) break;

		/* Pass graph through filter tree */
		if(FilterTree_applyFilters(filter->filterTree, r) == FILTER_PASS) break;
		else Record_Free(r);
	}

	return r;
}

/* Restart iterator */
static OpResult Reset(OpBase *ctx) {
	return OP_OK;
}

/* Frees Filter*/
static void Free(OpBase *ctx) {
	Filter *filter = (Filter *)ctx;
	if(filter->filterTree) {
		FilterTree_Free(filter->filterTree);
		filter->filterTree = NULL;
	}
}
