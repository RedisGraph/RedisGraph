/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "undo_log.h"
#include "query_ctx.h"
#include "update_functions.h"
#include "../../../execution_plan/ops/op_delete.h"
#include "../../../graph/entities/node.h"
#include "../../../graph/entities/edge.h"

UndoLogCtx UndoLog_New() {
    UndoLogCtx ctx;
    ctx.is_valid = true;
    ctx.undo_log = array_new(Undo_Op, 0);
    ctx.n_ops_commited = 0;
    return ctx;
}

void UndoLog_Free(UndoLogCtx *undo_log_ctx) {
    array_free(undo_log_ctx->undo_log);
    undo_log_ctx->n_ops_commited = 0;
    undo_log_ctx->undo_log = NULL;
    undo_log_ctx->is_valid = false;
}

// Adds to the undo_log the undo for the pending_update
// orig_value: The original value which pending_update is about to override
void UndoLog_Add_Update(UndoLogCtx *undo_log_ctx, const PendingUpdateCtx *pending_update, const SIValue *orig_value) {
    // The undo is equal to pending_update except for the new value
    Undo_Op op;
    op.type = UL_UPDATE;
    op.update = *pending_update;
    op.update.new_value = *orig_value;
    undo_log_ctx->undo_log = array_append(undo_log_ctx->undo_log, op);
}

// Rollback the updates taken place by current query.
static void _UndoLog_Rollback_Updates(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
    const Undo_Op *undo_log = ctx->undo_log_ctx.undo_log;
    size_t len = seq_end - seq_start + 1; 
    PendingUpdateCtx *updates = array_newlen(PendingUpdateCtx, len);
    for(int i = 0; i < len; ++i) {
        updates[i] = undo_log[seq_start + i].update;
    }

    // lock everything
	QueryCtx_LockForCommit();
	{
		CommitUpdates(ctx->gc, QueryCtx_GetResultSetStatistics(), updates, false, NULL);
	}

	// release lock
    // Here the execution plan is already drained so undo can pretend it's the last writer
	QueryCtx_UnlockCommit(ctx->internal_exec_ctx.last_writer);

    array_free(updates);
}

static inline Undo_Op _CreateNodeOp_To_UndoOp(Node **n) {
    return (Undo_Op){ .type = UL_CREATE_NODE, .n = **n };
}

static inline Undo_Op _CreateEdgeOp_To_UndoOp(Edge **e) {
    return (Undo_Op){ .type = UL_CREATE_EDGE, .e = **e };
}

// Adds to the undo_log the undo for the pending creation of entities.
void UndoLog_Add_Create(UndoLogCtx *ctx, Node **created_nodes, Edge **created_edges) {
    ctx->undo_log = array_ensure_append_with_copy_func(ctx->undo_log, created_nodes,
    array_len(created_nodes), _CreateNodeOp_To_UndoOp, Undo_Op);
    ctx->undo_log = array_ensure_append_with_copy_func(ctx->undo_log, created_edges,
    array_len(created_edges), _CreateEdgeOp_To_UndoOp, Undo_Op);
}

// Deletes all nodes that have been created by the query
static void _UndoLog_Rollback_Create_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
    const Undo_Op *undo_log = ctx->undo_log_ctx.undo_log;
    size_t len = seq_end - seq_start + 1;
    Node *nodes_to_delete = array_newlen(Node, len);
    for(int i = 0; i < len; ++i) {
        nodes_to_delete[i] = undo_log[seq_start + i].n;
    }

    OpDelete op = { .gc = ctx->gc, .deleted_nodes = nodes_to_delete, .deleted_edges = NULL, .stats = QueryCtx_GetResultSetStatistics()};
    DeleteEntities(&op, true);

    array_free(nodes_to_delete);
}

// Deletes all edges that have been created by the query
static void _UndoLog_Rollback_Create_Edge(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
    const Undo_Op *undo_log = ctx->undo_log_ctx.undo_log;
    size_t len = seq_end - seq_start + 1;
    Edge *edges_to_delete = array_newlen(Edge, len);
    for(int i = 0; i < len; ++i) {
        edges_to_delete[i] = undo_log[seq_start + i].e;
    }

    OpDelete op = { .gc = ctx->gc, .deleted_nodes = NULL, .deleted_edges = edges_to_delete, .stats = QueryCtx_GetResultSetStatistics()};
    DeleteEntities(&op, true);

    array_free(edges_to_delete);
}

// Allocates and deep copy the entity
static inline void _deep_copy_entity(Entity *dst, Entity *src) {
    if(src->properties != NULL) {
		dst->properties = rm_malloc(sizeof(EntityProperty) * (src->prop_count));
        memcpy(dst->properties, src->properties, sizeof(EntityProperty) * (src->prop_count));
    }
    dst->prop_count = src->prop_count;
}

