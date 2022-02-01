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

	op->entity.n = node;
	op->type     = UL_CREATE_NODE;
}

// create edge creation operation
void UndoLog_CreateEdge
(
	UndoOp *op,
	Edge edge             // edge created
) {
	ASSERT(op != NULL);

	op->entity.e = edge;
	op->type     = UL_CREATE_EDGE;
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

	op->entity.n                = node;
	op->data.delete.labels      = labelIDs;
	op->data.delete.label_count = label_count;
	op->type                    = UL_DELETE_NODE;
}

// create edge deletion operation
void UndoLog_DeleteEdge
(
	UndoOp *op,
	Edge edge             // edge deleted
) {
	ASSERT(op != NULL);

	op->entity.e = edge;
	op->type     = UL_DELETE_EDGE;
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

	op->entity.n               = *n;
	op->data.update.attr_id    = attr_id;
	op->data.update.orig_value = orig_value;
	op->type                   = UL_UPDATE_NODE;
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

	op->entity.e               = *e;
	op->data.update.attr_id    = attr_id;
	op->data.update.orig_value = orig_value;
	op->type                   = UL_UPDATE_EDGE;
}

// rollback the updates taken place by current query.
static void _UndoLog_Rollback_Update_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1; 
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Graph_UpdateNode(ctx->gc->g, &op->entity.n, op->data.update.attr_id, op->data.update.orig_value);

		uint label_count;
		NODE_GET_LABELS(ctx->gc->g, &op->entity.n, label_count);
		for(uint j = 0; j < label_count; j++) {
			Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[j], SCHEMA_NODE);
			ASSERT(s);

			if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, &op->entity.n);
		}
	}
}

// rollback the updates taken place by current query.
static void _UndoLog_Rollback_Update_Edge(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1; 
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Graph_UpdateEdge(ctx->gc->g, &op->entity.e, op->data.update.attr_id, op->data.update.orig_value);

		Schema *s = GraphContext_GetSchemaByID(ctx->gc, op->entity.e.relationID, SCHEMA_EDGE);
		ASSERT(s);

		if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, &op->entity.e);
	}
}

// Deletes all nodes that have been created by the query
static void _UndoLog_Rollback_Create_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	Node *nodes_to_delete = array_newlen(Node, len);
	for(int i = 0; i < len; ++i) {
		nodes_to_delete[i] = undo_list[seq_start + i].entity.n;
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
		edges_to_delete[i] = undo_list[seq_start + i].entity.e;
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
		Graph_CreateNode(ctx->gc->g, &new, op->data.delete.labels, op->data.delete.label_count);
		new.entity->prop_count = op->entity.n.entity->prop_count;
		new.entity->properties = op->entity.n.entity->properties;
		
		for(uint j = 0; j < op->data.delete.label_count; j++) {
			Schema *s = GraphContext_GetSchemaByID(ctx->gc, op->data.delete.labels[j], SCHEMA_NODE);
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
		Graph_CreateEdge(ctx->gc->g, op->entity.e.srcNodeID, op->entity.e.destNodeID, op->entity.e.relationID, &new);
		new.entity->prop_count = op->entity.e.entity->prop_count;
		new.entity->properties = op->entity.e.entity->properties;

		Schema *s = GraphContext_GetSchemaByID(ctx->gc, op->entity.e.relationID, SCHEMA_EDGE);
		ASSERT(s);

		if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, &new);
	}
}

void UndoLog_Rollback(UndoLog *undo_log) {
	QueryCtx *ctx = QueryCtx_GetQueryCtx();
	ASSERT(ctx);
	ASSERT(undo_log);
	Graph_SetCrudHubPolicy(ctx->gc->g, CRUD_POLICY_NOP);

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

void UndoOp_Free
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
			rm_free(op->data.delete.labels);
			rm_free(op->entity.n.entity->properties);
			rm_free(op->entity.n.entity);
			break;
		case UL_DELETE_EDGE:
			rm_free(op->entity.e.entity->properties);
			rm_free(op->entity.e.entity);
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
