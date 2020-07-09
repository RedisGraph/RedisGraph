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

/* Delay updates until all entities are processed,
 * _QueueUpdate will queue up all information necessary to perform an update. */
static void _QueueUpdate(OpUpdate *op, GraphEntity *entity, GraphEntityType type,
						 Attribute_ID attr_id, SIValue new_value) {
	op->pending_updates[i].new_value = new_value;
	op->pending_updates[i].attr_id = attr_id;
	// Copy updated entity.
	if(type == GETYPE_NODE) {
		op->pending_updates[i].n = *((Node *)entity);
	} else {
		op->pending_updates[i].e = *((Edge *)entity);
	}
}

/* Introduce updated entity to index. */
static void _UpdateIndex(EntityUpdateCtx *ctx, GraphContext *gc, Schema *s, SIValue *old_value,
						 SIValue *new_value) {
	Node *n = &ctx->n;
	EntityID node_id = ENTITY_GET_ID(n);

	// Reindex.
	Schema_AddNodeToIndices(s, n, true);
}

/* Set a property on a node. For non-NULL values, the property
 * will be added or updated if it is already present.
 * For NULL values, the property will be deleted if present
 * and nothing will be done otherwise.
 * Relevant indexes will be updated accordingly.
 * Returns 1 if a property was set or deleted. */
static int _UpdateNode(OpUpdate *op, EntityUpdateCtx *ctx) {
	/* Retrieve GraphEntity:
	 * Due to Record freeing we can't maintain the original pointer to GraphEntity object,
	 * but only a pointer to an Entity object,
	 * to use the GraphEntity_Get, GraphEntity_Add functions we'll use a place holder
	 * to hold our entity. */
	Schema *s = NULL;
	Node *node = &ctx->n;

	int label_id = Graph_GetNodeLabel(op->gc->g, ENTITY_GET_ID(node));
	if(label_id != GRAPH_NO_LABEL) {
		s = GraphContext_GetSchemaByID(op->gc, label_id, SCHEMA_NODE);
	}

	// Try to get current property value.
	SIValue *old_value = GraphEntity_GetProperty((GraphEntity *)node, ctx->attr_id);

	if(old_value == PROPERTY_NOTFOUND) {
		// Adding a new property; do nothing if its value is NULL.
		if(SI_TYPE(ctx->new_value) == T_NULL) return 0;
		GraphEntity_AddProperty((GraphEntity *)node, ctx->attr_id, ctx->new_value);
	} else {
		// Update property.
		GraphEntity_SetProperty((GraphEntity *)node, ctx->attr_id, ctx->new_value);
	}

	// Update index for node entities.
	_UpdateIndex(ctx, op->gc, s, old_value, &ctx->new_value);
	return 1;
}

/* Set a property on an edge. For non-NULL values, the property
 * will be added or updated if it is already present.
 * For NULL values, the property will be deleted if present
 * and nothing will be done otherwise.
 * Returns 1 if a property was set or deleted. */
static int _UpdateEdge(OpUpdate *op, EntityUpdateCtx *ctx) {
	/* Retrieve GraphEntity:
	* Due to Record freeing we can't maintain the original pointer to GraphEntity object,
	* but only a pointer to an Entity object,
	* to use the GraphEntity_Get, GraphEntity_Add functions we'll use a place holder
	* to hold our entity. */
	Edge *edge = &ctx->e;

	int label_id = Graph_GetEdgeRelation(op->gc->g, edge);

	// Try to get current property value.
	SIValue *old_value = GraphEntity_GetProperty((GraphEntity *)edge, ctx->attr_id);

	if(old_value == PROPERTY_NOTFOUND) {
		// Adding a new property; do nothing if its value is NULL.
		if(SI_TYPE(ctx->new_value) == T_NULL) return 0;
		GraphEntity_AddProperty((GraphEntity *)edge, ctx->attr_id, ctx->new_value);
	} else {
		// Update property.
		GraphEntity_SetProperty((GraphEntity *)edge, ctx->attr_id, ctx->new_value);
	}
	return 1;
}

