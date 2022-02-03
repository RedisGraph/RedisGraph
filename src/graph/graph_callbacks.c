/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_callbacks.h"
#include "../query_ctx.h"
#include "../undo_log/undo_log.h"

static void _GraphCallbacks_NOP_NodeCreated(const Graph *g, Node *n) {}
static void _GraphCallbacks_NOP_EdgeCreated(const Graph *g, Edge *e) {}
static void _GraphCallbacks_NOP_NodeDeleted(const Graph *g, Node *n, LabelID *labels, uint label_count) {}
static void _GraphCallbacks_NOP_EdgeDeleted(const Graph *g, Edge *e) {}
static void _GraphCallbacks_NOP_NodeUpdated(const Graph *g, Node *n, Attribute_ID attr_id, SIValue *orig_value) {}
static void _GraphCallbacks_NOP_EdgeUpdated(const Graph *g, Edge *e, Attribute_ID attr_id, SIValue *orig_value) {}

// add operation to the undo log
static void _add_undo_op(UndoOp *op) {
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	array_append(query_ctx->undo_log.undo_list, *op);
}

static void _GraphCallbacks_UNDO_NodeCreated(const Graph *g, Node *n) {
	// add node creation operation to undo log
	UndoOp op;
	UndoLog_CreateNode(&op, *n);
	_add_undo_op(&op);
}

static void _GraphCallbacks_UNDO_EdgeCreated(const Graph *g, Edge *e) {
	// add edge creation operation to undo log
	UndoOp op;
	UndoLog_CreateEdge(&op, *e);
	_add_undo_op(&op);
}

static void _GraphCallbacks_UNDO_NodeDeleted(const Graph *g, Node *n, LabelID *labels, uint label_count) {
	// add node deletion operation to undo log
	UndoOp op;
	Node clone;
	LabelID *labels_clone = rm_malloc(sizeof(LabelID) * label_count);
	for (uint i = 0; i < label_count; i++) {
		labels_clone[i] = labels[i];
	}
	
	Node_Clone(n, &clone);
	UndoLog_DeleteNode(&op, clone, labels_clone, label_count);
	_add_undo_op(&op);
}

static void _GraphCallbacks_UNDO_EdgeDeleted(const Graph *g, Edge *e) {
	// add edge deletion operation to undo log
	UndoOp op;
	Edge clone;
	Edge_Clone(e, &clone);
	UndoLog_DeleteEdge(&op, clone);
	_add_undo_op(&op);
}

static void _GraphCallbacks_UNDO_NodeUpdated(const Graph *g, Node *n, Attribute_ID attr_id, SIValue *orig_value) {
	// add node update operation to undo log
	UndoOp op;
	SIValue clone = SI_CloneValue(*orig_value);
	UndoLog_UpdateNode(&op, n, attr_id, clone);
	_add_undo_op(&op);
}

static void _GraphCallbacks_UNDO_EdgeUpdated(const Graph *g, Edge *e, Attribute_ID attr_id, SIValue *orig_value) {
	// add edge update operation to undo log
	UndoOp op;
	SIValue clone = SI_CloneValue(*orig_value);
	UndoLog_UpdateEdge(&op, e, attr_id, clone);
	_add_undo_op(&op);
}

void Graph_SetCallbacks
(
	Graph *g,
	GRAPH_CALLBACKS_TYPE type
) {
	switch(type) {
		case GRAPH_CALLBACKS_UNDO:
			g->GraphCallbacks.NodeCreated = _GraphCallbacks_UNDO_NodeCreated;
			g->GraphCallbacks.EdgeCreated = _GraphCallbacks_UNDO_EdgeCreated;
			g->GraphCallbacks.NodeDeleted = _GraphCallbacks_UNDO_NodeDeleted;
			g->GraphCallbacks.EdgeDeleted = _GraphCallbacks_UNDO_EdgeDeleted;
			g->GraphCallbacks.NodeUpdated = _GraphCallbacks_UNDO_NodeUpdated;
			g->GraphCallbacks.EdgeUpdated = _GraphCallbacks_UNDO_EdgeUpdated;
			break;
		case GRAPH_CALLBACKS_NOP:
			g->GraphCallbacks.NodeCreated = _GraphCallbacks_NOP_NodeCreated;
			g->GraphCallbacks.EdgeCreated = _GraphCallbacks_NOP_EdgeCreated;
			g->GraphCallbacks.NodeDeleted = _GraphCallbacks_NOP_NodeDeleted;
			g->GraphCallbacks.EdgeDeleted = _GraphCallbacks_NOP_EdgeDeleted;
			g->GraphCallbacks.NodeUpdated = _GraphCallbacks_NOP_NodeUpdated;
			g->GraphCallbacks.EdgeUpdated = _GraphCallbacks_NOP_EdgeUpdated;
			break;
		default:
			ASSERT(false);
	}
}
