/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "undo_log.h"
#include "query_ctx.h"
#include "../execution_plan/ops/shared/update_functions.h"
#include "../execution_plan/ops/shared/create_functions.h"
#include "../graph/entities/attribute_set.h"

static void _index_node
(
	QueryCtx *ctx,
	Node *n
) {
	uint label_count;
	NODE_GET_LABELS(ctx->gc->g, n, label_count);
	for(uint j = 0; j < label_count; j++) {
		Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[j], SCHEMA_NODE);
		ASSERT(s);

		if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, n);
	}
}

static void _index_node_with_labels
(
	QueryCtx *ctx,
	Node *n,
	int *labels,
	uint label_count
) {
	for(uint i = 0; i < label_count; i++) {
		Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[i], SCHEMA_NODE);
		ASSERT(s != NULL);

		if(Schema_HasIndices(s)) {
			Schema_AddNodeToIndices(s, n);
		}
	}
}

static void _index_edge
(
	QueryCtx *ctx,
	Edge *e
) {
	Schema *s = GraphContext_GetSchemaByID(ctx->gc, e->relationID, SCHEMA_EDGE);
	ASSERT(s);

	if(Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, e);
}

static void _index_delete_node_with_labels
(
	QueryCtx *ctx,
	Node *n,
	int *labels,
	uint label_count
) {
	for(uint i = 0; i < label_count; i++) {
		Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[i], SCHEMA_NODE);
		ASSERT(s != NULL);

		// update any indices this entity is represented in
		Schema_RemoveNodeFromIndices(s, n);
	}
}

static void _index_delete_node
(
	QueryCtx *ctx,
	Node *n
) {
	uint label_count;
	NODE_GET_LABELS(ctx->gc->g, n, label_count);
	for(uint j = 0; j < label_count; j++) {
		Schema *s = GraphContext_GetSchemaByID(ctx->gc, labels[j], SCHEMA_NODE);
		ASSERT(s);

		// update any indices this entity is represented in
		Schema_RemoveNodeFromIndices(s, n);
	}
}

static void _index_delete_edge
(
	QueryCtx *ctx,
	Edge *e
) {
	Schema *s = GraphContext_GetSchemaByID(ctx->gc, e->relationID, SCHEMA_EDGE);
	ASSERT(s);

	// update any indices this entity is represented in
	Schema_RemoveEdgeFromIndices(s, e);
}

static void _UndoLog_Restore_Entity_Property
(
	GraphEntity *ge,
	Attribute_ID attr_id,
	SIValue value
) {
	// try to get current attribute value
	SIValue *old_value = GraphEntity_GetProperty(ge, attr_id);

	if(old_value == ATTRIBUTE_NOTFOUND) {
		// adding a new attribute; do nothing if its value is NULL
		if(SI_TYPE(value) != T_NULL) {
			AttributeSet_AddNoClone(ge->attributes, attr_id, value);
		}
	} else {
		// update attribute
		AttributeSet_UpdateNoClone(ge->attributes, attr_id, value);
	}
}

// rollback the updates taken place by current query
static void _UndoLog_Rollback_Update_Entity
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		UndoOp *op = undo_list + i;
		UndoUpdateOp *update_op = &op->update_op;

		// update indices
		if(update_op->entity_type == GETYPE_NODE) {
			_UndoLog_Restore_Entity_Property((GraphEntity *)&update_op->n, update_op->attr_id,
				update_op->orig_value);
			_index_node(ctx, &update_op->n);
		} else {
			_UndoLog_Restore_Entity_Property((GraphEntity *)&update_op->e, update_op->attr_id,
				update_op->orig_value);
			_index_edge(ctx, &update_op->e);
		}
	}
}

