/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "undo_log.h"
#include "query_ctx.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../execution_plan/ops/op_delete.h"
#include "../execution_plan/ops/shared/update_functions.h"
#include "../execution_plan/ops/shared/create_functions.h"

void UndoLog_New(UndoLog *undolog) {
	ASSERT(undolog != NULL);
	undolog->undo_list       =  array_new(UndoOp, 0);
}

//------------------------------------------------------------------------------
// Undo add changes
//------------------------------------------------------------------------------

// create node creation operation
void UndoLog_CreateNode
(
	UndoOp *op,
	Node node             // node created
) {
	ASSERT(op != NULL);

	op->create_op.n = node;
	op->type        = UL_CREATE_NODE;
}

// create edge creation operation
void UndoLog_CreateEdge
(
	UndoOp *op,
	Edge edge             // edge created
) {
	ASSERT(op != NULL);

	op->create_op.e = edge;
	op->type        = UL_CREATE_EDGE;
}

// create node deletion operation
void UndoLog_DeleteNode
(
	UndoOp *op,
	Node node,             // node deleted
	LabelID *labelIDs,
	uint label_count
) {
	ASSERT(op != NULL);

	op->delete_op.n           = node;
	op->delete_op.labels      = labelIDs;
	op->delete_op.label_count = label_count;
	op->type                  = UL_DELETE_NODE;
}

// create edge deletion operation
void UndoLog_DeleteEdge
(
	UndoOp *op,
	Edge edge             // edge deleted
) {
	ASSERT(op != NULL);

	op->delete_op.e = edge;
	op->type        = UL_DELETE_EDGE;
}

// create node update operation
void UndoLog_UpdateNode
(
	UndoOp *op,
	Node *n,
	Attribute_ID attr_id,
	SIValue orig_value   // the original value which pending_update is about to override
) {
	ASSERT(op != NULL);
	ASSERT(n != NULL);

	op->update_op.n          = *n;
	op->update_op.attr_id    = attr_id;
	op->update_op.orig_value = orig_value;
	op->type                 = UL_UPDATE_NODE;
}

// create edge update operation
void UndoLog_UpdateEdge
(
	UndoOp *op,
	Edge *e,
	Attribute_ID attr_id,
	SIValue orig_value   // the original value which pending_update is about to override
) {
	ASSERT(op != NULL);
	ASSERT(e != NULL);

	op->update_op.e          = *e;
	op->update_op.attr_id    = attr_id;
	op->update_op.orig_value = orig_value;
	op->type                 = UL_UPDATE_EDGE;
}

// rollback the updates taken place by current query.
static void _UndoLog_Rollback_Update_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1; 
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Graph_UpdateEntity(ctx->gc->g, (GraphEntity *)&op->update_op.n,
			op->update_op.attr_id, op->update_op.orig_value, GETYPE_NODE);

		uint label_count;
		NODE_GET_LABELS(ctx->gc->g, &op->update_op.n, label_count);
		for(uint j = 0; j < label_count; j++) {
			Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[j], SCHEMA_NODE);
			ASSERT(s);

			if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, &op->update_op.n);
		}
	}
}

// rollback the updates taken place by current query.
static void _UndoLog_Rollback_Update_Edge(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1; 
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Graph_UpdateEntity(ctx->gc->g, (GraphEntity *)&op->update_op.e,
			op->update_op.attr_id, op->update_op.orig_value, GETYPE_EDGE);

		Schema *s = GraphContext_GetSchemaByID(ctx->gc, op->update_op.e.relationID, SCHEMA_EDGE);
		ASSERT(s);

		if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, &op->update_op.e);
	}
}

// Deletes all nodes that have been created by the query
static void _UndoLog_Rollback_Create_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	Node *nodes_to_delete = array_newlen(Node, len);
	for(int i = 0; i < len; ++i) {
		nodes_to_delete[i] = undo_list[seq_start + i].create_op.n;
	}

	OpDelete op = { .gc = ctx->gc, .deleted_nodes = nodes_to_delete, .deleted_edges = NULL, .stats = QueryCtx_GetResultSetStatistics()};
	DeleteEntities(&op);

	array_free(nodes_to_delete);
}

