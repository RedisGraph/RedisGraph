/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_update.h"
#include "../../query_ctx.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include "../../util/rmalloc.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult UpdateInit(OpBase *opBase);
static Record UpdateConsume(OpBase *opBase);
static OpResult UpdateReset(OpBase *opBase);
static OpBase *UpdateClone(const ExecutionPlan *plan, const OpBase *opBase);
static void UpdateFree(OpBase *opBase);

static int _UpdateEntity(GraphEntity *ge, PendingUpdateCtx *update) {
	int res = 1;
	SIValue new_value = update->new_value;
	Attribute_ID attr_id = update->attr_id;

	// Try to get current property value.
	SIValue *old_value = GraphEntity_GetProperty(ge, attr_id);

	if(old_value == PROPERTY_NOTFOUND) {
		// Adding a new property; do nothing if its value is NULL.
		if(SI_TYPE(new_value) == T_NULL) {
			res = 0;
			goto cleanup;
		}
		GraphEntity_AddProperty(ge, attr_id, new_value);
	} else {
		// Update property.
		GraphEntity_SetProperty(ge, attr_id, new_value);
	}

cleanup:
	SIValue_Free(new_value);
	return res;
}

/* Set a property on an edge. For non-NULL values, the property
 * will be added or updated if it is already present.
 * For NULL values, the property will be deleted if present
 * and nothing will be done otherwise.
 * Returns 1 if a property was set or deleted. */
static int _UpdateEdge(OpUpdate *op, PendingUpdateCtx *updates,
					   uint update_count) {
	/* Retrieve GraphEntity:
	 * Due to Record freeing we can't maintain the original pointer to
	 * GraphEntity object, but only a pointer to an Entity object, to use the
	 * GraphEntity_Get, GraphEntity_Add functions we'll use a place holder to
	 * hold our entity. */
	int attributes_set = 0;
	GraphEntity *ge = (GraphEntity *)&updates->e;

	for(uint i = 0; i < update_count; i++) {
		PendingUpdateCtx *update = updates + i;
		attributes_set += _UpdateEntity(ge, update);
	}

	return attributes_set;
}

/* Set a property on a node. For non-NULL values, the property
 * will be added or updated if it is already present.
 * For NULL values, the property will be deleted if present
 * and nothing will be done otherwise.
 * Relevant indexes will be updated if required.
 * Returns 1 if a property was set or deleted.  */
static int _UpdateNode(OpUpdate *op, PendingUpdateCtx *updates,
					   uint update_count) {
	/* Retrieve GraphEntity:
	 * Due to Record freeing we can't maintain the original pointer to
	 * GraphEntity object, but only a pointer to an Entity object, to use the
	 * GraphEntity_Get, GraphEntity_Add functions we'll use a place holder to
	 * hold our entity. */
	int attributes_set = 0;
	bool update_index = false;
	Node *node = &updates->n;
	GraphEntity *ge = (GraphEntity *)node;

	for(uint i = 0; i < update_count; i++) {
		PendingUpdateCtx *update = updates + i;
		attributes_set += _UpdateEntity(ge, update);
		// Do we need to update an index for this property?
		update_index |= update->update_index;
	}

	// Update index for node entities if indexed fields have been modified.
	if(update_index) {
		int label_id = node->labelID;
		Schema *s = GraphContext_GetSchemaByID(op->gc, label_id, SCHEMA_NODE);
		// Introduce updated entity to index.
		Schema_AddNodeToIndices(s, node);
	}

	return attributes_set;
}

static void _CommitEntityUpdates(OpUpdate *op, EntityUpdateCtx *ctx) {
	uint properties_set = 0;
	uint updates_per_entity = array_len(ctx->exps);
	// Total_updates = updates_per_entity * number of entities being updated.
	uint total_updates = array_len(ctx->updates);

	/* For each iteration of this loop, perform all updates
	 * enqueued for a single entity. */
	for(uint i = 0; i < total_updates; i += updates_per_entity) {
		// Set a pointer to the first update context for this entity.
		PendingUpdateCtx *update_ctx = ctx->updates + i;
		if(update_ctx->entity_type == GETYPE_NODE) {
			properties_set += _UpdateNode(op, update_ctx, updates_per_entity);
		} else {
			properties_set += _UpdateEdge(op, update_ctx, updates_per_entity);
		}
	}

	if(op->stats) op->stats->properties_set += properties_set;
}

// Commits delayed updates.
static void _CommitUpdates(OpUpdate *op) {
	uint entity_count = array_len(op->update_ctxs);
	for(uint i = 0; i < entity_count; i++) {
		EntityUpdateCtx *entity_ctx = &op->update_ctxs[i];
		_CommitEntityUpdates(op, entity_ctx);
	}
}

