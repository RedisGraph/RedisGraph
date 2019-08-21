/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_filter.h"

OpBase *NewFilterOp(FT_FilterNode *filterTree) {
	Filter *filter = malloc(sizeof(Filter));
	filter->filterTree = filterTree;

	// Set our Op operations
	OpBase_Init(&filter->op);
	filter->op.name = "Filter";
	filter->op.type = OPType_FILTER;
	filter->op.consume = FilterConsume;
	filter->op.reset = FilterReset;
	filter->op.free = FilterFree;

	return (OpBase *)filter;
}

/* FilterConsume next operation
 * returns OP_OK when graph passes filter tree. */
Record FilterConsume(OpBase *opBase) {
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
OpResult FilterReset(OpBase *ctx) {
	return OP_OK;
}

/* Frees Filter*/
void FilterFree(OpBase *ctx) {
	Filter *filter = (Filter *)ctx;
	if(filter->filterTree) {
		FilterTree_Free(filter->filterTree);
		filter->filterTree = NULL;
	}
}
