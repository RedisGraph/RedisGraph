/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdint.h>
#include "../../src/value.h"
#include "../execution_plan/ops/shared/update_functions.h"

// this file contains the rollback functionality:
// when a query finished in timeout or exception
// all it's commited data should be rollbacked.
// the UndoLog accumulates the commited changes of the query.


// the type of operation of each item in the undo_list
typedef enum {
	UL_UPDATE = 0,   // undo update
	UL_CREATE_NODE,  // undo node creation
	UL_CREATE_EDGE,  // undo edge creation
	UL_DELETE_NODE,  // undo node deletion
	UL_DELETE_EDGE   // undo edge deletion
} UndoOpType;

// the abstract type of items in the undo_list
typedef struct UndoOp {
	union {
		Node *n;
		Edge *e;
		PendingUpdateCtx update;
	};
	UndoOpType type;
} UndoOp;

// container for undo_list and the index of the last commited op in the list.
typedef struct UndoLog {
	UndoOp *undo_list;         // list of undo operations
} UndoLog;

// allocate and init the UndoLog
void UndoLog_New
(
	UndoLog *undo_log
);

//------------------------------------------------------------------------------
// Undo add changes
//------------------------------------------------------------------------------

// introduce a node creation change to undo log
void UndoLog_AddCreateNode
(
	UndoLog *undo_log,     // undo log to append creations to
	Node *node             // node created
);

// introduce an edge creation change to undo log
void UndoLog_AddCreateEdge
(
	UndoLog *undo_log,     // undo log to append creations to
	Edge *edge             // edge created
);

// introduce a node deletion change to undo log
void UndoLog_AddDeleteNode
(
	UndoLog *undo_log,     // undo log to append creations to
	Node *node             // node deleted
);

// introduce an edge deletion change to undo log
void UndoLog_AddDeleteEdge
(
	UndoLog *undo_log,     // undo log to append creations to
	Edge *edge             // edge deleted
);

// introduce an update change to undo log
void UndoLog_AddUpdate
(
	UndoLog *undo_log,                       // undo log to append update to
	const PendingUpdateCtx *pending_update,  // update context
	const SIValue *orig_value                // original value prior to update
);

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