/* We only cache records if op_update is not the last
 * operation within the execution-plan, which means
 * others operations might inspect thoese records.
 * Example: MATCH (n) SET n.v=n.v+1 RETURN n */
static inline bool _ShouldCacheRecord(OpUpdate *op) {
	return (op->op.parent != NULL);
}

static Record _handoff(OpUpdate *op) {
	/* TODO: poping a record out of op->records
	 * will reverse the order in which records
	 * are passed down the execution plan. */
	if(op->records && array_len(op->records) > 0) return array_pop(op->records);
	return NULL;
}

static void _groupUpdateExps(OpUpdate *op, EntityUpdateEvalCtx *update_ctxs) {
	// Sort update contexts by unique Record ID.
	#define islt(a,b) (a->record_idx < b->record_idx)

	uint n = array_len(update_ctxs);
	QSORT(EntityUpdateEvalCtx, update_ctxs, n, islt);

	/* Create an EntityUpdateCtx for each alias that will be modified by this op.
	 * Group all relevant EvalCtx structs on each new context. */
	op->update_ctxs = array_new(EntityUpdateCtx, 1);

	EntityUpdateEvalCtx *prev = NULL;
	EntityUpdateCtx *entity_ctx = NULL;

	for(uint i = 0; i < n; i++) {
		EntityUpdateEvalCtx *current = &update_ctxs[i];
		if(!prev || current->record_idx != prev->record_idx) {
			// Encountered a diffrent entity being updated; create a new context.
			EntityUpdateCtx new_ctx = { .alias = current->alias,
										.record_idx = current->record_idx,
										.updates = array_new(PendingUpdateCtx, 1),
										.exps = array_new(EntityUpdateEvalCtx, 1),
									  };

			// Append the new context to the array of contexts.
			op->update_ctxs = array_append(op->update_ctxs, new_ctx);

			// Update the entity_ctx pointer to reference the new context.
			uint latest_group_idx = array_len(op->update_ctxs) - 1;
			entity_ctx = &op->update_ctxs[latest_group_idx];

			prev = current;
		}

		// Append the current expression to the latest context.
		entity_ctx->exps = array_append(entity_ctx->exps, *current);
	}
}

OpBase *NewUpdateOp(const ExecutionPlan *plan, EntityUpdateEvalCtx
		*update_exps) {
	OpUpdate *op = rm_calloc(1, sizeof(OpUpdate));
	op->records = NULL;
	op->update_ctxs = NULL;
	op->updates_commited = false;
	op->gc = QueryCtx_GetGraphCtx();

	// Set our Op operations.
	OpBase_Init((OpBase *)op, OPType_UPDATE, "Update", UpdateInit, UpdateConsume,
				UpdateReset, NULL, UpdateClone, UpdateFree, true, plan);

	// Set the record index for every entity modified by this operation.
	uint exp_count = array_len(update_exps);
	for(uint i = 0; i < exp_count; i++) {
		update_exps[i].record_idx = OpBase_Modifies((OpBase *)op, update_exps[i].alias);
	}

	// Group update expression by entity.
	_groupUpdateExps(op, update_exps);

	return (OpBase *)op;
}

static OpResult UpdateInit(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	op->stats = QueryCtx_GetResultSetStatistics();
	if(_ShouldCacheRecord(op)) op->records = array_new(Record, 64);
	return OP_OK;
}

