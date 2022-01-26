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
	Node *node             // node created
) {
	ASSERT(op != NULL);
	ASSERT(node != NULL);

	op->n    = node;
	op->type = UL_CREATE_NODE;
}

// create edge creation operation
void UndoLog_CreateEdge
(
	UndoOp *op,
	Edge *edge             // edge created
) {
	ASSERT(op != NULL);
	ASSERT(edge != NULL);

	op->e    = edge;
	op->type = UL_CREATE_EDGE;
}

// create node deletion operation
void UndoLog_DeleteNode
(
	UndoOp *op,
	Node *node             // node deleted
) {
	ASSERT(op != NULL);
	ASSERT(node != NULL);

	op->n    = node;
	op->type = UL_DELETE_NODE;
}

// create edge deletion operation
void UndoLog_DeleteEdge
(
	UndoOp *op,
	Edge *edge             // edge deleted
) {
	ASSERT(op != NULL);
	ASSERT(edge != NULL);

	op->e    = edge;
	op->type = UL_DELETE_EDGE;
}

// create update operation
void UndoLog_Update
(
	UndoOp *op,
	const PendingUpdateCtx *pending_update,
	const SIValue *orig_value   // the original value which pending_update is about to override
) {
	ASSERT(op != NULL);
	ASSERT(orig_value != NULL);
	ASSERT(pending_update != NULL);

	op->update            = *pending_update;
	op->update.new_value  = *orig_value;
	op->type              = UL_UPDATE;
}

// rollback the updates taken place by current query.
static void _UndoLog_Rollback_Updates(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1; 
	PendingUpdateCtx *updates = array_newlen(PendingUpdateCtx, len);
	for(int i = 0; i < len; ++i) {
		updates[i] = undo_list[seq_start + i].update;
	}

	CommitUpdates(ctx->gc, QueryCtx_GetResultSetStatistics(), updates, ENTITY_NODE);

	array_free(updates);
}

// Deletes all nodes that have been created by the query
static void _UndoLog_Rollback_Create_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	Node *nodes_to_delete = array_newlen(Node, len);
	for(int i = 0; i < len; ++i) {
		nodes_to_delete[i] = *undo_list[seq_start + i].n;
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
		edges_to_delete[i] = *undo_list[seq_start + i].e;
	}

	OpDelete op = { .gc = ctx->gc, .deleted_nodes = NULL, .deleted_edges = edges_to_delete, .stats = QueryCtx_GetResultSetStatistics()};
	DeleteEntities(&op);

	array_free(edges_to_delete);
}

static void _UndoLog_Rollback_Delete_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	PendingCreations pc = NewPendingCreationsContainer(NULL, NULL);
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	Node *nodes_to_delete = array_newlen(Node, len);
	for(int i = 0; i < len; ++i) {
		array_append(pc.created_nodes, undo_list[seq_start + i].n);
	}
	CommitNewEntities(NULL, &pc);
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
			case UL_UPDATE:
				_UndoLog_Rollback_Updates(ctx, i, seq_end);
				break;
			case UL_CREATE_NODE:
				_UndoLog_Rollback_Create_Node(ctx, i, seq_end);
				break;
			case UL_CREATE_EDGE:
				_UndoLog_Rollback_Create_Edge(ctx, i, seq_end);
				break;
			case UL_DELETE_NODE:
				// _UndoLog_Rollback_Delete_Node(ctx, i, seq_end);
				break;
			case UL_DELETE_EDGE:
				break;
			default:
				ASSERT(false);           
		}

		i = seq_end;
	}
}

void UndoLog_Free(UndoLog *undo_log) {
	array_free(undo_log->undo_list);
	undo_log->undo_list = NULL;
}
