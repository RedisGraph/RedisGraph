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

void UndoLog_New(UndoLog *undolog) {
	ASSERT(undolog != NULL);
	undolog->undo_list       =  array_new(UndoOp, 0);
}

//------------------------------------------------------------------------------
// Undo add changes
//------------------------------------------------------------------------------

// create node creation operation
void UndoLog_CreateNode
(
	UndoOp *op,
	Node node             // node created
) {
	ASSERT(op != NULL);

	op->create_op.n = node;
	op->type        = UL_CREATE_NODE;
}

// create edge creation operation
void UndoLog_CreateEdge
(
	UndoOp *op,
	Edge edge             // edge created
) {
	ASSERT(op != NULL);

	op->create_op.e = edge;
	op->type        = UL_CREATE_EDGE;
}

// create node deletion operation
void UndoLog_DeleteNode
(
	UndoOp *op,
	Node node,             // node deleted
	LabelID *labelIDs,
	uint label_count
) {
	ASSERT(op != NULL);

	op->delete_op.n           = node;
	op->delete_op.labels      = labelIDs;
	op->delete_op.label_count = label_count;
	op->type                  = UL_DELETE_NODE;
}

// create edge deletion operation
void UndoLog_DeleteEdge
(
	UndoOp *op,
	Edge edge             // edge deleted
) {
	ASSERT(op != NULL);

	op->delete_op.e = edge;
	op->type        = UL_DELETE_EDGE;
}

// create entity update operation
void UndoLog_UpdateEntity
(
	UndoOp *op,
	GraphEntity *ge,
	Attribute_ID attr_id,
	SIValue orig_value,   // the original value which pending_update is about to override
	GraphEntityType entity_type
) {
	ASSERT(op != NULL);
	ASSERT(ge != NULL);

	op->update_op.ge          = ge;
	op->update_op.entity_type = entity_type;
	op->update_op.attr_id     = attr_id;
	op->update_op.orig_value  = orig_value;
	op->type                  = UL_UPDATE;
}

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

// Deletes all nodes that have been created by the query
static void _UndoLog_Rollback_Create_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	for(int i = 0; i < len; ++i) {
		Graph_DeleteNode(ctx->gc->g, &undo_list[seq_start + i].create_op.n);
	}
}

// Deletes all edges that have been created by the query
static void _UndoLog_Rollback_Create_Edge(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	for(int i = 0; i < len; ++i) {
		Graph_DeleteEdge(ctx->gc->g, &undo_list[seq_start + i].create_op.e);
	}
}

static void _UndoLog_Rollback_Delete_Node(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
	size_t len = seq_end - seq_start + 1;
	for(int i = 0; i < len; ++i) {
		UndoOp *op = undo_list + seq_start + i;
		Node new;
		Graph_CreateNode(ctx->gc->g, &new, op->delete_op.labels, op->delete_op.label_count);
		new.attributes->attr_count = op->delete_op.n.attributes->attr_count;
		new.attributes->attributes = op->delete_op.n.attributes->attributes;
		
		for(uint j = 0; j < op->delete_op.label_count; j++) {
			Schema *s = GraphContext_GetSchemaByID(ctx->gc,
				op->delete_op.labels[j], SCHEMA_NODE);
			ASSERT(s);

			if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, &new);
		}
	}
}

static void _UndoLog_Rollback_Delete_Edge(QueryCtx *ctx, size_t seq_start, size_t seq_end) {
	UndoOp *undo_list = ctx->undo_log.undo_list;
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

		if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, &new);
	}
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

static void UndoOp_Free
(
	UndoOp *op
) {
	switch(op->type) {
		case UL_UPDATE:
			break;
		case UL_CREATE_NODE:
			break;
		case UL_CREATE_EDGE:
			break;
		case UL_DELETE_NODE:
			rm_free(op->delete_op.labels);
			AttributeSet_Free(op->delete_op.n.attributes);
			break;
		case UL_DELETE_EDGE:
			AttributeSet_Free(op->delete_op.e.attributes);
			break;
		default:
			ASSERT(false);           
	}
}

void UndoLog_Free(UndoLog *undo_log) {
	uint count = array_len(undo_log->undo_list);
	for (uint i = 0; i < count; i++) {
		UndoOp_Free(undo_log->undo_list + i);
	}
	array_free(undo_log->undo_list);
	undo_log->undo_list = NULL;
}
