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

// introduce a node creation change to undo log
void UndoLog_AddCreateNode
(
	UndoLog *undo_log,     // undo log to append creations to
	Node *node             // node created
) {
	ASSERT(undo_log != NULL);
	ASSERT(node != NULL);

	UndoOp op = {.n = node, .type = UL_CREATE_NODE};
	array_append(undo_log->undo_list, op);
}

// introduce an edge creation change to undo log
void UndoLog_AddCreateEdge
(
	UndoLog *undo_log,     // undo log to append creations to
	Edge *edge             // edge created
) {
	ASSERT(undo_log != NULL);
	ASSERT(edge != NULL);

	UndoOp op = {.e = edge, .type = UL_CREATE_EDGE};
	array_append(undo_log->undo_list, op);
}

// introduce a node deletion change to undo log
void UndoLog_AddDeleteNode
(
	UndoLog *undo_log,     // undo log to append creations to
	Node *node             // node deleted
) {
	ASSERT(undo_log != NULL);
	ASSERT(node != NULL);

	UndoOp op = {.n = node, .type = UL_DELETE_NODE};
	array_append(undo_log->undo_list, op);
}

// introduce an edge deletion change to undo log
void UndoLog_AddDeleteEdge
(
	UndoLog *undo_log,     // undo log to append creations to
	Edge *edge             // edge deleted
) {
	ASSERT(undo_log != NULL);
	ASSERT(edge != NULL);

	UndoOp op = {.e = edge, .type = UL_DELETE_EDGE};
	array_append(undo_log->undo_list, op);
}

// adds to the undo_log the undo for the pending_update
void UndoLog_AddUpdate
(
	UndoLog *undo_log,
	const PendingUpdateCtx *pending_update,
	const SIValue *orig_value   // the original value which pending_update is about to override
) {
	ASSERT(undo_log != NULL);
	ASSERT(orig_value != NULL);
	ASSERT(pending_update != NULL);

	UndoOp op = {.update = *pending_update, .type = UL_UPDATE};
	op.update.new_value  =  *orig_value;
	array_append(undo_log->undo_list, op);
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
	DeleteEntities(&op, true);

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
	DeleteEntities(&op, true);

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
