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

/* Introduce updated entity to index. */
static void _UpdateIndex(PendingUpdateCtx *ctx, GraphContext *gc, Schema *s, SIValue *old_value,
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
static int _UpdateNode(OpUpdate *op, PendingUpdateCtx *ctx) {
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

	// Update index for node entities if they are modified.
	// TODO only required once per node after all updates are committed, not per update.
	if(ctx->update_index) _UpdateIndex(ctx, op->gc, s, old_value, &ctx->new_value);
	return 1;
}

/* Set a property on an edge. For non-NULL values, the property
 * will be added or updated if it is already present.
 * For NULL values, the property will be deleted if present
 * and nothing will be done otherwise.
 * Returns 1 if a property was set or deleted. */
static int _UpdateEdge(OpUpdate *op, PendingUpdateCtx *ctx) {
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
	uint  properties_set  =  0;
	uint  entity_count    =  array_len(op->update_ctxs);

	for(uint i = 0; i < entity_count; i++) {
		EntityUpdateCtx *entity_ctx = &op->update_ctxs[i];
		uint entity_update_count = array_len(entity_ctx->updates);
		for(uint j = 0; j < entity_update_count; j ++) {
			PendingUpdateCtx *ctx = &entity_ctx->updates[j];
			if(ctx->entity_type == GETYPE_NODE) {
				properties_set += _UpdateNode(op, ctx);
			} else {
				properties_set += _UpdateEdge(op, ctx);
			}
			SIValue_Free(ctx->new_value);
		}
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
	#define islt(a,b) (a->record_idx < b->record_idx)

	uint n = array_len(update_ctxs);
	QSORT(EntityUpdateEvalCtx, update_ctxs, n, islt);

	// each OpUpdate is initialized with a flex array of EvalCtx structs, which describe
	// the entity and property being updated as well as an AR_ExpNode to represent the new property value. */
	// group expression by modified entity
	EntityUpdateCtx *groups = array_new(EntityUpdateCtx, 1);

	EntityUpdateEvalCtx  *prev        =  NULL;
	EntityUpdateCtx      *entity_ctx  =  NULL;

	for(uint i = 0; i < n; i++) {
		EntityUpdateEvalCtx *current = &update_ctxs[i];
		if(!prev || current->record_idx != prev->record_idx) {
			// Encountered new entity or are the first, create new context.
			EntityUpdateCtx new_ctx = { 0 };
			groups = array_append(groups, new_ctx);
			entity_ctx = &groups[array_len(groups) - 1];

			entity_ctx->alias = current->alias;
			entity_ctx->record_idx = current->record_idx;
			entity_ctx->updates = array_new(PendingUpdateCtx, 1);
			entity_ctx->exps = array_new(EntityUpdateEvalCtx, 1);

			prev = current;
		}

		entity_ctx->exps = array_append(entity_ctx->exps, *current);
	}

	op->update_ctxs = groups;
}

OpBase *NewUpdateOp(const ExecutionPlan *plan, EntityUpdateEvalCtx *update_exps) {
	OpUpdate *op = rm_calloc(1, sizeof(OpUpdate));
	op->gc                     =  QueryCtx_GetGraphCtx();
	op->records                =  NULL;
	op->updates_commited       =  false;
	op->update_ctxs            =  NULL;

	// set our Op operations
	OpBase_Init((OpBase *)op, OPType_UPDATE, "Update", UpdateInit, UpdateConsume,
				UpdateReset, NULL, UpdateClone, UpdateFree, true, plan);

	// set updated entity record entry index
	uint exp_count = array_len(update_exps);
	for(uint i = 0; i < exp_count; i++) {
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
	return OP_OK;
}

static void _EvalEntityUpdates(EntityUpdateCtx *ctx, Record r) {
	Node          *n            =  NULL;
	Edge          *e            =  NULL;
	Schema        *s            =  NULL;
	const char    *label        =  NULL;
	bool          update_index  =  false;
	int           label_id      =  GRAPH_NO_LABEL;
	GraphContext  *gc           =  QueryCtx_GetGraphCtx();

	// get the type of the entity to update
	// if the expected entity was not found
	// make no updates but do not error
	RecordEntryType t = Record_GetType(r, ctx->record_idx);
	if(t == REC_TYPE_UNKNOWN) return;

	// make sure we're updating either a node or an edge
	if(t != REC_TYPE_NODE && t != REC_TYPE_EDGE) {
		QueryCtx_SetError("Update error: alias '%s' did not resolve to a graph entity",
						  ctx->alias);
		QueryCtx_RaiseRuntimeException();
	}

	GraphEntity *entity = Record_GetGraphEntity(r, ctx->record_idx);
	GraphEntityType type = (t == REC_TYPE_NODE) ? GETYPE_NODE : GETYPE_EDGE;

	if(type == GETYPE_EDGE) {
		e = (Edge *)entity;
	} else {
		n      =  (Node *)entity;
		label  =  n->label; // will be set if specified in query string

		if(label == NULL) {
			label_id = Graph_GetNodeLabel(gc->g, ENTITY_GET_ID(entity));
			s = GraphContext_GetSchemaByID(gc, label_id, SCHEMA_NODE);
			if(s) label = Schema_GetName(s);
		}

		n->label = label;
		n->labelID = label_id;
	}

	uint exp_count = array_len(ctx->exps);
	for(uint i = 0; i < exp_count; i++) {
		EntityUpdateEvalCtx *update_ctx = ctx->exps + i;
		SIValue new_value = SI_CloneValue(AR_EXP_Evaluate(update_ctx->exp, r));

		// determine whether we must update the index for this set of updates
		if(!update_index && label) {
			Attribute_ID attr_id = update_ctx->attribute_id;
			const char *field = GraphContext_GetAttributeString(gc, attr_id);
			update_index = GraphContext_GetIndex(gc, label, field, IDX_ANY);
			if(update_index && i > 0) {
				// swap the current update expression with the first one
				// so that subsequent searches will find the index immediately
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
			// add the edge to the update context
			update.e = *((Edge *)entity);
		} else {
			// add the node to the update context
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
	uint ctx_count = array_len(op->update_ctxs);
	EntityUpdateEvalCtx *update_exps = array_new(EntityUpdateEvalCtx, ctx_count);
	// TODO tmp
	for(uint i = 0; i < ctx_count; i ++) {
		EntityUpdateCtx *ctx = &op->update_ctxs[i];
		uint entity_update_count = array_len(ctx->exps);
		for(uint j = 0; j < entity_update_count; j ++) {
			update_exps = array_append(update_exps, ctx->exps[j]);
		}
	}
	// array_clone_with_cb(update_exps, op->update_expressions, EntityUpdateEvalCtx_Clone);
	return NewUpdateOp(plan, update_exps);
}

static OpResult UpdateReset(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;
	// Reset all pending updates.
	// TODO
	// op->pending_updates_count = 0;
	// op->pending_updates_cap = 16; [> 16 seems reasonable number to start with. <]
	// op->pending_updates = rm_realloc(op->pending_updates,
	// op->pending_updates_cap * sizeof(EntityUpdateCtx));
	return OP_OK;
}

static void UpdateFree(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;
	/* Free each update context. */
	// TODO
	// if(op->update_expressions_count) {
	// for(uint i = 0; i < op->update_expressions_count; i++) {
	// AR_EXP_Free(op->update_expressions[i].exp);
	// }
	// op->update_expressions_count = 0;
	// }

	// if(op->records) {
	// uint records_count = array_len(op->records);
	// for(uint i = 0; i < records_count; i++) OpBase_DeleteRecord(op->records[i]);
	// array_free(op->records);
	// op->records = NULL;
	// }

	// if(op->update_expressions) {
	// array_free(op->update_expressions);
	// op->update_expressions = NULL;
	// }

	// if(op->pending_updates) {
	// rm_free(op->pending_updates);
	// op->pending_updates = NULL;
	// }
}

