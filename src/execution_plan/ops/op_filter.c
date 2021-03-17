/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_filter.h"
#include "RG.h"

/* Forward declarations. */
static Record FilterConsume(OpBase *opBase);
static OpBase *FilterClone(const ExecutionPlan *plan, const OpBase *opBase);
static void FilterFree(OpBase *opBase);

OpBase *NewFilterOp(const ExecutionPlan *plan, FT_FilterNode *filterTree) {
	OpFilter *op = rm_malloc(sizeof(OpFilter));
	op->filterTree = filterTree;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_FILTER, "Filter", NULL, FilterConsume,
				NULL, NULL, FilterClone, FilterFree, false, plan);

	return (OpBase *)op;
}

/* FilterConsume next operation
 * returns OP_OK when graph passes filter tree. */
static Record FilterConsume(OpBase *opBase) {
	Record r = NULL;
	OpFilter *filter = (OpFilter *)opBase;
	OpBase *child = filter->op.children[0];
	FT_FilterNode *filterTree = filter->filterTree;

	OP_BATCH_CLEAR();

	while(OP_BATCH_HAS_ROOM()) {
		RecordBatch batch = OpBase_Consume(child);
		uint record_count = RecordBatch_Len(batch);
		if(record_count == 0) break; // depleted

		for(uint i = 0; i < record_count; i++) {
			Record r = batch[i];
			// pass graph through filter tree
			bool pass = (FilterTree_applyFilters(filterTree, r) == FILTER_PASS); 
			if(pass) OP_BATCH_ADD(r);
			else OpBase_DeleteRecord(r);
		}
	}

	OP_BATCH_EMIT();
}

static inline OpBase *FilterClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_FILTER);
	OpFilter *op = (OpFilter *)opBase;
	return NewFilterOp(plan, FilterTree_Clone(op->filterTree));
}

/* Frees OpFilter*/
static void FilterFree(OpBase *ctx) {
	OpFilter *filter = (OpFilter *)ctx;
	if(filter->filterTree) {
		FilterTree_Free(filter->filterTree);
		filter->filterTree = NULL;
	}
}