/* Executes delayed updates. */
static void _CommitUpdates(OpUpdate *op) {
	uint properties_set = 0;
	for(uint i = 0; i < op->pending_updates_count; i++) {
		EntityUpdateCtx *ctx = &op->pending_updates[i];
		if(ctx->entity_type == GETYPE_NODE) {
			properties_set += _UpdateNode(op, ctx);
		} else {
			properties_set += _UpdateEdge(op, ctx);
		}
		SIValue_Free(ctx->new_value);
	}

	if(op->stats) op->stats->properties_set += properties_set;
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
	// sort update contexts on updated entity
	#define islt(a,b) (strcmp((*a).alias,(*b).alias)<0)

	int n = array_len(update_ctxs);
	QSORT(EntityUpdateEvalCtx, update_ctxs, n, islt);

	// group expression by modified entity
	EntityUpdateCtx entity_ctx;
	entity_ctx.alias = update_ctxs[0].alias;
	entity_ctx.record_idx = update_ctxs[0].record_idx;
	entity_ctx.updates = array_new(PendingUpdateCtx, 1);
	entity_ctx.exps = array_new(EntityUpdateEvalCtx, 1);
	entity_ctx.exps = array_append(entity_ctx.exps, update_ctxs[0]);

	EntityUpdateCtx *groups = array_new(EntityUpdateCtx, 1);
	group = array_append(group, entity_ctx);

	for(int i = 1; i < n; i++) {
		EntityUpdateEvalCtx  next     =  update_ctxs[i];
		EntityUpdateEvalCtx  current  =  entity_ctx.exps[0];

		if(strcmp(next.alias, current.alias)) {
			// encounter new entity, save current ctx and create new one
			entity_ctx.nexp = array_len(entity_ctx.exps);
			groups = array_append(groups, entity_ctx);

			entity_ctx.alias = next.alias;
			entity_ctx.record_idx = next.record_idx;
			entity_ctx.updates = array_new(PendingUpdateCtx, 1);
			entity_ctx.exps = array_new(EntityUpdateEvalCtx, 1);
		}

		// add expression to group
		entity_ctx.exps = array_append(entity_ctx.exps, next);
	}

	// save last group
	entity_ctx.nexp = array_len(entity_ctx.exps);
	groups = array_append(groups, entity_ctx);

	op->update_ctxs = groups;
}

OpBase *NewUpdateOp(const ExecutionPlan *plan, EntityUpdateEvalCtx *update_exps) {
	OpUpdate *op = rm_calloc(1, sizeof(OpUpdate));
	op->gc                     =  QueryCtx_GetGraphCtx();
	op->records                =  NULL;
	op->updates_commited       =  false;
	op->pending_updates_cap    =  16;		// 16 seems reasonable number to start with
	op->pending_updates_count  =  0;
	op->pending_updates        =  NULL;
	op->update_ctxs            =  NULL:

	// set our Op operations
	OpBase_Init((OpBase *)op, OPType_UPDATE, "Update", UpdateInit, UpdateConsume,
				UpdateReset, NULL, UpdateClone, UpdateFree, true, plan);

	// set updated entity record entry index
	uint nexp = array_len(update_exps);
	for(uint i = 0; i < nexp; i++) {
		update_exps[i].record_idx = OpBase_Modifies((OpBase *)op, update_exps[i].alias);
	}

	// group update expression by entity
	_groupUpdateExps(op, update_exps);

	return (OpBase *)op;
}

static OpResult UpdateInit(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	op->stats = QueryCtx_GetResultSetStatistics();
	if(_ShouldCacheRecord(op)) op->records = array_new(Record, 64);
}

