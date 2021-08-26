/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_sort.h"
#include "op_project.h"
#include "op_aggregate.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include "../../util/rmalloc.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static void SortFree(OpBase *opBase);

OpBase *NewSortOp(const ExecutionPlan *plan, AR_ExpNode **exps, int *directions) {
	OpSort *op = rm_malloc(sizeof(OpSort));
	op->skip = 0;
	op->limit = UNLIMITED;
	op->directions = directions;
	op->exps = exps;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_SORT, "Sort", NULL, SortFree, false, plan);

	return (OpBase *)op;
}

/* Frees Sort */
static void SortFree(OpBase *ctx) {
	OpSort *op = (OpSort *)ctx;
	if(op->directions) {
		array_free(op->directions);
		op->directions = NULL;
	}

	if(op->exps) {
		uint exps_count = array_len(op->exps);
		for(uint i = 0; i < exps_count; i++) AR_EXP_Free(op->exps[i]);
		array_free(op->exps);
		op->exps = NULL;
	}
}
