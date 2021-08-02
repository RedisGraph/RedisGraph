/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "undo_log.h"
#include "query_ctx.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../execution_plan/ops/op_delete.h" // TODO: delete
#include "../execution_plan/ops/shared/update_functions.h"

void UndoLog_New(UndoLog *undolog) {
    /*
	ASSERT(undolog != NULL);
	undolog->undo_list       =  array_new(UndoOp, 0);
	undolog->n_commited_ops  =  0;
*/
}

//------------------------------------------------------------------------------
// Undo add changes
//------------------------------------------------------------------------------

// Adds to the undo_log the undo for the pending_update
// orig_value: The original value which pending_update is about to override
void UndoLog_AddUpdate
(
	UndoLog *undo_log,
	const PendingUpdateCtx *pending_update,
	const SIValue *orig_value
) {
	ASSERT(undo_log != NULL);
	ASSERT(orig_value != NULL);
	ASSERT(pending_update != NULL);

	// The undo is equal to pending_update except for the new value
    UndoOp *op = array_ensure_at(&undo_log->undo_list, array_len(undo_log->undo_list), UndoOp);
	op->type              =  UL_UPDATE;
	op->update            =  *pending_update;
	op->update.new_value  =  *orig_value;
}

static inline void _CreateNodeOp_To_UndoOp(UndoOp *op, Node **n) {
    op->type = UL_CREATE_NODE;
    op->n = **n;
}

static inline void _CreateEdgeOp_To_UndoOp(UndoOp *op, Edge **e) {
    op->type = UL_CREATE_EDGE;
    op->e = **e;
}

/**
 * Appends elements to the end of the array and initialize them using cb function
 * @param arrpp array pointer.
 * @param src array (i.e. C array) of elements to append
 * @param n length of sec
 * @param init_fn cb function used for item initialization
 * @return the array
 */
#define array_ensure_append_with_init_func(arrpp, src, n, init_fn, T)                \
  ({                                                                                 \
    size_t a__oldlen = array_len(arrpp);                                             \
    arrpp = (T *)array_grow(arrpp, n);                                               \
    for(size_t i = 0; i < (n); i++) {                                                \
        T *v = &arrpp[a__oldlen + i];                                                \
        init_fn(v, &(src)[i]);                                                       \
    }                                                                                \
    arrpp;                                                                           \
  })

// Adds to the undo_log the undo for the pending creation of entities.
void UndoLog_AddCreate
(
	UndoLog *undo_log,
	Node **created_nodes,
	Edge **created_edges
) {
	ASSERT(undo_log != NULL);
	ASSERT(created_nodes != NULL);
	ASSERT(created_edges != NULL);

	if(array_len(created_edges) > 0) {
		array_ensure_append_with_init_func(
				undo_log->undo_list, created_edges, array_len(created_edges),
				_CreateEdgeOp_To_UndoOp, UndoOp);
	}

	if(array_len(created_nodes) > 0) {
		array_ensure_append_with_init_func(
				undo_log->undo_list, created_nodes, array_len(created_nodes),
				_CreateNodeOp_To_UndoOp, UndoOp);
	}
}

// Rollback the updates taken place by current query.
static void _UndoLog_Rollback_Updates(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
    UndoOp *undo_list = ctx->undo_log.undo_list;
    size_t len = seq_end - seq_start + 1; 
    PendingUpdateCtx *updates = array_newlen(PendingUpdateCtx, len);
    for(int i = 0; i < len; ++i) {
        updates[i] = undo_list[seq_start + i].update;
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

// Deletes all nodes that have been created by the query
static void _UndoLog_Rollback_Create_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
    UndoOp *undo_list = ctx->undo_log.undo_list;
    size_t len = seq_end - seq_start + 1;
    Node *nodes_to_delete = array_newlen(Node, len);
    for(int i = 0; i < len; ++i) {
        nodes_to_delete[i] = undo_list[seq_start + i].n;
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
        edges_to_delete[i] = undo_list[seq_start + i].e;
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

static inline void _DeleteNodeOp_To_UndoOp(UndoOp *op, Node *n) {
    // Note that each entity is going to assign the same id after the 
    // rollback since the blocks ids are stored on a stack.
    op->n = *n;
    if(n->entity != NULL) {
        op->n.entity = rm_calloc(sizeof(Entity), 1);
        _deep_copy_entity(op->n.entity, n->entity);
    }

    op->type = UL_DELETE_NODE;
}

static inline void _DeleteEdgeOp_To_UndoOp(UndoOp *op, Edge *e) {
    // Note that each entity is going to assign the same id after the 
    // rollback since the blocks ids are stored on a stack.
    op->e = *e;
    if(e->entity != NULL) {
        op->e.entity = rm_calloc(sizeof(Entity), 1);
        _deep_copy_entity(op->e.entity, e->entity);
    }

    op->type = UL_DELETE_EDGE;
}

// Adds to the undo_log the undo for the pending deletion of entities.
void UndoLog_AddDelete(UndoLog *undo_log, Node *deleted_nodes, Edge *deleted_edges) {
    //DataBlock_GetItem(g->nodes, ENTITY_GET_ID(n));
    array_ensure_append_with_init_func(undo_log->undo_list, deleted_nodes,
    array_len(deleted_nodes), _DeleteNodeOp_To_UndoOp, UndoOp);
    array_ensure_append_with_init_func(undo_log->undo_list, deleted_edges,
    array_len(deleted_edges), _DeleteEdgeOp_To_UndoOp, UndoOp);
}
/*
// Creates all edges that have been deleted by the query
static void _UndoLog_Rollback_Create_Edge(UndoLog *undo_list, size_t seq_start, size_t seq_end) {
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
*/

// check if both operation rollback delete of an entity
static inline bool __UndoLog_OpType_Are_Both_Delete(UndoOpType op1, UndoOpType op2) {
    return op1 >= UL_DELETE_NODE && op2 >= UL_DELETE_NODE;
}

void UndoLog_Rollback(UndoLog *undo_log) {
    QueryCtx *ctx = QueryCtx_GetQueryCtx();
    ASSERT(ctx);
    ASSERT(undo_log);
    uint64_t n_commited_ops = undo_log->n_commited_ops;
    if(n_commited_ops == 0) return;

    // We like to rollback only the commited ops so removing the uncommitted ones.
    // Please note that n_commited_ops is inc before an update is being commited so we might
    // Rollback update that haven't being commited and it's ok because the undo updates are idempotent.
    undo_log->undo_list = array_trimm(undo_log->undo_list, undo_log->n_commited_ops, ARR_CAP_NOSHRINK);

    // Reverse the undo_log since we need to commit the ops in reverse order for rollback correctness
    array_reverse(undo_log->undo_list);

    // Find sequences of operation of the same type and rollback them as a bulk
    size_t seq_start = 0, seq_end;
    UndoOp *undo_list = undo_log->undo_list;
    while(seq_start < n_commited_ops) {
        UndoOpType cur_type = undo_list[seq_start].type;
        seq_end = seq_start;
        while((seq_end + 1) < n_commited_ops && 
        (undo_list[seq_end + 1].type == cur_type || __UndoLog_OpType_Are_Both_Delete(undo_list[seq_end + 1].type, cur_type))) {
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

void UndoLog_Free(UndoLog *undo_log) {
    /*
    array_free(undo_log->undo_list);
    undo_log->n_commited_ops = 0;
    undo_log->undo_list = NULL;
    */
}