// Deletes all edges that have been created by the query
static void _UndoLog_Rollback_Create_Edge(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	Edge *edges_to_delete = array_newlen(Edge, len);
	for(int i = 0; i < len; ++i) {
		edges_to_delete[i] = undo_list[seq_start + i].create_op.e;
	}

	OpDelete op = { .gc = ctx->gc, .deleted_nodes = NULL, .deleted_edges = edges_to_delete, .stats = QueryCtx_GetResultSetStatistics()};
	DeleteEntities(&op);

	array_free(edges_to_delete);
}

static void _UndoLog_Rollback_Delete_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Node new;
		Graph_CreateNode(ctx->gc->g, &new, op->delete_op.labels, op->delete_op.label_count);
		new.entity->prop_count = op->delete_op.n.entity->prop_count;
		new.entity->properties = op->delete_op.n.entity->properties;
		
		for(uint j = 0; j < op->delete_op.label_count; j++) {
			Schema *s = GraphContext_GetSchemaByID(ctx->gc,
				op->delete_op.labels[j], SCHEMA_NODE);
			ASSERT(s);

			if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, &new);
		}
	}
}

static void _UndoLog_Rollback_Delete_Edge(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Edge new;
		Graph_CreateEdge(ctx->gc->g, op->delete_op.e.srcNodeID,
			op->delete_op.e.destNodeID, op->delete_op.e.relationID, &new);
		new.entity->prop_count = op->delete_op.e.entity->prop_count;
		new.entity->properties = op->delete_op.e.entity->properties;

		Schema *s = GraphContext_GetSchemaByID(ctx->gc, op->delete_op.e.relationID, SCHEMA_EDGE);
		ASSERT(s);

		if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, &new);
	}
}

void UndoLog_Rollback(UndoLog *undo_log) {
	QueryCtx *ctx = QueryCtx_GetQueryCtx();
	ASSERT(ctx);
	ASSERT(undo_log);
	Graph_SetCallbacks(ctx->gc->g, GRAPH_CALLBACKS_NOP);

	uint64_t count = array_len(undo_log->undo_list);

	// Reverse the undo_log since we need to commit the ops in reverse order for rollback correctness
	array_reverse(undo_log->undo_list);

	// Find sequences of operation of the same type and rollback them as a bulk
	UndoOp *undo_list = undo_log->undo_list;
	for (uint i = 0; i < count; i++) {
		UndoOpType cur_type = undo_list[i].type;
		uint seq_end = i;
		while(seq_end + 1 < count && undo_list[seq_end + 1].type == cur_type) {
			seq_end++;
		}

		switch(cur_type) {
			case UL_UPDATE_NODE:
				_UndoLog_Rollback_Update_Node(ctx, i, seq_end);
				break;
			case UL_UPDATE_EDGE:
				_UndoLog_Rollback_Update_Edge(ctx, i, seq_end);
				break;
			case UL_CREATE_NODE:
				_UndoLog_Rollback_Create_Node(ctx, i, seq_end);
				break;
			case UL_CREATE_EDGE:
				_UndoLog_Rollback_Create_Edge(ctx, i, seq_end);
				break;
			case UL_DELETE_NODE:
				_UndoLog_Rollback_Delete_Node(ctx, i, seq_end);
				break;
			case UL_DELETE_EDGE:
				_UndoLog_Rollback_Delete_Edge(ctx, i, seq_end);
				break;
			default:
				ASSERT(false);           
		}

		i = seq_end;
	}

	array_clear(undo_list);
}

static void UndoOp_Free
(
	UndoOp *op
) {
	switch(op->type) {
		case UL_UPDATE_NODE:
			break;
		case UL_UPDATE_EDGE:
			break;
		case UL_CREATE_NODE:
			break;
		case UL_CREATE_EDGE:
			break;
		case UL_DELETE_NODE:
			rm_free(op->delete_op.labels);
			GraphEntity_Free(op->delete_op.n.entity);
			break;
		case UL_DELETE_EDGE:
			GraphEntity_Free(op->delete_op.e.entity);
			break;
		default:
			ASSERT(false);           
	}
}

void UndoLog_Free(UndoLog *undo_log) {
	uint count = array_len(undo_log->undo_list);
	for (uint i = 0; i < count; i++) {
		UndoOp_Free(undo_log->undo_list + i);
	}
	array_free(undo_log->undo_list);
	undo_log->undo_list = NULL;
}
