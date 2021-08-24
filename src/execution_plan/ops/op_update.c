/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_update.h"
#include "RG.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include "../../util/rmalloc.h"
#include "../../util/rax_extensions.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static void UpdateFree(OpBase *opBase);

OpBase *NewUpdateOp(const ExecutionPlan *plan, rax *update_exps) {
	OpUpdate *op     = rm_calloc(1, sizeof(OpUpdate));
	op->update_ctxs  =  update_exps;

	// set our op operations
	OpBase_Init((OpBase *)op, OPType_UPDATE, "Update", NULL, UpdateFree, true,
		plan);

	raxIterator it;
	// iterate over all update expressions
	// set the record index for every entity modified by this operation
	raxStart(&it, update_exps);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		EntityUpdateEvalCtx *ctx = it.data;
		OpBase_Modifies((OpBase *)op, ctx->alias);
	}
	raxStop(&it);

	return (OpBase *)op;
}

static void UpdateFree(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;

	// Free each update context.
	if(op->update_ctxs) {
		raxFreeWithCallback(op->update_ctxs, (void(*)(void *))UpdateCtx_Free);
		op->update_ctxs = NULL;
	}
}
