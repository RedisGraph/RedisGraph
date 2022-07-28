/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_hub.h"
#include "../query_ctx.h"
#include "../undo_log/undo_log.h"

// delete all references to a node from any relevant index
static void _DeleteNodeFromIndices
(
	GraphContext *gc,
	Node *n
) {
	ASSERT(n  != NULL);
	ASSERT(gc != NULL);

	Schema    *s       =  NULL;
	Graph     *g       =  gc->g;
	EntityID  node_id  =  ENTITY_GET_ID(n);

	// retrieve node labels
	uint label_count;
	NODE_GET_LABELS(g, n, label_count);

	for(uint i = 0; i < label_count; i++) {
		int label_id = labels[i];
		s = GraphContext_GetSchemaByID(gc, label_id, SCHEMA_NODE);
		ASSERT(s != NULL);

		// update any indices this entity is represented in
		Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
		if(idx) Index_RemoveNode(idx, n);

		idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
		if(idx) Index_RemoveNode(idx, n);
	}
}

static void _DeleteEdgeFromIndices
(
	GraphContext *gc,
	Edge *e
) {
	Schema  *s  =  NULL;
	Graph   *g  =  gc->g;

	int relation_id = EDGE_GET_RELATION_ID(e, g);

	s = GraphContext_GetSchemaByID(gc, relation_id, SCHEMA_EDGE);

	// update any indices this entity is represented in
	Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
	if(idx) Index_RemoveEdge(idx, e);

	idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
	if(idx) Index_RemoveEdge(idx, e);
}

// add node to any relevant index
static void _AddNodeToIndices
(
	GraphContext *gc,
	Node *n
) {
	ASSERT(n  != NULL);
	ASSERT(gc != NULL);

	Schema    *s       =  NULL;
	Graph     *g       =  gc->g;
	EntityID  node_id  =  ENTITY_GET_ID(n);

	// retrieve node labels
	uint label_count;
	NODE_GET_LABELS(g, n, label_count);

	for(uint i = 0; i < label_count; i++) {
		int label_id = labels[i];
		s = GraphContext_GetSchemaByID(gc, label_id, SCHEMA_NODE);
		ASSERT(s != NULL);
		Schema_AddNodeToIndices(s, n);
	}
}

// add edge to any relevant index
static void _AddEdgeToIndices(GraphContext *gc, Edge *e) {
	Schema  *s  =  NULL;
	Graph   *g  =  gc->g;

	int relation_id = EDGE_GET_RELATION_ID(e, g);

	s = GraphContext_GetSchemaByID(gc, relation_id, SCHEMA_EDGE);
	Schema_AddEdgeToIndices(s, e);
}

// add properties to the GraphEntity
static inline uint _AddProperties
(
	GraphEntity *e,
	const AttributeSet set
) {
	uint updates = 0;
	uint attr_count = ATTRIBUTE_SET_COUNT(set);
	for(int i = 0; i < attr_count; i++) {
		Attribute_ID attr_id;
		SIValue v = AttributeSet_GetIdx(set, i, &attr_id);
		updates += GraphEntity_AddProperty(e, attr_id, v);
	}

	return updates;
}

uint CreateNode
(
	GraphContext *gc,
	Node *n,
	LabelID *labels,
	uint label_count,
	const AttributeSet props
) {
	ASSERT(gc != NULL);
	ASSERT(n != NULL);

	Graph_CreateNode(gc->g, n, labels, label_count);
	uint properties_set = _AddProperties((GraphEntity *)n, props);

	// add node labels
	for(uint i = 0; i < label_count; i++) {
		Schema *s = GraphContext_GetSchemaByID(gc, labels[i], SCHEMA_NODE);
		ASSERT(s);
		Schema_AddNodeToIndices(s, n);
	}

	// add node creation operation to undo log
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_CreateNode(&query_ctx->undo_log, *n);

	return properties_set;
}

uint CreateEdge
(
	GraphContext *gc,
	Edge *e,
	NodeID src,
	NodeID dst,
	int r,
	const AttributeSet props
) {
	ASSERT(gc != NULL);
	ASSERT(e != NULL);

	Graph_CreateEdge(gc->g, src, dst, r, e);
	uint properties_set = _AddProperties((GraphEntity *)e, props);

	Schema *s = GraphContext_GetSchema(gc, e->relationship, SCHEMA_EDGE);
	// all schemas have been created in the edge blueprint loop or earlier
	ASSERT(s != NULL);
	Schema_AddEdgeToIndices(s, e);

	// add edge creation operation to undo log
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_CreateEdge(&query_ctx->undo_log, *e);

	return properties_set;
}

uint DeleteNode
(
	GraphContext *gc,
	Node *n,
	uint *deleted_labels_count,
	uint *deleted_properties_count
) {
	ASSERT(n != NULL);
	ASSERT(gc != NULL);

	// add node deletion operation to undo log	
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_DeleteNode(&query_ctx->undo_log, n);

	if(GraphContext_HasIndices(gc)) {
		_DeleteNodeFromIndices(gc, n);
	}

	if(deleted_labels_count) {
		uint label_count;
		NODE_GET_LABELS(gc->g, n, label_count);
		*deleted_labels_count = label_count;
	}

	if(deleted_properties_count) {
		const AttributeSet attributes = GraphEntity_GetAttributes(n);
		*deleted_properties_count = attributes ? attributes->attr_count : 0;
	}

	Graph_DeleteNode(gc->g, n);

	return 1;
}

