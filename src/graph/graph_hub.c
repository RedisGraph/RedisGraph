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
	ASSERT(s != NULL);

	Schema_AddEdgeToIndices(s, e);
}

uint CreateNode
(
	GraphContext *gc,
	Node *n,
	LabelID *labels,
	uint label_count,
	AttributeSet set,
	bool log
) {
	ASSERT(n  != NULL);
	ASSERT(gc != NULL);

	Graph_CreateNode(gc->g, n, labels, label_count);
	*n->attributes = set;

	// add node labels
	for(uint i = 0; i < label_count; i++) {
		Schema *s = GraphContext_GetSchemaByID(gc, labels[i], SCHEMA_NODE);
		ASSERT(s);
		Schema_AddNodeToIndices(s, n);
	}

	// add node creation operation to undo log
	if(log == true) {
		UndoLog *undo_log = QueryCtx_GetUndoLog();
		UndoLog_CreateNode(undo_log, n);
	}

	return ATTRIBUTE_SET_COUNT(set);
}

uint CreateEdge
(
	GraphContext *gc,
	Edge *e,
	NodeID src,
	NodeID dst,
	RelationID r,
	AttributeSet set,
	bool log
) {
	ASSERT(e  != NULL);
	ASSERT(gc != NULL);

	Graph_CreateEdge(gc->g, src, dst, r, e);
	*e->attributes = set;

	Schema *s = GraphContext_GetSchemaByID(gc, r, SCHEMA_EDGE);
	// all schemas have been created in the edge blueprint loop or earlier
	ASSERT(s != NULL);
	Schema_AddEdgeToIndices(s, e);

	// add edge creation operation to undo log
	if(log == true) {
		UndoLog *undo_log = QueryCtx_GetUndoLog();
		UndoLog_CreateEdge(undo_log, e);
	}

	return ATTRIBUTE_SET_COUNT(set);
}

// delete a node
// remove the node from the relevant indexes
// add node deletion operation to undo-log
// return 1 on success, 0 otherwise
void DeleteNodes
(
	GraphContext *gc,
	Node *nodes,
	uint n,
	bool log
) {
	ASSERT(gc != NULL);
	ASSERT(nodes != NULL);

	UndoLog *undo_log = (log == true) ? QueryCtx_GetUndoLog() : NULL;
	bool has_indices = GraphContext_HasIndices(gc);

	if(log == true || has_indices) {
		for(uint i = 0; i < n; i++) {
			Node *n = nodes + i;

			if(log == true) {
				// add node deletion operation to undo log
				UndoLog_DeleteNode(undo_log, n);
			}

			if(has_indices == true) {
				_DeleteNodeFromIndices(gc, n);
			}
		}
	}

	Graph_DeleteNodes(gc->g, nodes, n);
}

