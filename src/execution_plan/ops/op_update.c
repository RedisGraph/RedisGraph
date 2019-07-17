/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_update.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Build an evaluation context foreach update expression. */
static void _BuildUpdateEvalCtx(OpUpdate *op, AST *ast) {
	AST_SetElement *element;
	AST_SetNode *setNode = ast->setNode;
	op->update_expressions_count = Vector_Size(setNode->set_elements);
	op->update_expressions = rm_malloc(sizeof(EntityUpdateEvalCtx) * op->update_expressions_count);

	for(uint i = 0; i < op->update_expressions_count; i++) {
		/* Get a reference to the entity in the SET clause. */
		Vector_Get(setNode->set_elements, i, &element);
		/* Track all required informantion to perform an update. */
		op->update_expressions[i].attribute = element->entity->property;
		op->update_expressions[i].exp = AR_EXP_BuildFromAST(ast, element->exp);
		op->update_expressions[i].entityRecIdx = AST_GetAliasID(op->ast, element->entity->alias);
	}
}

/* Delay updates until all entities are processed,
 * _QueueUpdate will queue up all information necessary to perform an update. */
static void _QueueUpdate(OpUpdate *op, GraphEntity *entity, GraphEntityType type, char *attribute,
						 SIValue new_value) {
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
static void _UpdateIndex(EntityUpdateCtx *ctx, GraphContext *gc, Schema *s) {
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
	if(s) _UpdateIndex(ctx, op->gc, s);
}

static void _UpdateEdge(OpUpdate *op, EntityUpdateCtx *ctx) {
	/* Retrieve GraphEntity:
	* Due to Record freeing we can't maintain the original pointer to GraphEntity object,
	* but only a pointer to an Entity object,
	* to use the GraphEntity_Get, GraphEntity_Add functions we'll use a place holder
	* to hold our entity. */
	Edge *edge = &ctx->e;

	int label_id = Graph_GetEdgeRelation(op->gc->g, edge);
	Schema *s = GraphContext_GetSchemaByID(op->gc, label_id, SCHEMA_EDGE);

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
	}

	if(op->result_set) op->result_set->stats.properties_set += op->pending_updates_count;
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

OpBase *NewUpdateOp(AST *ast, ResultSet *result_set) {
	OpUpdate *op_update = calloc(1, sizeof(OpUpdate));
	op_update->gc = GraphContext_GetFromTLS();
	op_update->ast = ast;
	op_update->result_set = result_set;

	op_update->update_expressions = NULL;
	op_update->update_expressions_count = 0;
	op_update->pending_updates_count = 0;
	op_update->pending_updates_cap = 16; /* 16 seems reasonable number to start with. */
	op_update->pending_updates = rm_malloc(sizeof(EntityUpdateCtx) * op_update->pending_updates_cap);
	op_update->records = NULL;
	op_update->updates_commited = false;

	// Create a context for each update expression.
	_BuildUpdateEvalCtx(op_update, ast);

	// Set our Op operations
	OpBase_Init(&op_update->op);
	op_update->op.name = "Update";
	op_update->op.type = OPType_UPDATE;
	op_update->op.consume = OpUpdateConsume;
	op_update->op.reset = OpUpdateReset;
	op_update->op.free = OpUpdateFree;
	op_update->op.init = OpUpdateInit;

	return (OpBase *)op_update;
}

OpResult OpUpdateInit(OpBase *opBase) {
	OpUpdate *op = (OpUpdate *)opBase;
	if(_ShouldCacheRecord(op)) op->records = array_new(Record, 64);
	return OP_OK;
}

Record OpUpdateConsume(OpBase *opBase) {
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
			SIValue new_value = AR_EXP_Evaluate(update_expression->exp, r);

			// Make sure we're updating either a node or an edge.
			RecordEntryType t = Record_GetType(r, update_expression->entityRecIdx);
			assert(t == REC_TYPE_NODE || t == REC_TYPE_EDGE);
			GraphEntityType type = (t == REC_TYPE_NODE) ? GETYPE_NODE : GETYPE_EDGE;

			GraphEntity *entity = Record_GetGraphEntity(r, update_expression->entityRecIdx);
			_QueueUpdate(op, entity, type, update_expression->attribute, new_value);
		}

		if(_ShouldCacheRecord(op)) {
			op->records = array_append(op->records, r);
		} else {
			// Record not going to be used, discard.
			Record_Free(r);
		}
	}

	/* Done reading, we're not going to call consume any longer
	 * there might be operations e.g. index scan that need to free
	 * index R/W lock, as such free all execution plan operation up the chain. */
	OpBase_PropegateFree(child);

	/* Lock everything. */
	Graph_AcquireWriteLock(op->gc->g);
	_CommitUpdates(op);
	// Release lock.
	Graph_ReleaseLock(op->gc->g);

	op->updates_commited = true;
	return _handoff(op);
}

OpResult OpUpdateReset(OpBase *ctx) {
	OpUpdate *op = (OpUpdate *)ctx;
	// Reset all pending updates.
	op->pending_updates_count = 0;
	op->pending_updates_cap = 16; /* 16 seems reasonable number to start with. */
	op->pending_updates = rm_realloc(op->pending_updates,
									 op->pending_updates_cap * sizeof(EntityUpdateCtx));
	return OP_OK;
}

void OpUpdateFree(OpBase *ctx) {
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
		for(uint i = 0; i < records_count; i++) Record_Free(op->records[i]);
		array_free(op->records);
		op->records = NULL;
	}

	if(op->update_expressions) {
		rm_free(op->update_expressions);
		op->update_expressions = NULL;
	}

	if(op->pending_updates) {
		rm_free(op->pending_updates);
		op->pending_updates = NULL;
	}
}