static void _EvalEntityUpdates(EntityUpdateCtx *ctx, Record r) {
	// get the type of the entity to update
	// if the expected entity was not found
	// make no updates but do not error
	RecordEntryType t = Record_GetType(r, ctx.record_idx);
	if(t == REC_TYPE_UNKNOWN) continue;

	// make sure we're updating either a node or an edge
	if(t != REC_TYPE_NODE && t != REC_TYPE_EDGE) {
		QueryCtx_SetError("Update error: alias '%s' did not resolve to
				a graph entity", update_expression->alias);
		QueryCtx_RaiseRuntimeException();
	}

	GraphEntityType type;
	(t == REC_TYPE_NODE) ? type GETYPE_NODE: GETYPE_EDGE;
	GraphEntity *entity = Record_GetGraphEntity(r, ctx.record_idx);

	for(uint j = 0; j < ctx.nexp; j++) {
		SIValue new_value = SI_CloneValue(AR_EXP_Evaluate(ctx.exps[j].exp, r));

		PndingUpdateCtx update;
		update.
			pending_updates[i].new_value = new_value;
		pending_updates[i].attr_id = attr_id;
		ctx.updates = array_append(ctx.updates, );
		_QueueUpdate(op, entity, type, update_expression->attribute_id, new_value);
	}

	pending_update->entity_type = type;
	if(type == GETYPE_NODE) pending_updates.n = *((Node *)entity);
	else pending_updates.e = *((Edge *)entity);
}

static Record UpdateConsume(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	OpBase *child = op->op.children[0];
	Record r;

	// Updates already performed.
	if(op->updates_commited) return _handoff(op);

	uint nctx = array_len(op->update_ctxs);

	while((r = OpBase_Consume(child))) {
		// evaluate update expressions
		for(uint i = 0; i < nctx; i++) {
			_EvalEntityUpdates(op->update_ctxs + i, r);
		}

		if(_ShouldCacheRecord(op)) {
			op->records = array_append(op->records, r);
		} else {
			// Record not going to be used, discard.
			OpBase_DeleteRecord(r);
		}
	}

	/* Done reading, we're not going to call consume any longer
	 * there might be operations e.g. index scan that need to free
	 * index R/W lock, as such free all execution plan operation up the chain. */
	OpBase_PropagateFree(child);

	/* Lock everything. */
	QueryCtx_LockForCommit();
	_CommitUpdates(op);
	// Release lock.
	QueryCtx_UnlockCommit(opBase);

	op->updates_commited = true;
	return _handoff(op);
}

static OpBase *UpdateClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_UPDATE);
	OpUpdate *op = (OpUpdate *)opBase;
	EntityUpdateEvalCtx *update_exps;
	array_clone_with_cb(update_exps, op->update_expressions, EntityUpdateEvalCtx_Clone);
	return NewUpdateOp(plan, update_exps);
}

static OpResult UpdateReset(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;
	// Reset all pending updates.
	op->pending_updates_count = 0;
	op->pending_updates_cap = 16; /* 16 seems reasonable number to start with. */
	op->pending_updates = rm_realloc(op->pending_updates,
									 op->pending_updates_cap * sizeof(EntityUpdateCtx));
	return OP_OK;
}

static void UpdateFree(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;
	/* Free each update context. */
	if(op->update_expressions_count) {
		for(uint i = 0; i < op->update_expressions_count; i++) {
			AR_EXP_Free(op->update_expressions[i].exp);
		}
		op->update_expressions_count = 0;
	}

	if(op->records) {
		uint records_count = array_len(op->records);
		for(uint i = 0; i < records_count; i++) OpBase_DeleteRecord(op->records[i]);
		array_free(op->records);
		op->records = NULL;
	}

	if(op->update_expressions) {
		array_free(op->update_expressions);
		op->update_expressions = NULL;
	}

	if(op->pending_updates) {
		rm_free(op->pending_updates);
		op->pending_updates = NULL;
	}
}

