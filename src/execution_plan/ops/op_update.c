/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
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
static OpResult UpdateInit(OpBase *opBase);
static Record UpdateConsume(OpBase *opBase);
static OpResult UpdateReset(OpBase *opBase);
static OpBase *UpdateClone(const ExecutionPlan *plan, const OpBase *opBase);
static void UpdateFree(OpBase *opBase);

static Record _handoff(OpUpdate *op) {
	/* TODO: poping a record out of op->records
	 * will reverse the order in which records
	 * are passed down the execution plan. */
	if(op->records && array_len(op->records) > 0) return array_pop(op->records);
	return NULL;
}

OpBase *NewUpdateOp(const ExecutionPlan *plan, rax *update_exps) {
	OpUpdate *op = rm_calloc(1, sizeof(OpUpdate));
	op->records = NULL;
	op->updates = NULL;
	op->updates_commited = false;
	op->update_ctxs = update_exps;
	op->gc = QueryCtx_GetGraphCtx();

	// Set our Op operations.
	OpBase_Init((OpBase *)op, OPType_UPDATE, "Update", UpdateInit, UpdateConsume,
				UpdateReset, NULL, UpdateClone, UpdateFree, true, plan);

	raxIterator it;
	raxStart(&it, update_exps);
	raxSeek(&it, "^", NULL, 0);
	// Iterate over all update expressions
	while(raxNext(&it)) {
		char *alias = (char *)it.key;
		EntityUpdateEvalCtx *ctx = it.data;
		// Set the record index for every entity modified by this operation
		ctx->record_idx = OpBase_Modifies((OpBase *)op, alias);
	}
	raxStop(&it);

	return (OpBase *)op;
}

static OpResult UpdateInit(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	op->stats = QueryCtx_GetResultSetStatistics();
	op->records = array_new(Record, 64);
	op->updates = array_new(PendingUpdateCtx, raxSize(op->update_ctxs));
	return OP_OK;
}

static Record UpdateConsume(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	OpBase *child = op->op.children[0];
	Record r;

	// Updates already performed.
	if(op->updates_commited) return _handoff(op);

	while((r = OpBase_Consume(child))) {
		Record_PersistScalars(r);

		// Evaluate update expressions.
		raxIterator it;
		raxStart(&it, op->update_ctxs);
		raxSeek(&it, "^", NULL, 0);
		while(raxNext(&it)) {
			EvalEntityUpdates(op->gc, &op->updates, r, (char *)it.key, it.data, true);
		}
		raxStop(&it);

		op->records = array_append(op->records, r);
	}

	/* Done reading; we're not going to call Consume any longer.
	 * There might be operations like Index Scan that need to free the
	 * index R/W lock - as such, free all ExecutionPlan operations up the chain. */
	OpBase_PropagateFree(child);

	// Lock everything.
	QueryCtx_LockForCommit();
	CommitUpdates(op->gc, op->stats, op->updates);
	// Release lock.
	QueryCtx_UnlockCommit(opBase);

	op->updates_commited = true;
	return _handoff(op);
}

static OpBase *UpdateClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_UPDATE);
	OpUpdate *op = (OpUpdate *)opBase;

	rax *update_ctxs = raxCloneWithCallback(op->update_ctxs, (void *(*)(void *))UpdateCtx_Clone);
	return NewUpdateOp(plan, update_ctxs);
}

static OpResult UpdateReset(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;
	return OP_OK;
}

static void UpdateFree(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;

	if(op->updates) {
		// TODO adequate?
		array_free(op->updates);
		op->updates = NULL;
	}

	// Free each update context.
	if(op->update_ctxs) {
		raxFreeWithCallback(op->update_ctxs, (void(*)(void *))UpdateCtx_Free);
		op->update_ctxs = NULL;
	}

	if(op->records) {
		uint records_count = array_len(op->records);
		for(uint i = 0; i < records_count; i++) OpBase_DeleteRecord(op->records[i]);
		array_free(op->records);
		op->records = NULL;
	}
}

