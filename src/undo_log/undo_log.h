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

// UndoLog
// matains a list of undo operation reverting all changes
// performed by a query: CREATE, UPDATE, DELETE
// 
// upon failure for which ever reason we can apply the
// operations within the undo log to rollback the graph to its
// original state

// UndoLog operation types
typedef enum {
	UNDO_UPDATE = 0,        // undo entity update
	UNDO_CREATE_NODE,       // undo node creation
	UNDO_CREATE_EDGE,       // undo edge creation
	UNDO_DELETE_NODE,       // undo node deletion
	UNDO_DELETE_EDGE        // undo edge deletion
} UndoOpType;

//------------------------------------------------------------------------------
// Undo operations
//------------------------------------------------------------------------------

// undo node/edge creation
typedef struct {
	union {
		Node n;
		Edge e;
	};
} UndoCreateOp;

// undo node deletion
typedef struct {
	EntityID id;
	AttributeSet set;
	LabelID *labels;   // labels attached to deleted entity
	uint label_count;  // number of labels attached to deleted entity
} UndoDeleteNodeOp;

// undo edge deletion
typedef struct {
	EntityID id;
	int relationID;             // Relation ID
	NodeID srcNodeID;           // Source node ID
	NodeID destNodeID;          // Destination node ID
	AttributeSet set;
} UndoDeleteEdgeOp;

// undo graph entity update
typedef struct {
	union {
		Node n;
		Edge e;
	};
	GraphEntityType entity_type;  // node/edge
	Attribute_ID attr_id;         // attribute update
	SIValue orig_value;           // attribute original value
} UndoUpdateOp;

// Undo operation
typedef struct {
	union {
		UndoCreateOp create_op;
		UndoDeleteNodeOp delete_node_op;
		UndoDeleteEdgeOp delete_edge_op;
		UndoUpdateOp update_op;
	};
	UndoOpType type;  // type of undo operation
} UndoOp;

// container for undo_list
typedef UndoOp *UndoLog;

// create a new undo-log
UndoLog UndoLog_New(void);

//------------------------------------------------------------------------------
// UndoLog add operations
//------------------------------------------------------------------------------

// undo node creation
void UndoLog_CreateNode
(
	UndoLog *log,  // undo log
	Node *node     // node created
);

// undo edge creation
void UndoLog_CreateEdge
(
	UndoLog *log,  // undo log
	Edge *edge     // edge created
);

// undo node deletion
void UndoLog_DeleteNode
(
	UndoLog *log,  // undo log
	Node *node     // node deleted
);

// undo edge deletion
void UndoLog_DeleteEdge
(
	UndoLog *log,  // undo log
	Edge *edge     // edge deleted
);

// undo entity update
void UndoLog_UpdateEntity
(
	UndoLog *log,                // undo log
	GraphEntity *ge,             // updated entity
	Attribute_ID attr_id,        // updated attribute ID
	SIValue orig_value,          // attribute original value
	GraphEntityType entity_type  // entity type
);

// rollback all modifications tracked by this undo log
void UndoLog_Rollback
(
	UndoLog log
);

// free UndoLog
void UndoLog_Free
(
	UndoLog log
);

