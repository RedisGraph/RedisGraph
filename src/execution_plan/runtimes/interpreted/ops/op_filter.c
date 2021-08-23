/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_filter.h"
#include "RG.h"

/* Forward declarations. */
static Record FilterConsume(RT_OpBase *opBase);
static RT_OpBase *FilterClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);
static void FilterFree(RT_OpBase *opBase);

RT_OpBase *RT_NewFilterOp(const RT_ExecutionPlan *plan, FT_FilterNode *filterTree) {
	RT_OpFilter *op = rm_malloc(sizeof(RT_OpFilter));
	op->filterTree = filterTree;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_FILTER, NULL, FilterConsume,
				NULL, FilterClone, FilterFree, false, plan);

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

static inline RT_OpBase *FilterClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_FILTER);
	RT_OpFilter *op = (RT_OpFilter *)opBase;
	return RT_NewFilterOp(plan, FilterTree_Clone(op->filterTree));
}

/* Frees OpFilter*/
static void FilterFree(RT_OpBase *ctx) {
	RT_OpFilter *filter = (RT_OpFilter *)ctx;
	if(filter->filterTree) {
		FilterTree_Free(filter->filterTree);
		filter->filterTree = NULL;
	}
}