static void _EvalEntityUpdates(EntityUpdateCtx *ctx, GraphContext *gc,
							   Record r) {
	Schema *s = NULL;
	const char *label = NULL;
	bool update_index = false;
	int label_id = GRAPH_NO_LABEL;

	// Get the type of the entity to update. If the expected entity was not
	// found, make no updates but do not error.
	RecordEntryType t = Record_GetType(r, ctx->record_idx);
	if(t == REC_TYPE_UNKNOWN) return;

	// Make sure we're updating either a node or an edge.
	if(t != REC_TYPE_NODE && t != REC_TYPE_EDGE) {
		QueryCtx_SetError("Update error: alias '%s' did not resolve to a graph entity",
						  ctx->alias);
		QueryCtx_RaiseRuntimeException();
	}

	GraphEntity *entity = Record_GetGraphEntity(r, ctx->record_idx);
	GraphEntityType type = (t == REC_TYPE_NODE) ? GETYPE_NODE : GETYPE_EDGE;

	// If the entity is a node, set its label if possible.
	if(type == GETYPE_NODE) {
		Node *n = (Node *)entity;
		label_id = Graph_GetNodeLabel(gc->g, ENTITY_GET_ID(entity));

		if(label_id != GRAPH_NO_LABEL) {
			s = GraphContext_GetSchemaByID(gc, label_id, SCHEMA_NODE);
			label = Schema_GetName(s);
		}

		n->label = label;
		n->labelID = label_id;
	}

	uint exp_count = array_len(ctx->exps);
	for(uint i = 0; i < exp_count; i++) {
		EntityUpdateEvalCtx *update_ctx = ctx->exps + i;
		SIValue new_value = AR_EXP_Evaluate(update_ctx->exp, r);

		/* Determine whether we must update the index for this set of updates.
		 * If at least one property being updated is indexed, each node will be reindexed. */
		if(!update_index && label) {
			Attribute_ID attr_id = update_ctx->attribute_id;
			const char *field = GraphContext_GetAttributeString(gc, attr_id);
			// If the label-index combination has an index, we must reindex this entity.
			update_index = GraphContext_GetIndex(gc, label, field, IDX_ANY) != NULL;
			if(update_index && (i > 0)) {
				/* Swap the current update expression with the first one
				 * so that subsequent searches will find the index immediately. */
				EntityUpdateEvalCtx first = ctx->exps[0];
				ctx->exps[0] = ctx->exps[i];
				ctx->exps[i] = first;
			}
		}

		PendingUpdateCtx update = {
			.entity_type = type,
			.new_value = new_value,
			.update_index = update_index,
			.attr_id = update_ctx->attribute_id,
		};

		if(type == GETYPE_EDGE) {
			// Add the edge to the update context.
			update.e = *((Edge *)entity);
		} else {
			// Add the node to the update context.
			update.n = *((Node *)entity);
		}

		// Enqueue the current update.
		ctx->updates = array_append(ctx->updates, update);
	}
}

static Record UpdateConsume(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	OpBase *child = op->op.children[0];
	Record r;

	// Updates already performed.
	if(op->updates_commited) return _handoff(op);

	uint nctx = array_len(op->update_ctxs);

	while((r = OpBase_Consume(child))) {
		// Evaluate update expressions.
		for(uint i = 0; i < nctx; i++) {
			_EvalEntityUpdates(op->update_ctxs + i, op->gc, r);
		}

		if(_ShouldCacheRecord(op)) {
			op->records = array_append(op->records, r);
		} else {
			// Record not going to be used, discard.
			OpBase_DeleteRecord(r);
		}
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

// Collect all EvalCtx structs held within elements of the inputs array.
static EntityUpdateEvalCtx *_CollectEvalContexts(EntityUpdateCtx *inputs) {
	EntityUpdateEvalCtx *eval_contexts = array_new(EntityUpdateEvalCtx, 1);
	uint nctx = array_len(inputs);

	// For each update context, collect all EvalCtx structs.
	for(uint i = 0; i < nctx; i++) {
		EntityUpdateCtx update_ctx = inputs[i];
		uint nexps = array_len(update_ctx.exps);

		for(uint j = 0; j < nexps; j++) {
			EntityUpdateEvalCtx exp = update_ctx.exps[j];
			eval_contexts = array_append(eval_contexts, EntityUpdateEvalCtx_Clone(exp));
		}
	}

	return eval_contexts;
}

static OpBase *UpdateClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_UPDATE);
	OpUpdate *op = (OpUpdate *)opBase;

	// Recreate original input.
	EntityUpdateEvalCtx *update_exps = _CollectEvalContexts(op->update_ctxs);
	OpBase *clone = NewUpdateOp(plan, update_exps);
	array_free(update_exps);

	return clone;
}

static OpResult UpdateReset(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;
	return OP_OK;
}

static void _UpdateContexts_Free(EntityUpdateCtx *contexts) {
	uint ctx_count = array_len(contexts);
	for(uint i = 0; i < ctx_count; i++) {
		EntityUpdateCtx update_ctx = contexts[i];
		if(update_ctx.exps) {
			uint eval_ctx_count = array_len(update_ctx.exps);
			for(uint j = 0; j < eval_ctx_count; j++) {
				AR_EXP_Free(update_ctx.exps[j].exp);
			}
			array_free(update_ctx.exps);
		}
		if(update_ctx.updates) array_free(update_ctx.updates);
	}
}

static void UpdateFree(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;
	// Free each update context.
	if(op->update_ctxs) {
		_UpdateContexts_Free(op->update_ctxs);
		array_free(op->update_ctxs);
		op->update_ctxs = NULL;
	}

	if(op->records) {
		uint records_count = array_len(op->records);
		for(uint i = 0; i < records_count; i++) OpBase_DeleteRecord(op->records[i]);
		array_free(op->records);
		op->records = NULL;
	}
}

