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

// rollback the updates taken place by current query
static void _UndoLog_Rollback_Update_Entity
(
	QueryCtx *ctx,
	size_t seq_start,
	size_t seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	size_t len = seq_end - seq_start + 1; 
	for(int i = seq_end; i < len; --i) {
		UndoOp *op = undo_list + seq_start + i;
		Graph_UpdateEntity(op->update_op.ge, op->update_op.attr_id,
			op->update_op.orig_value, op->update_op.entity_type);

		// update indices
		if(op->update_op.entity_type == GETYPE_NODE) {
			Node *n = (Node *)op->update_op.ge;
			uint label_count;
			NODE_GET_LABELS(ctx->gc->g, n, label_count);
			for(uint j = 0; j < label_count; j++) {
				Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[j], SCHEMA_NODE);
				ASSERT(s);

				if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, n);
			}
		} else {
			Edge *e = (Edge *)op->update_op.ge;
			Schema *s = GraphContext_GetSchemaByID(ctx->gc, e->relationID, SCHEMA_EDGE);
			ASSERT(s);

			if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, e);
		}
	}
}

// undo node creation
static void _UndoLog_Rollback_Create_Node
(
	QueryCtx *ctx,
	size_t seq_start,
	size_t seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	size_t len = seq_end - seq_start + 1;
	for(int i = seq_end; i < len; --i) {
		Graph_DeleteNode(ctx->gc->g, &undo_list[seq_start + i].create_op.n);
	}
}

// undo edge creation
static void _UndoLog_Rollback_Create_Edge
(
	QueryCtx *ctx,
	size_t seq_start,
	size_t seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	size_t len = seq_end - seq_start + 1;
	for(int i = seq_end; i < len; --i) {
		Graph_DeleteEdge(ctx->gc->g, &undo_list[seq_start + i].create_op.e);
	}
}

// undo node deletion
static void _UndoLog_Rollback_Delete_Node
(
	QueryCtx *ctx,
	size_t seq_start,
	size_t seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	size_t len = seq_end - seq_start + 1;
	for(int i = seq_end; i < len; --i) {
		UndoOp *op = undo_list + seq_start + i;
		Node n;
		Graph_CreateNode(ctx->gc->g, &n, op->delete_node_op.labels,
				op->delete_node_op.label_count);
		n.attributes = &op->delete_node_op.set;

		// re-introduce node to indices
		for(uint j = 0; j < op->delete_node_op.label_count; j++) {
			Schema *s = GraphContext_GetSchemaByID(ctx->gc,
				op->delete_node_op.labels[j], SCHEMA_NODE);
			ASSERT(s);

			if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, &n);
		}
	}
}

// undo edge deletion
static void _UndoLog_Rollback_Delete_Edge
(
	QueryCtx *ctx,
	size_t seq_start,
	size_t seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	size_t len = seq_end - seq_start + 1;
	for(int i = seq_end; i < len; --i) {
		UndoOp *op = undo_list + seq_start + i;
		Edge e;
		Graph_CreateEdge(ctx->gc->g, op->delete_edge_op.srcNodeID,
			op->delete_edge_op.destNodeID, op->delete_edge_op.relationID, &e);
		e.attributes = &op->delete_edge_op.set;

		Schema *s = GraphContext_GetSchemaByID(ctx->gc, op->delete_edge_op.relationID, SCHEMA_EDGE);
		ASSERT(s);

		// re-introduce edge to index
		if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, &e);
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
	UndoLog *log,       // undo log
	Node *node,         // node deleted
	LabelID *labelIDs,  // node labels
	uint label_count    // node label count
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(node != NULL);

	UndoOp op;

	op.type                        =  UNDO_DELETE_NODE;
	op.delete_node_op.id           =  node->id;
	op.delete_node_op.set          =  AttributeSet_Clone(*node->attributes);
	op.delete_node_op.labels       =  labelIDs;
	op.delete_node_op.label_count  =  label_count;

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
	op.update_op.orig_value  = orig_value;
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
	QueryCtx *ctx = QueryCtx_GetQueryCtx();
	ASSERT(ctx != NULL);
	ASSERT(log != NULL);

	uint64_t count = array_len(log);

	if(count == 0) return;

	// apply undo operations in reverse order for rollback correctness
	// find sequences of operation of the same type and rollback them as a bulk
	uint i = count - 1;
	while (i > 0) {
		UndoOpType cur_type = log[i].type;
		uint seq_end = i;
		while(i > 0 && log[i - 1].type == cur_type) {
			i--;
		}

		switch(cur_type) {
			case UNDO_UPDATE:
				_UndoLog_Rollback_Update_Entity(ctx, i, seq_end);
				break;
			case UNDO_CREATE_NODE:
				_UndoLog_Rollback_Create_Node(ctx, i, seq_end);
				break;
			case UNDO_CREATE_EDGE:
				_UndoLog_Rollback_Create_Edge(ctx, i, seq_end);
				break;
			case UNDO_DELETE_NODE:
				_UndoLog_Rollback_Delete_Node(ctx, i, seq_end);
				break;
			case UNDO_DELETE_EDGE:
				_UndoLog_Rollback_Delete_Edge(ctx, i, seq_end);
				break;
			default:
				ASSERT(false);
		}
 
		if(i > 0) i--;
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
		if (op->type == UNDO_DELETE_NODE) {
			rm_free(op->delete_node_op.labels);
			AttributeSet_Free(&op->delete_node_op.set);
		} else if (op->type == UNDO_DELETE_EDGE) {
			AttributeSet_Free(&op->delete_edge_op.set);
		}
	}

	array_free(log);
}
