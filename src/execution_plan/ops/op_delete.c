/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static void DeleteFree(OpBase *opBase);

OpBase *NewDeleteOp(const ExecutionPlan *plan, AR_ExpNode **exps) {
	OpDelete *op = rm_malloc(sizeof(OpDelete));

	op->exps = exps;
	op->exp_count = array_len(exps);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_DELETE, "Delete", NULL, DeleteFree, true,
		plan);

	return (OpBase *)op;
}

static void DeleteFree(OpBase *ctx) {
	OpDelete *op = (OpDelete *)ctx;

	if(op->exps) {
		for(int i = 0; i < op->exp_count; i++) AR_EXP_Free(op->exps[i]);
		array_free(op->exps);
		op->exps = NULL;
	}
}

