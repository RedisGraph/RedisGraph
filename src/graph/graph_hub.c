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
	UndoLog_CreateNode(&query_ctx->undo_log, n);

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
	UndoLog_CreateEdge(&query_ctx->undo_log, e);

	return properties_set;
}

uint DeleteNode
(
	GraphContext *gc,
	Node *n
) {
	ASSERT(n != NULL);
	ASSERT(gc != NULL);

	// add node deletion operation to undo log	
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_DeleteNode(&query_ctx->undo_log, n);

	if(GraphContext_HasIndices(gc)) {
		_DeleteNodeFromIndices(gc, n);
	}

	Graph_DeleteNode(gc->g, n);

	return 1;
}

int DeleteEdge
(
	GraphContext *gc,
	Edge *e
) {
	ASSERT(e  != NULL);
	ASSERT(gc != NULL);

	// add edge deletion operation to undo log
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_DeleteEdge(&query_ctx->undo_log, e);

	if(GraphContext_HasIndices(gc)) {
		_DeleteEdgeFromIndices(gc, e);
	}

	return Graph_DeleteEdge(gc->g, e);
}

// update entity attributes and update undo log
// in case attr_id is ATTRIBUTE_ID_ALL clear all attributes values
static void _Update_Entity_Property
(
	GraphContext *gc,
	GraphEntity *ge,
	Attribute_ID attr_id,
	SIValue new_value,
	GraphEntityType entity_type,
	uint *props_set_count,
	uint *props_removed_count
) {
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	if(attr_id == ATTRIBUTE_ID_ALL) {
		// we're requested to clear entitiy's attribute-set
		// backup entity's attributes in case we'll need to roolback
		const AttributeSet set = GraphEntity_GetAttributes(ge);
		for(int i = 0; i < ATTRIBUTE_SET_COUNT(set); i++) {
			Attribute_ID id;
			// add entity update operation to undo log
			SIValue value = AttributeSet_GetIdx(set, i, &id);
			UndoLog_UpdateEntity(&query_ctx->undo_log, ge, id, value,
					entity_type);
		}
	} else {
		// TODO: what if orig_value returns as 'ATTRIBUTE_NOTFOUND' ?
		SIValue *orig_value = GraphEntity_GetProperty(ge, attr_id);
		// add entity update operation to undo log
		UndoLog_UpdateEntity(&query_ctx->undo_log, ge, attr_id, *orig_value,
				entity_type);
	}

	// update the property and set the appropriate counter.
	// TODO: what if attr_id is ATTRIBUTE_ID_ALL ? see graph.c:1008 
	SIValue *old_value = GraphEntity_GetProperty(ge, attr_id);
	int updates = Graph_UpdateEntity(ge, attr_id, new_value, entity_type);

	if(SIValue_IsNull(new_value)) {
		// removal of an attribute. In case the attribute is not present,
		// the update will not be counted (Graph_UpdateEntity logic).
		*props_removed_count = updates;
	} else {
		// addition of an attribte
		*props_set_count = updates;
		// overwrite exiting attribute is considered a removal
		if(old_value != ATTRIBUTE_NOTFOUND) {
			*props_removed_count = updates;
		}
	}
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

	int set_props     = 0;
	int removed_props = 0;

	for (uint i = 0; i < ATTRIBUTE_SET_COUNT(set); i++) {
		Attribute *prop = set->attributes + i;
		uint _set_props     = 0;
		uint _removed_props = 0;

		_Update_Entity_Property(gc, ge, prop->id, prop->value, entity_type,
				&_set_props, &_removed_props);

		set_props     += _set_props;
		removed_props += _removed_props;
	}

	if(entity_type == GETYPE_NODE) {
		_AddNodeToIndices(gc, (Node *)ge);
	} else {
		_AddEdgeToIndices(gc, (Edge *)ge);
	}

	// TODO: a bit annoying that we collect `set_props` and `removed_props`
	// when we might not need them
	if(props_set_count != NULL) {
		*props_set_count = set_props;
	}
	if(props_removed_count != NULL) {
		*props_removed_count = removed_props;
	}
}

void UpdateNodeLabels
(
	GraphContext *gc,            // graph context to update the entity
	Node *node,                  // the node to be updated
	rax *labels,     	         // labels to update
	uint *labels_added_count,    // number of labels added (out param)
	uint *labels_removed_count   // number of labels removed (out param)
) {
	ASSERT(gc   != NULL);
	ASSERT(node != NULL);

	// quick return if there are no labels
	if(labels == NULL) {
		return;
	}

	uint label_count = raxSize(labels);
	if(label_count == 0) {
		return;
	}

	raxIterator it;
	raxStart(&it, labels);

	// TODO: consider switching to a stack base arrays
	// int[label_count] add_labels
	int add_labels[label_count];
	int remove_labels[label_count];
	uint add_labels_index = 0;
	uint remove_labels_index = 0;

	// iterate over all keys in the rax
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		// get label string
		unsigned char *label = it.key;
		// TODO:this condition is a bit confusing
		// consider adding 2 unique pointers
		// e.g. CREATE_LABEL and REMOVE_LABEL
		if(it.data) {
			// get or create label matrix
			const Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
			bool schema_created = false;
			if(s == NULL) {
				s = GraphContext_AddSchema(gc, label, SCHEMA_NODE);
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
				add_labels[add_labels_index++] = schema_id;
				// add to index
				Schema_AddNodeToIndices(s, node);
			}
		} else {
			ASSERT(it.data == REMOVE_LABEL);
			// label removal
			// get or create label matrix
			const Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
			if(s == NULL) {
				// skip removal of none existing label
				continue;
			}

			// append label id
			remove_labels[remove_labels_index++] = Schema_GetID(s);
			// remove node from index
			Schema_RemoveNodeFromIndices(s, node);
		}
	}
	raxStop(&it); 

	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	if(add_labels_index) {
		*labels_added_count = add_labels_index;
	}
	// update node's labels
	if(add_labels_index > 0) {
		Graph_LabelNode(gc->g, node->id ,add_labels, add_labels_index);
		UndoLog_AddLabels(&query_ctx->undo_log, node, add_labels, add_labels_index);
	}

	if(remove_labels_index) {
		*labels_removed_count = remove_labels_index;
	}
	// update node's labels
	if(remove_labels_index > 0) {
		Graph_RemoveNodeLabels(gc->g, ENTITY_GET_ID(node), remove_labels,
				remove_labels_index);
		UndoLog_RemoveLabels(&query_ctx->undo_log, node, remove_labels, remove_labels_index);
	}
}

