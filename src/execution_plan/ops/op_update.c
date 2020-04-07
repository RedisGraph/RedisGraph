/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_update.h"
#include "../../query_ctx.h"
#include "../../util/arr.h"
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
						 const char *attribute, SIValue new_value) {
	/* Make sure we've got enough room in queue. */
	if(op->pending_updates_count == op->pending_updates_cap) {
		op->pending_updates_cap *= 2;
		op->pending_updates = rm_realloc(op->pending_updates,
										 op->pending_updates_cap * sizeof(EntityUpdateCtx));
	}

	uint i = op->pending_updates_count;
	op->pending_updates[i].new_value = new_value;
	op->pending_updates[i].attribute = attribute;
	op->pending_updates[i].attr_id = GraphContext_GetAttributeID(op->gc, attribute);
	op->pending_updates[i].entity_type = type;
	// Copy updated entity.
	if(type == GETYPE_NODE) {
		op->pending_updates[i].n = *((Node *)entity);
	} else {
		op->pending_updates[i].e = *((Edge *)entity);
	}
	op->pending_updates_count++;
}

/* Introduce updated entity to index. */
static void _UpdateIndex(EntityUpdateCtx *ctx, GraphContext *gc, Schema *s, SIValue *old_value,
						 SIValue *new_value) {
	Node *n = &ctx->n;
	EntityID node_id = ENTITY_GET_ID(n);

	// Reindex.
	Schema_AddNodeToIndices(s, n, true);
}

static void _UpdateNode(OpUpdate *op, EntityUpdateCtx *ctx) {
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
		// Add new property.
		GraphEntity_AddProperty((GraphEntity *)node, ctx->attr_id, ctx->new_value);
	} else {
		// Update property.
		GraphEntity_SetProperty((GraphEntity *)node, ctx->attr_id, ctx->new_value);
	}

	// Update index for node entities.
	_UpdateIndex(ctx, op->gc, s, old_value, &ctx->new_value);
}

static void _UpdateEdge(OpUpdate *op, EntityUpdateCtx *ctx) {
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
		// Add new property.
		GraphEntity_AddProperty((GraphEntity *)edge, ctx->attr_id, ctx->new_value);
	} else {
		// Update property.
		GraphEntity_SetProperty((GraphEntity *)edge, ctx->attr_id, ctx->new_value);
	}
}

/* Executes delayed updates. */
static void _CommitUpdates(OpUpdate *op) {
	for(uint i = 0; i < op->pending_updates_count; i++) {
		EntityUpdateCtx *ctx = &op->pending_updates[i];
		// Map the attribute key if it has not been encountered before
		if(ctx->attr_id == ATTRIBUTE_NOTFOUND) {
			ctx->attr_id = GraphContext_FindOrAddAttribute(op->gc, ctx->attribute);
		}
		if(ctx->entity_type == GETYPE_NODE) {
			_UpdateNode(op, ctx);
		} else {
			_UpdateEdge(op, ctx);
		}
		SIValue_Free(ctx->new_value);
	}

	if(op->stats) op->stats->properties_set += op->pending_updates_count;
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

OpBase *NewUpdateOp(const ExecutionPlan *plan, EntityUpdateEvalCtx *update_exps) {
	OpUpdate *op = rm_calloc(1, sizeof(OpUpdate));
	op->gc = QueryCtx_GetGraphCtx();
	op->stats = QueryCtx_GetResultSetStatistics();
	op->records = NULL;
	op->updates_commited = false;
	op->pending_updates_cap = 16; /* 16 seems reasonable number to start with. */
	op->pending_updates_count = 0;
	op->update_expressions = update_exps;
	op->update_expressions_count = array_len(update_exps);
	op->pending_updates = rm_malloc(sizeof(EntityUpdateCtx) * op->pending_updates_cap);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_UPDATE, "Update", UpdateInit, UpdateConsume,
				UpdateReset, NULL, UpdateClone, UpdateFree, true, plan);

	for(uint i = 0; i < op->update_expressions_count; i ++) {
		op->update_expressions[i].record_idx = OpBase_Modifies((OpBase *)op, update_exps[i].alias);
	}

	return (OpBase *)op;
}

static OpResult UpdateInit(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	if(_ShouldCacheRecord(op)) op->records = array_new(Record, 64);
	return OP_OK;
}

static Record UpdateConsume(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	OpBase *child = op->op.children[0];
	Record r;

	// Updates already performed.
	if(op->updates_commited) return _handoff(op);

	while((r = OpBase_Consume(child))) {
		/* Evaluate each update expression and store result
		 * for later execution. */
		EntityUpdateEvalCtx *update_expression = op->update_expressions;
		for(uint i = 0; i < op->update_expressions_count; i++, update_expression++) {
			SIValue new_value = SI_CloneValue(AR_EXP_Evaluate(update_expression->exp, r));
			// Make sure we're updating either a node or an edge.
			RecordEntryType t = Record_GetType(r, update_expression->record_idx);
			assert(t == REC_TYPE_NODE || t == REC_TYPE_EDGE);
			GraphEntityType type = (t == REC_TYPE_NODE) ? GETYPE_NODE : GETYPE_EDGE;

			GraphEntity *entity = Record_GetGraphEntity(r, update_expression->record_idx);
			_QueueUpdate(op, entity, type, update_expression->attribute, new_value);
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