static void _UndoLog_Rollback_Set_Labels
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		Graph        *g                = QueryCtx_GetGraph();
		UndoOp       *op               = undo_list + i;
		UndoLabelsOp *update_labels_op = &(op->labels_op);
		uint         labels_count      = update_labels_op->labels_count;

		Graph_RemoveNodeLabels(g, update_labels_op->node.id,
				update_labels_op->label_lds, labels_count);
				
		_index_delete_node_with_labels(ctx, &(update_labels_op->node),
				update_labels_op->label_lds, labels_count);

		array_free(update_labels_op->label_lds);
	}
}

static void _UndoLog_Rollback_Remove_Labels
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		Graph        *g                = QueryCtx_GetGraph();
		UndoOp       *op               = undo_list + i;
		UndoLabelsOp *update_labels_op = &(op->labels_op);
		uint         labels_count      = update_labels_op->labels_count;

		Graph_LabelNode(g, update_labels_op->node.id, 
				update_labels_op->label_lds, labels_count);

		_index_node_with_labels(ctx, &(update_labels_op->node),
				update_labels_op->label_lds, labels_count);

		array_free(update_labels_op->label_lds);
	}
}

// undo node creation
static void _UndoLog_Rollback_Create_Node
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		Node *n = &undo_list[i].create_op.n;
		_index_delete_node(ctx, n);
		Graph_DeleteNode(ctx->gc->g, n);
	}
}

// undo edge creation
static void _UndoLog_Rollback_Create_Edge
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	Edge *edges = array_new(Edge, seq_start - seq_end);
	for(int i = seq_start; i > seq_end; --i) {
		Edge *e = &undo_list[i].create_op.e;
		_index_delete_edge(ctx, e);
		array_append(edges, *e);
	}
	Graph_DeleteEdges(ctx->gc->g, edges);
	array_free(edges);
}

// undo node deletion
static void _UndoLog_Rollback_Delete_Node
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		Node n;
		UndoOp *op = undo_list + i;
		UndoDeleteNodeOp *delete_op = &(op->delete_node_op);

		Graph_CreateNode(ctx->gc->g, &n, delete_op->labels,
				delete_op->label_count);
		*n.attributes = delete_op->set;

		// re-introduce node to indices
		_index_node(ctx, &n);
		// Cleanup after undo rollback, as the op D'tor is not called.
		rm_free(delete_op->labels);
	}
}

// undo edge deletion
static void _UndoLog_Rollback_Delete_Edge
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		Edge e;
		UndoOp *op = undo_list + i;
		UndoDeleteEdgeOp delete_op = op->delete_edge_op;

		Graph_CreateEdge(ctx->gc->g, delete_op.srcNodeID, delete_op.destNodeID,
				delete_op.relationID, &e);
		*e.attributes = delete_op.set;

		_index_edge(ctx, &e);
	}
}

// undo schema addition
static void _UndoLog_Rollback_Add_Schema
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		Edge e;
		UndoOp *op = undo_list + i;
		UndoAddSchemaOp schema_op = op->schema_op;
		int schema_id = schema_op.schema_id;
		int schema_count = GraphContext_SchemaCount(ctx->gc, schema_op.t);
		ASSERT(schema_id == schema_count - 1);
		GraphContext_RemoveSchema(ctx->gc, schema_id, schema_op.t);
		if(schema_op.t == SCHEMA_NODE) {
			Graph_RemoveLabel(ctx->gc->g, schema_id);
		} else {
			Graph_RemoveRelation(ctx->gc->g, schema_id);
		}	
	}
}

// undo attribute addition
static void _UndoLog_Rollback_Add_Attribute
(
	QueryCtx *ctx,
	int seq_start,
	int seq_end
) {
	UndoOp *undo_list = ctx->undo_log;
	for(int i = seq_start; i > seq_end; --i) {
		Edge e;
		UndoOp *op = undo_list + i;
		UndoAddAttributeOp attribute_op = op->attribute_op;
		int attribute_id = attribute_op.attribute_id;
		GraphContext_RemoveAttribute(ctx->gc, attribute_id);	
	}
}