static inline Undo_Op _DeleteNodeOp_To_UndoOp(Node *n) {
    // Note that each entity is going to assign the same id after the 
    // rollback since the blocks ids are stored on a stack.
    Node n_copy = *n;
    if(n->entity != NULL) {
        n_copy->entity = rm_calloc(sizeof(Entity));
        _deep_copy_entity(n_copy->entity, n->entity);
    }

    return (Undo_Op){ .type = UL_DELETE_NODE, .n = n_copy };
}

static inline Undo_Op _DeleteEdgeOp_To_UndoOp(Edge *e) {
    // Note that each entity is going to assign the same id after the 
    // rollback since the blocks ids are stored on a stack.
    Edge e_copy = *e;
    if(e->entity != NULL) {
        e_copy->entity = rm_calloc(sizeof(Entity));
        _deep_copy_entity(e_copy->entity, e->entity);
    }

    return (Undo_Op){ .type = UL_DELETE_EDGE, .e = e_copy };
}

// Adds to the undo_log the undo for the pending deletion of entities.
void UndoLog_Add_Delete(UndoLogCtx *ctx, Node *deleted_nodes, Edge *deleted_edges) {
    //DataBlock_GetItem(g->nodes, ENTITY_GET_ID(n));
    ctx->undo_log = array_ensure_append_with_copy_func(ctx->undo_log, deleted_nodes,
    array_len(deleted_nodes), _DeleteNodeOp_To_UndoOp, Undo_Op);
    ctx->undo_log = array_ensure_append_with_copy_func(ctx->undo_log, deleted_edges,
    array_len(deleted_edges), _DeleteEdgeOp_To_UndoOp, Undo_Op);
}

// Creates all edges that have been deleted by the query
static void _UndoLog_Rollback_Create_Edge(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
    const Undo_Op *undo_log = ctx->undo_log_ctx.undo_log;
    size_t len = seq_end - seq_start + 1;
    Edge *edges_to_delete = array_newlen(Edge, len);
    for(int i = 0; i < len; ++i) {
        edges_to_delete[i] = undo_log[seq_start + i].e;
    }

    OpCreate op = { .gc = ctx->gc, .deleted_nodes = NULL, .deleted_edges = edges_to_delete, .stats = QueryCtx_GetResultSetStatistics()};
    DeleteEntities(&op, true);

    array_free(edges_to_delete);
}

// check if both operation rollback delete of an entity
static inline bool __UndoLog_OpType_Are_Both_Delete(UndoLog_OpType op1, UndoLog_OpType op2) {
    return op1 => UL_DELETE_NODE && op2 => UL_DELETE_NODE;
}

void UndoLog_Rollback(void) {
    QueryCtx *ctx = QueryCtx_GetQueryCtx();
    ASSERT(ctx);
    uint64_t n_ops_commited = ctx->undo_log_ctx.n_ops_commited;
    if(n_ops_commited == 0) return;

    // We like to rollback only the commited ops so removing the uncommitted ones.
    // Please note that n_ops_commited is inc before an update is being commited so we might
    // Rollback update that haven't being commited and it's ok because the undo updates are idempotent.
    ctx->undo_log_ctx.undo_log = array_trimm(ctx->undo_log_ctx.undo_log, ctx->undo_log_ctx.n_ops_commited, ARR_CAP_NOSHRINK);

    // Reverse the undo_log since we need to commit the ops in reverse order for rollback correctness
    array_reverse(ctx->undo_log_ctx.undo_log);

    // Find sequences of operation of the same type and rollback them as a bulk
    size_t seq_start = 0, seq_end;
    Undo_Op *undo_log = ctx->undo_log_ctx.undo_log;
    while(seq_start < n_ops_commited) {
        UndoLog_OpType cur_type = undo_log[seq_start].type;
        seq_end = seq_start;
        while((seq_end + 1) < n_ops_commited && 
        (undo_log[seq_end + 1].type == cur_type || __UndoLog_OpType_Are_Both_Delete(undo_log[seq_end + 1].type, cur_type))) {
            seq_end++;
        }

        switch(cur_type) {
            case UL_UPDATE:
                _UndoLog_Rollback_Updates(ctx, seq_start, seq_end);
                break;
            case UL_CREATE_NODE:
                _UndoLog_Rollback_Create_Node(ctx, seq_start, seq_end);
                break;
            case UL_CREATE_EDGE:
                _UndoLog_Rollback_Create_Edge(ctx, seq_start, seq_end);
                break;
            case UL_DELETE_NODE:
            case UL_DELETE_EDGE:
                
                break;
            default:
                ASSERT(false);           
        }

        seq_start = ++seq_end;
    }
}