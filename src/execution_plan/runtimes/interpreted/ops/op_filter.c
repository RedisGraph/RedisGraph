/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_filter.h"
#include "RG.h"

/* Forward declarations. */
static Record FilterConsume(RT_OpBase *opBase);
static void FilterFree(RT_OpBase *ctx);

RT_OpBase *RT_NewFilterOp(const RT_ExecutionPlan *plan, const OpFilter *op_desc) {
	RT_OpFilter *op = rm_malloc(sizeof(RT_OpFilter));
	op->op_desc = op_desc;
	op->filterTree = FilterTree_Clone(op_desc->filterTree);

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, NULL, NULL,
		FilterConsume, NULL, FilterFree, plan);

	return (RT_OpBase *)op;
}

/* FilterConsume next operation
 * returns OP_OK when graph passes filter tree. */
static Record FilterConsume(RT_OpBase *opBase) {
	Record r = NULL;
	RT_OpFilter *filter = (RT_OpFilter *)opBase;
	RT_OpBase *child = filter->op.children[0];

	while(true) {
		r = RT_OpBase_Consume(child);
		if(!r) break;

		/* Pass record through filter tree */
		if(FilterTree_applyFilters(filter->filterTree, r) == FILTER_PASS) break;
		else RT_OpBase_DeleteRecord(r);
	}

	return r;
}

static void FilterFree(RT_OpBase *ctx) {
	RT_OpFilter *filter = (RT_OpFilter *)ctx;
	if(filter->filterTree) {
		FilterTree_Free(filter->filterTree);
		filter->filterTree = NULL;
	}
}
