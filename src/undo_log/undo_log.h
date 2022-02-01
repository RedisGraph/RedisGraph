/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdint.h>
#include "../value.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../execution_plan/ops/shared/update_functions.h"

// this file contains the rollback functionality:
// when a query finished in timeout or exception
// all it's commited data should be rollbacked.
// the UndoLog accumulates the commited changes of the query.


// the type of operation of each item in the undo_list
typedef enum {
	UL_UPDATE_NODE = 0,   // undo node update
	UL_UPDATE_EDGE,       // undo edge update
	UL_CREATE_NODE,       // undo node creation
	UL_CREATE_EDGE,       // undo edge creation
	UL_DELETE_NODE,       // undo node deletion
	UL_DELETE_EDGE        // undo edge deletion
} UndoOpType;

// the abstract type of items in the undo_list
typedef struct UndoOp {
	union {
		Node n;
		Edge e;
	} entity;
	union {
		struct {
			LabelID *labels;
			uint label_count;
		} delete;
		struct {
			Attribute_ID attr_id;
			SIValue orig_value;
		} update;
	} data;
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

// create node creation operation
void UndoLog_CreateNode
(
	UndoOp *op,
	Node node             // node created
);

// create edge creation operation
void UndoLog_CreateEdge
(
	UndoOp *op,
	Edge edge             // edge created
);

// create node deletion operation
void UndoLog_DeleteNode
(
	UndoOp *op,
	Node node,             // node deleted
	LabelID *labelIDs,
	uint label_count
);

// create edge deletion operation
void UndoLog_DeleteEdge
(
	UndoOp *op,
	Edge edge             // edge deleted
);

// create node update operation
void UndoLog_UpdateNode
(
	UndoOp *op,
	Node *n,
	Attribute_ID attr_id,
	SIValue orig_value   // the original value which pending_update is about to override
);

// create edge update operation
void UndoLog_UpdateEdge
(
	UndoOp *op,
	Edge *e,
	Attribute_ID attr_id,
	SIValue orig_value   // the original value which pending_update is about to override
);

// rollback all modifications tracked by this undo log
void UndoLog_Rollback
(
	UndoLog *undo_log
);

void UndoOp_Free
(
	UndoOp *op
);

// deallocates the UndoLog
void UndoLog_Free
(
	UndoLog *undo_log
);

