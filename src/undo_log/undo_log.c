/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "undo_log.h"
#include "query_ctx.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../execution_plan/ops/shared/update_functions.h"
#include "../execution_plan/ops/shared/create_functions.h"

// rollback the updates taken place by current query.
static void _UndoLog_Rollback_Update_Entity(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1; 
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Graph_UpdateEntity(op->update_op.ge, op->update_op.attr_id,
			op->update_op.orig_value, op->update_op.entity_type);

		if(op->update_op.entity_type == GETYPE_NODE) {
			Node *n = (Node *)op->update_op.ge;
			uint label_count;
			NODE_GET_LABELS(ctx->gc->g, n, label_count);
			for(uint j = 0; j < label_count; j++) {
				Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[j], SCHEMA_NODE);
				ASSERT(s);

				if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, n);
			}
		} else {
			Edge *e = (Edge *)op->update_op.ge;
			Schema *s = GraphContext_GetSchemaByID(ctx->gc, e->relationID, SCHEMA_EDGE);
			ASSERT(s);

			if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, e);
		}
	}
}

// undo node creation
static void _UndoLog_Rollback_Create_Node
(
	QueryCtx *ctx,
	size_t seq_start,
	size_t seq_end
) {
	UndoOp *undo_list = ctx->undo_log
	size_t len = seq_end - seq_start + 1;
	for(int i = 0; i < len; ++i) {
		Graph_DeleteNode(ctx->gc->g, &undo_list[seq_start + i].create_op.n);
	}
}

// undo edge creation
static void _UndoLog_Rollback_Create_Edge
(
	QueryCtx *ctx,
	size_t seq_start,
	size_t seq_end
) {
	UndoOp undo_list = ctx->undo_log;
	size_t len = seq_end - seq_start + 1;
	for(int i = 0; i < len; ++i) {
		Graph_DeleteEdge(ctx->gc->g, &undo_list[seq_start + i].create_op.e);
	}
}

// undo node deletion
static void _UndoLog_Rollback_Delete_Node
(
	QueryCtx *ctx,
	size_t seq_start,
	size_t seq_end
) {
	UndoOp undo_list = ctx->undo_log;
	size_t len = seq_end - seq_start + 1;
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Node new;
		Graph_CreateNode(ctx->gc->g, &new, op->delete_op.labels, op->delete_op.label_count);
		new.attributes->attr_count = op->delete_op.n.attributes->attr_count;
		new.attributes->attributes = op->delete_op.n.attributes->attributes;
		
		// re-introduce node to indices
		for(uint j = 0; j < op->delete_op.label_count; j++) {
			Schema *s = GraphContext_GetSchemaByID(ctx->gc,
				op->delete_op.labels[j], SCHEMA_NODE);
			ASSERT(s);

			if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, &new);
		}
	}
}

// undo edge deletion
static void _UndoLog_Rollback_Delete_Edge
(
	QueryCtx *ctx,
	size_t seq_start,
	size_t seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	size_t len = seq_end - seq_start + 1;
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Edge new;
		Graph_CreateEdge(ctx->gc->g, op->delete_op.e.srcNodeID,
			op->delete_op.e.destNodeID, op->delete_op.e.relationID, &new);
		new.attributes->attr_count = op->delete_op.e.attributes->attr_count;
		new.attributes->attributes = op->delete_op.e.attributes->attributes;

		Schema *s = GraphContext_GetSchemaByID(ctx->gc, op->delete_op.e.relationID, SCHEMA_EDGE);
		ASSERT(s);

		// re-introduce edge to index
		if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, &new);
	}
}

UndoLog UndoLog_New(void) {
	return (UndoLog)array_new(UndoOp, 0);
}

//------------------------------------------------------------------------------
// Undo add changes
//------------------------------------------------------------------------------

// undo node creation
void UndoLog_CreateNode
(
	UndoLog *log,
	Node node             // node created
) {
	ASSERT(op != NULL && *op != NULL);

	UndoOp op;

	op.type        = UNDO_CREATE_NODE;
	op.create_op.n = node;

	*log = array_append(*log, op);
}

