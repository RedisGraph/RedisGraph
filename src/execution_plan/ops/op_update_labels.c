#include "op_update_labels.h"
#include "../../query_ctx.h"
#include "../../util/rax_extensions.h"

/* Forward declarations. */
static OpResult UpdateLabelsInit(OpBase *opBase);
static Record UpdateLabelsConsume(OpBase *opBase);
static OpResult UpdateLabelsReset(OpBase *opBase);
static OpBase *UpdateLabelsClone(const ExecutionPlan *plan, const OpBase *opBase);
static void UpdateLabelsFree(OpBase *opBase);

static Record _handoff(OpUpdateLabels *op) {
	/* TODO: poping a record out of op->records
	 * will reverse the order in which records
	 * are passed down the execution plan. */
	if(op->records && array_len(op->records) > 0) return array_pop(op->records);
	return NULL;
}

OpBase *NewUpdateLabelsOp(const ExecutionPlan *plan, rax *update_exps) {
	OpUpdateLabels *op = rm_calloc(1, sizeof(OpUpdateLabels));
	op->records            =  NULL;
	op->node_updates       =  NULL;
	op->updates_committed  =  false;
	op->update_ctxs        =  update_exps;
	op->gc                 =  QueryCtx_GetGraphCtx();

	// set our op operations
	OpBase_Init((OpBase *)op, OpType_SET_LABELS, "Node Add Labels", UpdateLabelsInit, UpdateLabelsConsume,
				UpdateLabelsReset, NULL, UpdateLabelsClone, UpdateLabelsFree, true, plan);

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

static OpResult UpdateLabelsInit(OpBase *opBase) {
	OpUpdateLabels *op = (OpUpdateLabels *)opBase;

	op->stats         =  QueryCtx_GetResultSetStatistics();
	op->records       =  array_new(Record, 64);
	op->node_updates  =  array_new(PendingUpdateCtx, raxSize(op->update_ctxs));

	return OP_OK;
}


static Record UpdateLabelsConsume(OpBase *opBase) {
	OpUpdateLabels *op = (OpUpdateLabels *)opBase;
	OpBase *child = op->op.children[0];
	Record r;

	// updates already performed
	if(op->updates_committed) return _handoff(op);

	while((r = OpBase_Consume(child))) {
		// evaluate update expressions
		raxSeek(&op->it, "^", NULL, 0);
		while(raxNext(&op->it)) {
			EntityUpdateEvalCtx *ctx = op->it.data;
			EvalEntityUpdates(op->gc, &op->node_updates, NULL, r, ctx, true);
		}

		array_append(op->records, r);
	}

	// done reading; we're not going to call Consume any longer
	// there might be operations like "Index Scan" that need to free the
	// index R/W lock - as such, free all ExecutionPlan operations up the chain.
	OpBase_PropagateFree(child);

	// lock everything
	QueryCtx_LockForCommit();
	{
        
		CommitLabelUpdates(op->gc, op->stats, op->node_updates);
	}
	// release lock
	QueryCtx_UnlockCommit(opBase);

	uint node_updates_count = array_len(op->node_updates);

	array_clear(op->node_updates);

	op->updates_committed = true;

	return _handoff(op);
}

static OpBase *UpdateLabelsClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OpType_SET_LABELS);
	OpUpdateLabels *op = (OpUpdateLabels *)opBase;

	rax *update_ctxs = raxCloneWithCallback(op->update_ctxs, (void *(*)(void *))UpdateCtx_Clone);
	return NewUpdateLabelsOp(plan, update_ctxs);
}

static OpResult UpdateLabelsReset(OpBase *ctx) {
	OpUpdateLabels *op = (OpUpdateLabels *)ctx;

	uint node_updates_count = array_len(op->node_updates);
	for(uint i = 0; i < node_updates_count; i ++) {
		PendingUpdateCtx *pending_update = op->node_updates + i;
		AttributeSet_Free(&pending_update->attributes);
	}
	array_free(op->node_updates);
	op->node_updates = NULL;

	op->updates_committed = false;
	return OP_OK;
}


static void UpdateLabelsFree(OpBase *ctx) {
	OpUpdateLabels *op = (OpUpdateLabels *)ctx;

    uint node_updates_count = array_len(op->node_updates);
    for(uint i = 0; i < node_updates_count; i ++) {
        PendingUpdateCtx *pending_update = op->node_updates + i;
        // remove labels?
    }
    array_free(op->node_updates);
    op->node_updates = NULL;

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