void DeleteEdges
(
	GraphContext *gc,
	Edge *edges,
	uint64_t n,
	bool log
) {
	ASSERT(gc != NULL);
	ASSERT(n > 0);
	ASSERT(edges != NULL);

	// add edge deletion operation to undo log
	bool has_indecise = GraphContext_HasIndices(gc);
	UndoLog *undo_log = (log == true) ? QueryCtx_GetUndoLog() : NULL;

	if(has_indecise == true || log == true) {
		for (uint i = 0; i < n; i++) {
			if(log == true) {
				UndoLog_DeleteEdge(undo_log, edges + i);
			}

			if(has_indecise == true) {
				_DeleteEdgeFromIndices(gc, edges + i);
			}
		}
	}

	Graph_DeleteEdges(gc->g, edges, n);
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
	uint *props_removed_count,
	bool log
) {
	SIValue *old_value = GraphEntity_GetProperty(ge, attr_id);

	if(log == true) {
		UndoLog *undo_log = QueryCtx_GetUndoLog();
		if(attr_id == ATTRIBUTE_ID_ALL) {
			// we're requested to clear entitiy's attribute-set
			// backup entity's attributes in case we'll need to roolback
			const AttributeSet set = GraphEntity_GetAttributes(ge);
			for(int i = 0; i < ATTRIBUTE_SET_COUNT(set); i++) {
				Attribute_ID id;
				// add entity update operation to undo log
				SIValue value = AttributeSet_GetIdx(set, i, &id);
				UndoLog_UpdateEntity(undo_log, ge, id, value, entity_type);
			}
		} else {
			// add entity update operation to undo log
			UndoLog_UpdateEntity(undo_log, ge, attr_id, *old_value,
					entity_type);
		}
	}

	// update the property and set the appropriate counter
	int updates = Graph_UpdateEntity(ge, attr_id, new_value, entity_type);

	if(SIValue_IsNull(new_value)) {
		// removal of an attribute
		// in case the attribute is not present
		// the update will not be counted (Graph_UpdateEntity logic)
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
	uint *props_removed_count,    // number of attributes removed
	bool log                      // log update in undo-log
) {
	ASSERT(gc != NULL);
	ASSERT(ge != NULL);
	ASSERT(props_set_count     != NULL);
	ASSERT(props_removed_count != NULL);

	int set_props     = 0;
	int removed_props = 0;

	for (uint i = 0; i < ATTRIBUTE_SET_COUNT(set); i++) {
		Attribute *prop = set->attributes + i;
		uint _set_props     = 0;
		uint _removed_props = 0;

		_Update_Entity_Property(gc, ge, prop->id, prop->value, entity_type,
				&_set_props, &_removed_props, log);

		set_props     += _set_props;
		removed_props += _removed_props;
	}

	if(set_props + removed_props > 0) {
		if(entity_type == GETYPE_NODE) {
			_AddNodeToIndices(gc, (Node *)ge);
		} else {
			_AddEdgeToIndices(gc, (Edge *)ge);
		}
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
	uint n_add_labels,           // number of labels to add
	uint n_remove_labels,        // number of labels to remove
	uint *n_labels_added,        // number of labels added (out param)
	uint *n_labels_removed,      // number of labels removed (out param)
	bool log                     // log this operation in undo-log
) {
	ASSERT(gc   != NULL);
	ASSERT(node != NULL);

	// quick return if there are no labels
	if(add_labels == NULL && remove_labels == NULL) {
		return;
	}

	// if add_labels is specified its count must be > 0
	ASSERT((add_labels != NULL && n_add_labels > 0) ||
		   (add_labels == NULL && n_add_labels == 0));

	// if remove_labels is specified its count must be > 0
	ASSERT((remove_labels != NULL && n_remove_labels > 0) ||
		   (remove_labels == NULL && n_remove_labels == 0));

	UndoLog *undo_log = (log == true) ? QueryCtx_GetUndoLog() : NULL;

	if(add_labels != NULL) {
		int add_labels_ids[n_add_labels];
		uint add_labels_index = 0;

		for (uint i = 0; i < n_add_labels; i++) {
			const char *label = add_labels[i];
			// get or create label matrix
			const Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
			bool schema_created = false;
			if(s == NULL) {
				s = AddSchema(gc, label, SCHEMA_NODE, log);
				schema_created = true;
			}

			int  schema_id = Schema_GetID(s);
			bool node_labeled = Graph_IsNodeLabeled(gc->g, node->id, schema_id);

			if(!node_labeled) {
				// sync matrix
				// make sure label matrix is of the right dimensions
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
			*n_labels_added = add_labels_index;

			// update node's labels
			Graph_LabelNode(gc->g, node->id ,add_labels_ids, add_labels_index);
			if(log == true) {
				UndoLog_AddLabels(undo_log, node, add_labels_ids,
						add_labels_index);
			}
		}
	}

	if(remove_labels != NULL) {
		int remove_labels_ids[n_remove_labels];
		uint remove_labels_index = 0;

		for (uint i = 0; i < n_remove_labels; i++) {
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
			*n_labels_removed = remove_labels_index;

			// update node's labels
			Graph_RemoveNodeLabels(gc->g, ENTITY_GET_ID(node),
					remove_labels_ids, remove_labels_index);
			if(log == true) {
				UndoLog_RemoveLabels(undo_log, node, remove_labels_ids,
						remove_labels_index);
			}
		}
	}
}

Schema *AddSchema
(
	GraphContext *gc,   // graph context to add the schema
	const char *label,  // schema label
	SchemaType t,       // schema type (node/edge)
	bool log            // should operation be logged in the undo-log      
) {
	ASSERT(gc != NULL);
	ASSERT(label != NULL);
	Schema *s = GraphContext_AddSchema(gc, label, t);

	if(log == true) {
		UndoLog *undo_log = QueryCtx_GetUndoLog();
		UndoLog_AddSchema(undo_log, s->id, s->type);
	}

	return s;
}

Attribute_ID FindOrAddAttribute
(
	GraphContext *gc,       // graph context to add the attribute
	const char *attribute,  // attribute name
	bool log                // should operation be logged in the undo-log
) {
	ASSERT(gc != NULL);
	ASSERT(attribute != NULL);

	bool created;
	Attribute_ID attr_id = GraphContext_FindOrAddAttribute(gc, attribute, &created);
	// in case there was an append, the latest id should be tracked
	if(created == true && log == true) {
		UndoLog *undo_log = QueryCtx_GetUndoLog();
		UndoLog_AddAttribute(undo_log, attr_id);
	}
	return attr_id;
}

