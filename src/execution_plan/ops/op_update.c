/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op_update.h"
#include "RG.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../util/rax_extensions.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record UpdateConsume(OpBase *opBase);
static OpResult UpdateReset(OpBase *opBase);
static OpBase *UpdateClone(const ExecutionPlan *plan, const OpBase *opBase);
static void UpdateFree(OpBase *opBase);

static Record _handoff(OpUpdate *op) {
	/* TODO: popping a record out of op->records
	 * will reverse the order in which records
	 * are passed down the execution plan. */
	if(op->records && array_len(op->records) > 0) return array_pop(op->records);
	return NULL;
}

OpBase *NewUpdateOp(const ExecutionPlan *plan, rax *update_exps) {
	OpUpdate *op = rm_calloc(1, sizeof(OpUpdate));
	op->gc                 =  QueryCtx_GetGraphCtx();
	op->stats              =  QueryCtx_GetResultSetStatistics();
	op->records            =  array_new(Record, 64);
	op->update_ctxs        =  update_exps;
	op->node_updates       =  HashTableCreate(NULL);
	op->edge_updates       =  HashTableCreate(NULL);
	op->updates_committed  =  false;

	// set our op operations
	OpBase_Init((OpBase *)op, OPType_UPDATE, "Update", NULL, UpdateConsume,
				UpdateReset, NULL, UpdateClone, UpdateFree, true, plan);

	// iterate over all update expressions
	// set the record index for every entity modified by this operation
	raxStart(&op->it, update_exps);
	raxSeek(&op->it, "^", NULL, 0);
	while(raxNext(&op->it)) {
		EntityUpdateEvalCtx *ctx = op->it.data;
		ctx->record_idx = OpBase_Modifies((OpBase *)op, ctx->alias);
	}

	return (OpBase *)op;
}

static Record UpdateConsume(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	OpBase *child = op->op.children[0];
	Record r;

	// updates already performed
	if(op->updates_committed) return _handoff(op);

	while((r = OpBase_Consume(child))) {
		Record_PersistScalars(r);

		// evaluate update expressions
		raxSeek(&op->it, "^", NULL, 0);
		while(raxNext(&op->it)) {
			EntityUpdateEvalCtx *ctx = op->it.data;
			EvalEntityUpdates(op->gc, op->node_updates, op->edge_updates, r, ctx, true);
		}

		array_append(op->records, r);
	}
	
	uint node_updates_count = HashTableElemCount(op->node_updates);
	uint edge_updates_count = HashTableElemCount(op->edge_updates);

	if(node_updates_count > 0 || edge_updates_count > 0) {
		// done reading; we're not going to call Consume any longer
		// there might be operations like "Index Scan" that need to free the
		// index R/W lock - as such, free all ExecutionPlan operations up the chain.
		OpBase_PropagateReset(child);

		// lock everything
		QueryCtx_LockForCommit();

		CommitUpdates(op->gc, op->stats, op->node_updates, ENTITY_NODE);
		CommitUpdates(op->gc, op->stats, op->edge_updates, ENTITY_EDGE);
	}

	dictIterator *it = HashTableGetIterator(op->node_updates);
	dictEntry *entry;
	while((entry = HashTableNext(it)) != NULL) {
		PendingUpdateCtx *pending_update = HashTableGetVal(entry);
		PendingUpdateCtx_Free(pending_update);
	}
	HashTableReleaseIterator(it);
	HashTableEmpty(op->node_updates, NULL);

	it = HashTableGetIterator(op->edge_updates);
	while((entry = HashTableNext(it)) != NULL) {
		PendingUpdateCtx *pending_update = HashTableGetVal(entry);
		PendingUpdateCtx_Free(pending_update);
	}
	HashTableReleaseIterator(it);
	HashTableEmpty(op->edge_updates, NULL);

	op->updates_committed = true;

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

	dictIterator *it = HashTableGetIterator(op->node_updates);
	dictEntry *entry;
	while((entry = HashTableNext(it)) != NULL) {
		PendingUpdateCtx *pending_update = HashTableGetVal(entry);
		PendingUpdateCtx_Free(pending_update);
	}
	HashTableReleaseIterator(it);
	HashTableEmpty(op->node_updates, NULL);

	it = HashTableGetIterator(op->edge_updates);
	while((entry = HashTableNext(it)) != NULL) {
		PendingUpdateCtx *pending_update = HashTableGetVal(entry);
		PendingUpdateCtx_Free(pending_update);;
	}
	HashTableReleaseIterator(it);
	HashTableEmpty(op->edge_updates, NULL);

	op->updates_committed = false;
	return OP_OK;
}

static void UpdateFree(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;

	if(op->node_updates) {
		dictIterator *it = HashTableGetIterator(op->node_updates);
		dictEntry *entry;
		while((entry  = HashTableNext(it)) != NULL) {
			PendingUpdateCtx *pending_update = HashTableGetVal(entry);
			PendingUpdateCtx_Free(pending_update);
		}
		HashTableReleaseIterator(it);
		HashTableRelease(op->node_updates);
		op->node_updates = NULL;
	}

	if(op->edge_updates) {
		dictIterator *it = HashTableGetIterator(op->edge_updates);
		dictEntry *entry;
		while((entry  = HashTableNext(it)) != NULL) {
			PendingUpdateCtx *pending_update = HashTableGetVal(entry);
			PendingUpdateCtx_Free(pending_update);
		}
		HashTableReleaseIterator(it);
		HashTableRelease(op->edge_updates);
		op->edge_updates = NULL;
	}

	// free each update context
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

	raxStop(&op->it);
}
