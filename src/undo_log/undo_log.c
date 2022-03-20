/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "undo_log.h"
#include "query_ctx.h"
#include "../execution_plan/ops/shared/update_functions.h"
#include "../execution_plan/ops/shared/create_functions.h"

static void _index_node
(
	QueryCtx *ctx,
	Node *n
) {
	uint label_count;
	NODE_GET_LABELS(ctx->gc->g, n, label_count);
	for(uint j = 0; j < label_count; j++) {
		Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[j], SCHEMA_NODE);
		ASSERT(s);

		if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, n);
	}
}

static void _index_edge
(
	QueryCtx *ctx,
	Edge *e
) {
	Schema *s = GraphContext_GetSchemaByID(ctx->gc, e->relationID, SCHEMA_EDGE);
	ASSERT(s);

	if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, e);
}

static void _index_delete_node
(
	QueryCtx *ctx,
	Node *n
) {
	uint label_count;
	NODE_GET_LABELS(ctx->gc->g, n, label_count);
	for(uint j = 0; j < label_count; j++) {
		Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[j], SCHEMA_NODE);
		ASSERT(s);

		// update any indices this entity is represented in
		Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
		if(idx) Index_RemoveNode(idx, n);

		idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
		if(idx) Index_RemoveNode(idx, n);
	}
}

static void _index_delete_edge
(
	QueryCtx *ctx,
	Edge *e
) {
	Schema *s = GraphContext_GetSchemaByID(ctx->gc, e->relationID, SCHEMA_EDGE);
	ASSERT(s);

	// update any indices this entity is represented in
	Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
	if(idx) Index_RemoveEdge(idx, e);

	idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
	if(idx) Index_RemoveEdge(idx, e);
}

// rollback the updates taken place by current query
static void _UndoLog_Rollback_Update_Entity
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		UndoOp *op = undo_list + i;
		Graph_UpdateEntity(op->update_op.ge, op->update_op.attr_id,
			op->update_op.orig_value, op->update_op.entity_type);

		// update indices
		if(op->update_op.entity_type == GETYPE_NODE) {
			Node *n = (Node *)op->update_op.ge;
			_index_node(ctx, n);
		} else {
			Edge *e = (Edge *)op->update_op.ge;
			_index_edge(ctx, e);
		}
	}
}

// undo node creation
static void _UndoLog_Rollback_Create_Node
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		Node *n = &undo_list[i].create_op.n;
		Graph_DeleteNode(ctx->gc->g, n);
		_index_delete_node(ctx, n);
	}
}

// undo edge creation
static void _UndoLog_Rollback_Create_Edge
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		Edge *e = &undo_list[i].create_op.e;
		Graph_DeleteEdge(ctx->gc->g, e);
		_index_delete_edge(ctx, e);
	}
}

// undo node deletion
static void _UndoLog_Rollback_Delete_Node
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		UndoOp *op = undo_list + i;
		Node n;
		Graph_CreateNode(ctx->gc->g, &n, op->delete_node_op.labels,
				op->delete_node_op.label_count);
		*n.attributes = op->delete_node_op.set;

		// re-introduce node to indices
		_index_node(ctx, &n);
	}
}

// undo edge deletion
static void _UndoLog_Rollback_Delete_Edge
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		UndoOp *op = undo_list + i;
		Edge e;
		Graph_CreateEdge(ctx->gc->g, op->delete_edge_op.srcNodeID,
			op->delete_edge_op.destNodeID, op->delete_edge_op.relationID, &e);
		*e.attributes = op->delete_edge_op.set;

		_index_edge(ctx, &e);
	}
}

UndoLog UndoLog_New(void) {
	return (UndoLog)array_new(UndoOp, 0);
}

//------------------------------------------------------------------------------
// Undo add changes
//------------------------------------------------------------------------------

// undo node creation
void UndoLog_CreateNode
(
	UndoLog *log,
	Node node             // node created
) {
	ASSERT(log != NULL && *log != NULL);

	UndoOp op;

	op.type        = UNDO_CREATE_NODE;
	op.create_op.n = node;

	array_append(*log, op);
}

// undo edge creation
void UndoLog_CreateEdge
(
	UndoLog *log,
	Edge edge             // edge created
) {
	ASSERT(log != NULL && *log != NULL);

	UndoOp op;

	op.type        = UNDO_CREATE_EDGE;
	op.create_op.e = edge;

	array_append(*log, op);
}