// undo edge creation
void UndoLog_CreateEdge
(
	UndoLog *log,
	Edge edge             // edge created
) {
	ASSERT(op != NULL && *op != NULL);

	UndoOp op;

	op.type        = UL_CREATE_EDGE;
	op.create_op.e = edge;

	*log = array_append(*log, op);
}

// undo node deletion
void UndoLog_DeleteNode
(
	UndoLog *log,       // undo log
	Node node,          // node deleted
	LabelID *labelIDs,  // node labels
	uint label_count    // node label count
) {
	ASSERT(log != NULL && *log != NULL);

	UndoOp op;

	op.type                   =  UL_DELETE_NODE;
	op.delete_op.n            =  node;
	op.delete_op.labels       =  labelIDs;
	op.delete_op.label_count  =  label_count;

	*log = array_append(*log, op);
}

// undo edge deletion
void UndoLog_DeleteEdge
(
	UndoLog *log,  // undo log
	Edge edge      // edge deleted
) {
	ASSERT(op != NULL && *op != NULL);

	UndoOp op;

	op.type        = UL_DELETE_EDGE;
	op.delete_op.e = edge;

	*log = array_append(*log, op);
}

// undo entity update
void UndoLog_UpdateEntity
(
	UndoLog *log,                // undo log
	GraphEntity *ge,             // updated entity
	Attribute_ID attr_id,        // updated attribute ID
	SIValue orig_value,          // attribute original value
	GraphEntityType entity_type  // entity type
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(ge != NULL);
	ASSERT(attr_id != ATTRIBUTE_ID_NONE && attr_id != ATTRIBUTE_ID_ALL);

	UndoOp op;

	op.type                  = UL_UPDATE;
	op.update_op.ge          = ge;
	op.update_op.attr_id     = attr_id;
	op.update_op.orig_value  = orig_value;
	op.update_op.entity_type = entity_type;

	*log = array_append(*log, op);
}
//------------------------------------------------------------------------------
// rollback
//------------------------------------------------------------------------------

void UndoLog_Rollback
(
	UndoLog log
) {
	QueryCtx *ctx = QueryCtx_GetQueryCtx();
	ASSERT(ctx != NULL);
	ASSERT(log != NULL);

	uint64_t count = array_len(undo_list);

	// apply undo operations is reverse order for rollback correctness
	// Find sequences of operation of the same type and rollback them as a bulk
	for (uint i = count-1; i >= 0 ; i--) {
		UndoOpType cur_type = undo_list[i].type;
		uint seq_end = i;
		while(seq_end - 1 >= 0 && undo_list[seq_end - 1].type == cur_type) {
			seq_end--;
		}

		switch(cur_type) {
			case UL_UPDATE:
				_UndoLog_Rollback_Update_Entity(ctx, i, seq_end);
				break;
			case UL_CREATE_NODE:
				_UndoLog_Rollback_Create_Node(ctx, i, seq_end);
				break;
			case UL_CREATE_EDGE:
				_UndoLog_Rollback_Create_Edge(ctx, i, seq_end);
				break;
			case UL_DELETE_NODE:
				_UndoLog_Rollback_Delete_Node(ctx, i, seq_end);
				break;
			case UL_DELETE_EDGE:
				_UndoLog_Rollback_Delete_Edge(ctx, i, seq_end);
				break;
			default:
				ASSERT(false);           
		}

		i = seq_end;
	}

	array_clear(undo_list);
}

void UndoLog_Free
(
	UndoLog *undo_log
) {
	// free each undo operation
	uint count = array_len(undo_log);
	for (uint i = 0; i < count; i++) {
		UndoOp *op = undo_log->undo_list + i;
		if (op->type == UNDO_DELETE_NODE) {
			rm_free(op->delete_op.labels);
			AttributeSet_Free(op->delete_op.n.attributes);
		} else if (op->type == UNDO_DELETE_EDGE) {
			AttributeSet_Free(op->delete_op.e.attributes);
		}
	}

	array_free(undo_log);
	*undo_log = NULL;
}

