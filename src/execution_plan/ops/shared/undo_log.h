/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdint.h>
#include "../../src/value.h"
#include "update_functions.h"

typedef enum {
    UL_UPDATE = 0,
    UL_CREATE_NODE,
    UL_CREATE_EDGE,
    UL_DELETE_NODE,
    UL_DELETE_EDGE
} UndoLog_OpType;

typedef struct Undo_Op {
    UndoLog_OpType type;
    union {
        Node n;
        Edge e;
        PendingUpdateCtx update;
    };
} Undo_Op;

typedef struct UndoLogCtx {
    bool is_valid;                    // Are the fields allocated and need to be freed.
    Undo_Op *undo_log;                // Undo log (Array), used in the case of timeout or any other situation rollback is needed.
    uint64_t n_ops_commited;          // Number of commits which needs rollback.
} UndoLogCtx;

// Allocate and init the UndoLogCtx structure.
UndoLogCtx UndoLog_New();

// Deallocates the UndoLogCtx structure.
void UndoLog_Free(UndoLogCtx *undo_log_ctx);

// Increments the counter which counts the number of commited updates.
static inline void UndoLog_Inc_N_Ops_Commited(UndoLogCtx *undo_log_ctx, uint64_t n_ops) {
    undo_log_ctx->n_ops_commited += n_ops;
}

// Append updates to the undo log.
void UndoLog_Add_Update(UndoLogCtx *undo_log_ctx, const PendingUpdateCtx *pending_update, const SIValue *orig_value);

// Append arrays of newly created nodes and edges to the undo log.
void UndoLog_Add_Create(UndoLogCtx *undo_log_ctx, Node **created_nodes, Edge **created_edges);

// Rollback all modifications made by the query execution to the graph or index.
// This function is being called when timeout or other exception took place and the execution plan already drained
void UndoLog_Rollback(void);