// add an operation to undo log
static inline void _UndoLog_AddOperation
(
	UndoLog *log,  // undo log
	UndoOp *op     // undo operation
) {
	ASSERT(op != NULL);
	ASSERT(log != NULL && *log != NULL);

	array_append(*log, *op);
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
	Node *node             // node created
) {
	ASSERT(log != NULL && *log != NULL);

	UndoOp op;

	op.type        = UNDO_CREATE_NODE;
	op.create_op.n = *node;

	_UndoLog_AddOperation(log, &op);
}

// undo edge creation
void UndoLog_CreateEdge
(
	UndoLog *log,
	Edge *edge             // edge created
) {
	ASSERT(log != NULL && *log != NULL);

	UndoOp op;

	op.type        = UNDO_CREATE_EDGE;
	op.create_op.e = *edge;

	_UndoLog_AddOperation(log, &op);
}

// undo node deletion
void UndoLog_DeleteNode
(
	UndoLog *log,      // undo log
	Node *node         // node deleted
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(node != NULL);

	UndoOp op;

	op.type                        =  UNDO_DELETE_NODE;
	op.delete_node_op.id           =  node->id;
	op.delete_node_op.set          =  AttributeSet_Clone(*node->attributes);

	Graph *g = QueryCtx_GetGraph();
	NODE_GET_LABELS(g, node, op.delete_node_op.label_count);
	op.delete_node_op.labels = rm_malloc(sizeof(LabelID) * op.delete_node_op.label_count);
	for (uint i = 0; i < op.delete_node_op.label_count; i++) {
		op.delete_node_op.labels[i] = labels[i];
	}

	_UndoLog_AddOperation(log, &op);
}

// undo edge deletion
void UndoLog_DeleteEdge
(
	UndoLog *log,  // undo log
	Edge *edge     // edge deleted
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(edge != NULL);

	UndoOp op;

	op.type                       = UNDO_DELETE_EDGE;
	op.delete_edge_op.id          = edge->id;
	op.delete_edge_op.relationID  = edge->relationID;
	op.delete_edge_op.srcNodeID   = edge->srcNodeID;
	op.delete_edge_op.destNodeID  = edge->destNodeID;
	op.delete_edge_op.set         = AttributeSet_Clone(*edge->attributes);

	_UndoLog_AddOperation(log, &op);
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

	op.type                  = UNDO_UPDATE;
	op.update_op.attr_id     = attr_id;
	op.update_op.orig_value  = SI_CloneValue(orig_value);
	op.update_op.entity_type = entity_type;

	if(entity_type == GETYPE_NODE) {
		op.update_op.n = *(Node *)ge;
	} else {
		op.update_op.e = *(Edge *)ge;
	}

	_UndoLog_AddOperation(log, &op);
}

// undo node add label
void UndoLog_AddLabels
(
	UndoLog *log,                // undo log
	Node *node,                  // updated node
	int *label_ids,              // added labels
	size_t labels_count          // number of removed labels
) {
	ASSERT(node != NULL);
	ASSERT(label_ids != NULL);

	UndoOp op;

	op.type = UNDO_SET_LABELS;
	op.labels_op.node = *node;
	op.labels_op.label_lds = array_new(int, labels_count);
	memcpy(op.labels_op.label_lds, label_ids, sizeof(int)*labels_count);
	op.labels_op.labels_count = labels_count;
	_UndoLog_AddOperation(log, &op);
}

// undo node remove label
void UndoLog_RemoveLabels
(
	UndoLog *log,                // undo log
	Node *node,                  // updated node
	int *label_ids,              // removed labels
	size_t labels_count          // number of removed labels
) {
	ASSERT(node != NULL);
	ASSERT(label_ids != NULL);

	UndoOp op;

	op.type = UNDO_REMOVE_LABELS;
	op.labels_op.node = *node;
	op.labels_op.label_lds = array_new(int, labels_count);
	memcpy(op.labels_op.label_lds, label_ids, sizeof(int)*labels_count);
	op.labels_op.labels_count = labels_count;
	_UndoLog_AddOperation(log, &op);
}