int DeleteEdge
(
	GraphContext *gc,
	Edge *e,
	uint *deleted_properties_count
) {
	ASSERT(gc != NULL);
	ASSERT(e != NULL);

	// add edge deletion operation to undo log
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_DeleteEdge(&query_ctx->undo_log, e);

	if(GraphContext_HasIndices(gc)) {
		_DeleteEdgeFromIndices(gc, e);
	}
	const AttributeSet attributes = GraphEntity_GetAttributes(e);
	uint properties_count = attributes ? attributes->attr_count : 0;
	int res = Graph_DeleteEdge(gc->g, e);
	if(res && deleted_properties_count) {
		*deleted_properties_count = properties_count;
	}
	return res;
}

// update entity attributes and update undo log
// in case attr_id is ATTRIBUTE_ID_ALL clear all attributes values
static int _Update_Entity_Property
(
	GraphContext *gc,
	GraphEntity *ge,
	Attribute_ID attr_id,
	SIValue new_value,
	GraphEntityType entity_type
) {
	if(attr_id == ATTRIBUTE_ID_ALL) {
		// we're requested to clear entitiy's attribute-set
		// backup entity's attributes in case we'll need to roolback
		const AttributeSet set = GraphEntity_GetAttributes(ge);
		for(int i = 0; i < ATTRIBUTE_SET_COUNT(set); i++) {
			Attribute_ID id;
			// add entity update operation to undo log
			SIValue value = AttributeSet_GetIdx(set, i, &id);
			QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
			UndoLog_UpdateEntity(&query_ctx->undo_log, ge, id, value, entity_type);
		}
	} else {
		SIValue *orig_value = GraphEntity_GetProperty(ge, attr_id);
		// add entity update operation to undo log
		QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
		UndoLog_UpdateEntity(&query_ctx->undo_log, ge, attr_id, *orig_value, entity_type);
	}

	return Graph_UpdateEntity(ge, attr_id, new_value, entity_type);
}

void UpdateEntityProperties
(
	GraphContext *gc,
	GraphEntity *ge,
	const AttributeSet set,
	GraphEntityType entity_type,
	uint *props_set_count,
	uint *props_removed_count
) {
	ASSERT(gc != NULL);
	ASSERT(ge != NULL);
	int set_props = 0;
	int removed_props = 0;
	for (uint i = 0; i < ATTRIBUTE_SET_COUNT(set); i++) {
		Attribute *prop = set->attributes + i;
		int updates = _Update_Entity_Property(gc, ge, prop->id, prop->value, entity_type);
		if(SIValue_IsNull(prop->value)) {
			removed_props +=updates;
		}
		else {
			set_props += updates;
			// When overwriting an exiting property it is considered also as removed.
			if(GraphEntity_GetProperty(ge, prop->id) != ATTRIBUTE_NOTFOUND) {
				removed_props +=updates;
			}
		}
	}
	if(entity_type == GETYPE_NODE) {
		_AddNodeToIndices(gc, (Node *)ge);
	} else {
		_AddEdgeToIndices(gc, (Edge *)ge);
	}
	if(props_set_count) *props_set_count = set_props;
	if(props_removed_count) *props_removed_count = removed_props;
}

void UpdateNodeLabels
(
	GraphContext *gc,            // graph context to update the entity
	Node *node,                  // the node to be updated
	rax *labels,     	         // labels to update
	uint *labels_added_count,    // number of labels added (out param)
	uint *labels_removed_count   // number of labels removed (out param)
) {
	ASSERT(gc != NULL);
	ASSERT(node != NULL);

	if(!labels) return;
	uint label_count = raxSize(labels);
	if(label_count == 0) return;

	int *add_labels = array_new(int, label_count);
	int *remove_labels = array_new(int, label_count);
	raxIterator it;
	raxStart(&it, labels);
	// Iterate over all keys in the rax.
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		// Get label string.
		unsigned char *raw_label = it.key;
		// Avoid rax not null terminating strings
		size_t len = it.key_len;
		char label[len];
		memcpy(label, raw_label, len);
		label[len] = 0;

		if(it.data) {
			// Get or create label matrix
			const Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
			if(s == NULL) {
				s = GraphContext_AddSchema(gc, label, SCHEMA_NODE);
			}

			int schema_id = Schema_GetID(s);
			bool exists = Graph_IsNodeLabeled(gc->g, node->id, schema_id);
			if(!exists) {
				// sync matrix, make sure label matrix is of the right dimensions
				RG_Matrix m = Graph_GetLabelMatrix(gc->g, schema_id);

				// Append label id.
				array_append(add_labels, schema_id);
				// Add to index.
				Schema_AddNodeToIndices(s, node);
			}
			
		} else {
			// Get or create label matrix
			const Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
			if(s == NULL) continue;

			// sync matrix, make sure label matrix is of the right dimensions
			Graph_GetLabelMatrix(gc->g, Schema_GetID(s));

			// Append label id.
			array_append(remove_labels, Schema_GetID(s));
			// remove node from  index.
			Schema_RemoveNodeFromIndices(s, node);
		}
	}
	raxStop(&it); 

	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	label_count = array_len(add_labels);
	if(labels_added_count) *labels_added_count = label_count;
	// Update label matrixes.
	if(label_count > 0) {
		Graph_LabelNode(gc->g, node->id ,add_labels, label_count);
		UndoLog_AddLabels(&query_ctx->undo_log, node, add_labels);
	}
	array_free(add_labels);

	label_count = array_len(remove_labels);
	if(labels_removed_count) *labels_removed_count = label_count;
	if(label_count > 0) {
		Graph_RemoveLabelNode(gc->g, node->id ,remove_labels, label_count);
		UndoLog_RemoveLabels(&query_ctx->undo_log, node, remove_labels);
	}
	array_free(remove_labels);
}