// undo node deletion
void UndoLog_DeleteNode
(
	UndoLog *log,      // undo log
	Node *node         // node deleted
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(node != NULL);

	UndoOp op;

	op.type                        =  UNDO_DELETE_NODE;
	op.delete_node_op.id           =  node->id;
	op.delete_node_op.set          =  AttributeSet_Clone(*node->attributes);

	Graph *g = QueryCtx_GetGraph();
	NODE_GET_LABELS(g, node, op.delete_node_op.label_count);
	op.delete_node_op.labels = rm_malloc(sizeof(LabelID) * op.delete_node_op.label_count);
	for (uint i = 0; i < op.delete_node_op.label_count; i++) {
		op.delete_node_op.labels[i] = labels[i];
	}

	array_append(*log, op);
}

// undo edge deletion
void UndoLog_DeleteEdge
(
	UndoLog *log,  // undo log
	Edge *edge      // edge deleted
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(edge != NULL);

	UndoOp op;

	op.type                       = UNDO_DELETE_EDGE;
	op.delete_edge_op.id          = edge->id;
	op.delete_edge_op.relationID  = edge->relationID;
	op.delete_edge_op.srcNodeID   = edge->srcNodeID;
	op.delete_edge_op.destNodeID  = edge->destNodeID;
	op.delete_edge_op.set         = AttributeSet_Clone(*edge->attributes);

	array_append(*log, op);
}

// undo entity update
void UndoLog_UpdateEntity
(
	UndoLog *log,                // undo log
	GraphEntity *ge,             // updated entity
	Attribute_ID attr_id,        // updated attribute ID
	SIValue orig_value,          // attribute original value
	GraphEntityType entity_type  // entity type
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(ge != NULL);
	ASSERT(attr_id != ATTRIBUTE_ID_NONE && attr_id != ATTRIBUTE_ID_ALL);

	UndoOp op;

	op.type                  = UNDO_UPDATE;
	op.update_op.ge          = ge;
	op.update_op.attr_id     = attr_id;
	op.update_op.orig_value  = SI_CloneValue(orig_value);
	op.update_op.entity_type = entity_type;

	array_append(*log, op);
}
//------------------------------------------------------------------------------
// rollback
//------------------------------------------------------------------------------

void UndoLog_Rollback
(
	UndoLog log
) {
	ASSERT(log != NULL);
	
	QueryCtx *ctx  = QueryCtx_GetQueryCtx();
	uint64_t count = array_len(log);

	if(count == 0) return;

	// apply undo operations in reverse order for rollback correctness
	// find sequences of the same operation and rollback them as a bulk
	int seq_end = count - 1;
	while (seq_end >= 0) {
		UndoOpType cur_type = log[seq_end].type;
		int seq_start = seq_end;
		seq_end--;
		while(seq_end > 0 && log[seq_end].type == cur_type) {
			seq_end--;
		}

		switch(cur_type) {
			case UNDO_UPDATE:
				_UndoLog_Rollback_Update_Entity(ctx, seq_start, seq_end);
				break;
			case UNDO_CREATE_NODE:
				_UndoLog_Rollback_Create_Node(ctx, seq_start, seq_end);
				break;
			case UNDO_CREATE_EDGE:
				_UndoLog_Rollback_Create_Edge(ctx, seq_start, seq_end);
				break;
			case UNDO_DELETE_NODE:
				_UndoLog_Rollback_Delete_Node(ctx, seq_start, seq_end);
				break;
			case UNDO_DELETE_EDGE:
				_UndoLog_Rollback_Delete_Edge(ctx, seq_start, seq_end);
				break;
			default:
				ASSERT(false);
		}
 	}

	array_clear(log);
}

void UndoLog_Free
(
	UndoLog log
) {
	// free each undo operation
	uint count = array_len(log);
	for (uint i = 0; i < count; i++) {
		UndoOp *op = log + i;
		switch(op->type) {
			case UNDO_UPDATE:
				SIValue_Free(op->update_op.orig_value);
				break;
			case UNDO_CREATE_NODE:
				break;
			case UNDO_CREATE_EDGE:
				break;
			case UNDO_DELETE_NODE:
				rm_free(op->delete_node_op.labels);
				AttributeSet_Free(&op->delete_node_op.set);
				break;
			case UNDO_DELETE_EDGE:
				AttributeSet_Free(&op->delete_edge_op.set);
				break;
			default:
				ASSERT(false);
		}
	}

	array_free(log);
}