// undo schema addition
void UndoLog_AddSchema
(
	UndoLog *log,                // undo log
	int schema_id,               // id of the schema
	SchemaType t                 // type of the schema
) {
	ASSERT(log != NULL);
	UndoOp op;

	op.type = UNDO_ADD_SCHEMA;
	op.schema_op.schema_id = schema_id;
	op.schema_op.t = t;
	_UndoLog_AddOperation(log, &op);
}

void UndoLog_AddAttribute
(
	UndoLog *log,                // undo log
	Attribute_ID attribute_id             // id of the attribute
) {
	ASSERT(log != NULL);
	UndoOp op;

	op.type = UNDO_ADD_ATTRIBUTE;
	op.attribute_op.attribute_id = attribute_id;
	_UndoLog_AddOperation(log, &op);
}


//------------------------------------------------------------------------------
// rollback
//------------------------------------------------------------------------------

void UndoLog_Rollback
(
	UndoLog log
) {
	ASSERT(log != NULL);
	
	QueryCtx *ctx  = QueryCtx_GetQueryCtx();
	uint64_t count = array_len(log);

	if(count == 0) return;

	// apply undo operations in reverse order for rollback correctness
	// find sequences of the same operation and rollback them as a bulk
	int seq_end = count - 1;
	while (seq_end >= 0) {
		UndoOpType cur_type = log[seq_end].type;
		int seq_start = seq_end;
		seq_end--;
		while(seq_end > 0 && log[seq_end].type == cur_type) {
			seq_end--;
		}

		switch(cur_type) {
			case UNDO_UPDATE:
				_UndoLog_Rollback_Update_Entity(ctx, seq_start, seq_end);
				break;
			case UNDO_CREATE_NODE:
				_UndoLog_Rollback_Create_Node(ctx, seq_start, seq_end);
				break;
			case UNDO_CREATE_EDGE:
				_UndoLog_Rollback_Create_Edge(ctx, seq_start, seq_end);
				break;
			case UNDO_DELETE_NODE:
				_UndoLog_Rollback_Delete_Node(ctx, seq_start, seq_end);
				break;
			case UNDO_DELETE_EDGE:
				_UndoLog_Rollback_Delete_Edge(ctx, seq_start, seq_end);
				break;
			case UNDO_SET_LABELS:
				_UndoLog_Rollback_Set_Labels(ctx, seq_start, seq_end);
				break;
			case UNDO_REMOVE_LABELS:
				_UndoLog_Rollback_Remove_Labels(ctx, seq_start, seq_end);
				break;
			case UNDO_ADD_SCHEMA:
				_UndoLog_Rollback_Add_Schema(ctx, seq_start, seq_end);
				break;
			case UNDO_ADD_ATTRIBUTE:
				_UndoLog_Rollback_Add_Attribute(ctx, seq_start, seq_end);
				break;
			default:
				ASSERT(false);
		}
 	}

	array_clear(log);

}

void UndoLog_Free
(
	UndoLog log
) {
	// free each undo operation
	uint count = array_len(log);
	for (uint i = 0; i < count; i++) {
		UndoOp *op = log + i;
		switch(op->type) {
			case UNDO_UPDATE:
				SIValue_Free(op->update_op.orig_value);
				break;
			case UNDO_CREATE_NODE:
				break;
			case UNDO_CREATE_EDGE:
				break;
			case UNDO_DELETE_NODE:
				rm_free(op->delete_node_op.labels);
				AttributeSet_Free(&op->delete_node_op.set);
				break;
			case UNDO_DELETE_EDGE:
				AttributeSet_Free(&op->delete_edge_op.set);
				break;
			case UNDO_SET_LABELS:
			case UNDO_REMOVE_LABELS:
				array_free(op->labels_op.label_lds);
				break;
			case UNDO_ADD_SCHEMA:
			case UNDO_ADD_ATTRIBUTE:
				break;
			default:
				ASSERT(false);
		}
	}

	array_free(log);
}
