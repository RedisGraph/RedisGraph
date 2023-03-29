/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
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

	Schema   *s      = NULL;
	Graph    *g      = gc->g;
	EntityID node_id = ENTITY_GET_ID(n);

	// retrieve node labels
	uint label_count;
	NODE_GET_LABELS(g, n, label_count);

	for(uint i = 0; i < label_count; i++) {
		int label_id = labels[i];
		s = GraphContext_GetSchemaByID(gc, label_id, SCHEMA_NODE);
		ASSERT(s != NULL);

		// update any indices this entity is represented in
		Schema_RemoveNodeFromIndices(s, n);
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
	Schema_RemoveEdgeFromIndices(s, e);
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

uint CreateNode
(
	GraphContext *gc,
	Node *n,
	LabelID *labels,
	uint label_count,
	AttributeSet set
) {
	ASSERT(gc != NULL);
	ASSERT(n != NULL);

	Graph_CreateNode(gc->g, n, labels, label_count);
	*n->attributes = set;

	// add node labels
	for(uint i = 0; i < label_count; i++) {
		Schema *s = GraphContext_GetSchemaByID(gc, labels[i], SCHEMA_NODE);
		ASSERT(s);
		Schema_AddNodeToIndices(s, n);
	}

	// add node creation operation to undo log
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_CreateNode(&query_ctx->undo_log, n);

	return ATTRIBUTE_SET_COUNT(set);
}

uint CreateEdge
(
	GraphContext *gc,
	Edge *e,
	NodeID src,
	NodeID dst,
	int r,
	AttributeSet set
) {
	ASSERT(gc != NULL);
	ASSERT(e != NULL);

	Graph_CreateEdge(gc->g, src, dst, r, e);
	*e->attributes = set;

	Schema *s = GraphContext_GetSchema(gc, e->relationship, SCHEMA_EDGE);
	// all schemas have been created in the edge blueprint loop or earlier
	ASSERT(s != NULL);
	Schema_AddEdgeToIndices(s, e);

	// add edge creation operation to undo log
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_CreateEdge(&query_ctx->undo_log, e);

	return ATTRIBUTE_SET_COUNT(set);
}

void DeleteNodes
(
	GraphContext *gc,
	Node *nodes,
	uint count
) {
	ASSERT(gc != NULL);
	ASSERT(nodes != NULL);

	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	bool has_indices = GraphContext_HasIndices(gc);

	for(uint i = 0; i < count; i++) {
		Node *n = nodes + i;
		// add node deletion operation to undo log
		UndoLog_DeleteNode(&query_ctx->undo_log, n);

		if(has_indices) {
			_DeleteNodeFromIndices(gc, n);
		}
	}

	Graph_DeleteNodes(gc->g, nodes, count);
}

void DeleteEdges
(
	GraphContext *gc,
	Edge *edges,
	uint64_t count
) {
	ASSERT(gc != NULL);
	ASSERT(count > 0);
	ASSERT(edges != NULL);

	// add edge deletion operation to undo log
	bool has_indecise = GraphContext_HasIndices(gc);
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	for (uint i = 0; i < count; i++) {
		UndoLog_DeleteEdge(&query_ctx->undo_log, edges + i);

		if(has_indecise) {
			_DeleteEdgeFromIndices(gc, edges + i);
		}
	}

	Graph_DeleteEdges(gc->g, edges, count);
}

// updates a graph entity attribute set. Returns as out params the number
// of properties set and removed.
void UpdateEntityProperties
(
	GraphContext *gc,             // graph context
	GraphEntity *ge,              // updated entity
	const AttributeSet set,       // new attributes
	GraphEntityType entity_type,  // entity type
	uint *props_set_count,        // number of attributes set
	uint *props_removed_count     // number of attributes removed
) {
	ASSERT(gc != NULL);
	ASSERT(ge != NULL);
	ASSERT(props_set_count     != NULL);
	ASSERT(props_removed_count != NULL);

	int set_props     = 0;
	int removed_props = 0;

	AttributeSet old_set = *ge->attributes;

	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_UpdateEntity(&query_ctx->undo_log, ge, old_set, entity_type);

	for (uint i = 0; i < ATTRIBUTE_SET_COUNT(set); i++) {
		Attribute *prop = set->attributes + i;

		SIValue *v = AttributeSet_Get(old_set, prop->id);

		if(v == ATTRIBUTE_NOTFOUND) {
			set_props++;
		} else if(SIValue_Compare(*v, prop->value, NULL) != 0) {
			set_props++;
			removed_props++;
		}
	}

	for (uint i = 0; i < ATTRIBUTE_SET_COUNT(old_set); i++) {
		Attribute *prop = old_set->attributes + i;

		SIValue *v = AttributeSet_Get(set, prop->id);

		if(v == ATTRIBUTE_NOTFOUND) {
			removed_props++;
		}
	}

	*ge->attributes = set;

	if(entity_type == GETYPE_NODE) {
		_AddNodeToIndices(gc, (Node *)ge);
	} else {
		_AddEdgeToIndices(gc, (Edge *)ge);
	}

	*props_set_count = set_props;
	*props_removed_count = removed_props;
}

void UpdateNodeLabels
(
	GraphContext *gc,            // graph context to update the entity
	Node *node,                  // the node to be updated
	const char **add_labels,     // labels to add to the node
	const char **remove_labels,  // labels to add to the node
	uint *labels_added_count,    // number of labels added (out param)
	uint *labels_removed_count   // number of labels removed (out param)
) {
	ASSERT(gc   != NULL);
	ASSERT(node != NULL);

	// quick return if there are no labels
	if(add_labels == NULL && remove_labels == NULL) {
		return;
	}

	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();

	if(add_labels != NULL) {
		uint label_count = array_len(add_labels);
		int add_labels_ids[label_count];
		uint add_labels_index = 0;

		for (uint i = 0; i < label_count; i++) {
			const char *label = add_labels[i];
			// get or create label matrix
			const Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
			bool schema_created = false;
			if(s == NULL) {
				s = AddSchema(gc, label, SCHEMA_NODE);
				schema_created = true;
			}

			int  schema_id = Schema_GetID(s);
			bool node_labeled = Graph_IsNodeLabeled(gc->g, node->id, schema_id);

			if(!node_labeled) {
				// sync matrix, make sure label matrix is of the right dimensions
				if(schema_created) {
					RG_Matrix m = Graph_GetLabelMatrix(gc->g, schema_id);
				}
				// append label id
				add_labels_ids[add_labels_index++] = schema_id;
				// add to index
				Schema_AddNodeToIndices(s, node);
			}
		}

		if(add_labels_index > 0) {
			*labels_added_count = add_labels_index;

			// update node's labels
			Graph_LabelNode(gc->g, node->id ,add_labels_ids, add_labels_index);
			UndoLog_AddLabels(&query_ctx->undo_log, node, add_labels_ids, add_labels_index);
		}
	}

	if(remove_labels != NULL) {
		uint label_count = array_len(remove_labels);
		int remove_labels_ids[label_count];
		uint remove_labels_index = 0;

		for (uint i = 0; i < label_count; i++) {
			const char *label = remove_labels[i];

			// label removal
			// get or create label matrix
			const Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
			if(s == NULL) {
				// skip removal of none existing label
				continue;
			}

			// append label id
			remove_labels_ids[remove_labels_index++] = Schema_GetID(s);
			// remove node from index
			Schema_RemoveNodeFromIndices(s, node);
		}

		if(remove_labels_index > 0) {
			*labels_removed_count = remove_labels_index;

			// update node's labels
			Graph_RemoveNodeLabels(gc->g, ENTITY_GET_ID(node), remove_labels_ids,
					remove_labels_index);
			UndoLog_RemoveLabels(&query_ctx->undo_log, node, remove_labels_ids, remove_labels_index);
		}
	}
}


Schema *AddSchema
(
	GraphContext *gc,             // graph context to add the schema
	const char *label,            // schema label
	SchemaType t                  // schema type (node/edge)
) {
	ASSERT(gc != NULL);
	ASSERT(label != NULL);
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	Schema *s = GraphContext_AddSchema(gc, label, t);
	UndoLog_AddSchema(&query_ctx->undo_log, s->id, s->type);
	return s;
}

Attribute_ID FindOrAddAttribute
(
	GraphContext *gc,     // graph context to add the attribute
	const char *attribute // attribute name
) {
	ASSERT(gc != NULL);
	ASSERT(attribute != NULL);

	bool created;
	Attribute_ID attr_id = GraphContext_FindOrAddAttribute(gc, attribute, &created);
	// In case there was an append, the latest id should be tracked
	if(created) {
		QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
		UndoLog_AddAttribute(&query_ctx->undo_log, attr_id);
	}
	return attr_id;
}

