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

/* Set a property on a graph entity. For non-NULL values, the property
 * will be added or updated if it is already present.
 * For NULL values, the property will be deleted if present
 * and nothing will be done otherwise.
 * Returns 1 if a property was set or deleted.  */
static int _UpdateEntity(PendingUpdateCtx *update) {
	int           res        =  0;
	GraphEntity   *ge        =  update->ge;
	Attribute_ID  attr_id    =  update->attr_id;
	SIValue       new_value  =  update->new_value;

	// If this entity has been deleted, perform no updates and return early.
	if(GraphEntity_IsDeleted(ge)) goto cleanup;

	// Handle the case in which we are deleting all properties.
	if(attr_id == ATTRIBUTE_ALL) return GraphEntity_ClearProperties(ge);

	// Try to get current property value.
	SIValue *old_value = GraphEntity_GetProperty(ge, attr_id);

	if(old_value == PROPERTY_NOTFOUND) {
		// Adding a new property; do nothing if its value is NULL.
		if(SI_TYPE(new_value) != T_NULL) {
			res = GraphEntity_AddProperty(ge, attr_id, new_value);
		}
	} else {
		// Update property.
		res = GraphEntity_SetProperty(ge, attr_id, new_value);
	}

cleanup:
	SIValue_Free(new_value);
	return res;
}

// Commits delayed updates.
static void _CommitUpdates(OpUpdate *op) {
	uint update_count = array_len(op->updates);
	uint properties_set = 0;
	for(uint i = 0; i < update_count; i++) {
		PendingUpdateCtx *update = op->updates + i;
		// Update the property on the graph entity.
		properties_set += _UpdateEntity(update);

		// Update index for node entities if indexed fields have been modified.
		if(update->update_index) {
			Schema *s = GraphContext_GetSchemaByID(op->gc, update->label_id, SCHEMA_NODE);
			// Introduce updated entity to index.
			Schema_AddNodeToIndices(s, (Node *)update->ge);
		}
	}

	if(op->stats) op->stats->properties_set += properties_set;
}

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
		EntityUpdateEvalCtx_NEW *ctx = it.data;
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

static void _EvalEntityUpdates(OpUpdate *op, Record r, char *alias,
							   EntityUpdateEvalCtx_NEW *ctx) {
	Schema *s         = NULL;
	int label_id      = GRAPH_NO_LABEL;
	const char *label = NULL;
	bool node_update  = false;

	// Get the type of the entity to update. If the expected entity was not
	// found, make no updates but do not error.
	RecordEntryType t = Record_GetType(r, ctx->record_idx);
	if(t == REC_TYPE_UNKNOWN) return;

	// Make sure we're updating either a node or an edge.
	if(t != REC_TYPE_NODE && t != REC_TYPE_EDGE) {
		ErrorCtx_RaiseRuntimeException("Update error: alias '%s' did not resolve to a graph entity",
									   alias);
	}

	GraphEntity *entity = Record_GetGraphEntity(r, ctx->record_idx);

	if(t == REC_TYPE_NODE) node_update = true;

	// If the entity is a node, set its label if possible.
	if(node_update) {
		Node *n = (Node *)entity;
		// Retrieve the node's local label if present.
		label = NODE_GET_LABEL(n);
		// Retrieve the node's Label ID from a local member or the graph.
		int label_id = NODE_GET_LABEL_ID(n, op->gc->g);
		if((label == NULL) && (label_id != GRAPH_NO_LABEL)) {
			// Label ID has been found but its name is unknown, retrieve its name and update the node.
			s = GraphContext_GetSchemaByID(op->gc, label_id, SCHEMA_NODE);
			label = Schema_GetName(s);
			n->label = label;
		}
	}

	if(ctx->mode == UPDATE_REPLACE) {
		PendingUpdateCtx update = {
			.ge            =  entity,
			.label_id      =  label_id,
			// .update_index  =  update_index,
			.attr_id       =  ATTRIBUTE_ALL,
		};

		// Enqueue an update to clear all properties.
		op->updates = array_append(op->updates, update);
	}

	uint exp_count = array_len(ctx->properties);
	for(uint i = 0; i < exp_count; i++) {
		bool update_index = false;
		SIValue new_value = AR_EXP_Evaluate(ctx->properties[i].value, r);

		// Emit an error and exit if we're trying to add an invalid type.
		// NULL is acceptable here as it indicates a deletion.
		if(!(SI_TYPE(new_value) & (SI_VALID_PROPERTY_VALUE | T_NULL))) {
			Error_InvalidPropertyValue();
			ErrorCtx_RaiseRuntimeException(NULL);
			break;
		}

		Attribute_ID attr_id = ctx->properties[i].id;
		// Determine whether we must update the index for this update.
		if(node_update && label_id != GRAPH_NO_LABEL) {
			// If the (label:attribute) combination has an index, take note.
			update_index = GraphContext_GetIndex(op->gc, label, &attr_id, IDX_ANY) != NULL;
		}

		PendingUpdateCtx update = {
			.ge            =  entity,
			.label_id      =  label_id,
			.new_value     =  new_value,
			.update_index  =  update_index,
			.attr_id       =  attr_id,
		};

		// Enqueue the current update.
		op->updates = array_append(op->updates, update);
	}
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
			_EvalEntityUpdates(op, r, (char *)it.key, it.data);
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
	_CommitUpdates(op);
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

