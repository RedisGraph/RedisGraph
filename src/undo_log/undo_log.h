/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdint.h>
#include "../../src/value.h"
#include "../execution_plan/ops/shared/update_functions.h"

/*
* This file contains the rollback functionality:
* When a query timeout or gets an exception all it's commited data should be rollbacked.
* For this reason the UndoLog accumulates the commited changes of the query and if needed also
* being called to roll them back.
*/

// The type of operation of each item in the undo_list
typedef enum {
    UL_UPDATE = 0,   // undo update
    UL_CREATE_NODE,  // undo node creation
    UL_CREATE_EDGE,  // undo edge creation
    UL_DELETE_NODE,  // undo node deletion
    UL_DELETE_EDGE   // undo edge deletion
} UndoOpType;

// The abstract type of items in the undo_list
typedef struct UndoOp {
    union {
        Node n;
        Edge e;
        PendingUpdateCtx update;
    };
    UndoOpType type;
} UndoOp;

// Container for undo_list and the index of the last commited op in the list.
typedef struct UndoLog {
    UndoOp *undo_list;         // list of undo operations
    uint64_t n_commited_ops;   // number of commited changes
} UndoLog;

// allocate and init the UndoLog
void UndoLog_New
(
	UndoLog *undo_log
);

// update last committed change index
static inline void UndoLog_UpdateCommitted
(
	UndoLog *undo_log,
	uint64_t n
) {
    ASSERT(undo_log != NULL);
    undo_log->n_commited_ops += n;

	// make sure we're not overflowing
	ASSERT(array_len(undo_log->undo_list) >= undo_log->n_commited_ops);
}

//------------------------------------------------------------------------------
// Undo add changes
//------------------------------------------------------------------------------

// introduce an update change to undo log
void UndoLog_AddUpdate
(
	UndoLog *undo_log,                       // undo log to append update to
	const PendingUpdateCtx *pending_update,  // update context
	const SIValue *orig_value                // original value prior to update
);

// introduce a creation change to undo log
void UndoLog_AddCreate
(
	UndoLog *undo_log,     // undo log to append creations to
	Node **created_nodes,  // list of node creation operations
	Edge **created_edges   // list of edge creation operations
);

// introduce a deletion change to undo log
void UndoLog_AddDelete(UndoLog *undo_log, Node *deleted_nodes, Edge *deleted_edges);

// rollback all modifications tracked by this undo log
void UndoLog_Rollback
(
	UndoLog *undo_log
);

// deallocates the UndoLog
void UndoLog_Free
(
	UndoLog *undo_log
);

