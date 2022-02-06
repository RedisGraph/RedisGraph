/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_hub.h"
#include "../query_ctx.h"
#include "../undo_log/undo_log.h"

// add operation to the undo log
static void _add_undo_op(UndoOp *op) {
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	array_append(query_ctx->undo_log.undo_list, *op);
}

// delete all references to a node from any indices built upon its properties
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

		// Update any indices this entity is represented in
		Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
		if(idx) Index_RemoveNode(idx, n);

		idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
		if(idx) Index_RemoveNode(idx, n);
	}
}

static void _DeleteEdgeFromIndices(GraphContext *gc, Edge *e) {
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

// delete all references to a node from any indices built upon its properties
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

		// Update any indices this entity is represented in
		Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
		if(idx) Index_IndexNode(idx, n);

		idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
		if(idx) Index_IndexNode(idx, n);
	}
}

static void _AddEdgeToIndices(GraphContext *gc, Edge *e) {
	Schema  *s  =  NULL;
	Graph   *g  =  gc->g;

	int relation_id = EDGE_GET_RELATION_ID(e, g);

	s = GraphContext_GetSchemaByID(gc, relation_id, SCHEMA_EDGE);

	// update any indices this entity is represented in
	Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
	if(idx) Index_IndexEdge(idx, e);

	idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
	if(idx) Index_IndexEdge(idx, e);
}

// Add properties to the GraphEntity.
static inline uint _AddProperties(Entity *e, Entity *props) {
	int failed_updates = 0;
	for(int i = 0; i < props->prop_count; i++) {
		EntityProperty *prop = props->properties + i;
		bool updated = Entity_AddProperty(e, prop->id, prop->value, false);
		if(!updated) failed_updates++;
	}

	return props->prop_count - failed_updates;
}

uint CreateNode
(
	GraphContext *gc,
	Node *n,
	LabelID *labels,
	uint label_count,
	Entity *props
) {
	ASSERT(gc != NULL);
	ASSERT(n != NULL);

	Graph_CreateNode(gc->g, n, labels, label_count);
	uint properties_set = _AddProperties(n->entity, props);

	// add node labels
	for(uint i = 0; i < label_count; i++) {
		Schema *s = GraphContext_GetSchemaByID(gc, labels[i], SCHEMA_NODE);
		ASSERT(s);

		if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, n);
	}

	// add node creation operation to undo log
	UndoOp op;
	UndoLog_CreateNode(&op, *n);
	_add_undo_op(&op);

	return properties_set;
}

uint CreateEdge
(
	GraphContext *gc,
	Edge *e,
	NodeID src,
	NodeID dst,
	int r,
	Entity *props
) {
	ASSERT(gc != NULL);
	ASSERT(e != NULL);

	Graph_CreateEdge(gc->g, src, dst, r, e);
	uint properties_set = _AddProperties(e->entity, props);

	Schema *s = GraphContext_GetSchema(gc, e->relationship, SCHEMA_EDGE);
	// all schemas have been created in the edge blueprint loop or earlier
	ASSERT(s != NULL);

	if(s && Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, e);

	// add edge creation operation to undo log
	UndoOp op;
	UndoLog_CreateEdge(&op, *e);
	_add_undo_op(&op);

	return properties_set;
}

uint DeleteNode
(
	GraphContext *gc,
	Node *n
) {
	ASSERT(gc != NULL);
	ASSERT(n != NULL);

	uint label_count;
	NODE_GET_LABELS(gc->g, n, label_count);

	Edge *edges = array_new(Edge, 1);

	GrB_Index src;
	GrB_Index dest;

	// collect edges
	Graph_GetNodeEdges(gc->g, n, GRAPH_EDGE_DIR_BOTH, GRAPH_NO_RELATION, &edges);

	uint edge_count = array_len(edges);
	for (uint i = 0; i < edge_count; i++) {
		DeleteEdge(gc, edges + i);
	}

	array_free(edges);

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

	if(GraphContext_HasIndices(gc)) {
		_DeleteNodeFromIndices(gc, n);
	}

	Graph_DeleteNode(gc->g, n);

	return edge_count;
}

int DeleteEdge
(
	GraphContext *gc,
	Edge *e
) {
	ASSERT(gc != NULL);
	ASSERT(e != NULL);

	// add edge deletion operation to undo log
	UndoOp op;
	Edge clone;
	Edge_Clone(e, &clone);
	UndoLog_DeleteEdge(&op, clone);
	_add_undo_op(&op);

	if(GraphContext_HasIndices(gc)) {
		_DeleteEdgeFromIndices(gc, e);
	}

	return Graph_DeleteEdge(gc->g, e);
}

static int _Update_Entity
(
	GraphContext *gc,
	GraphEntity *ge,
	Attribute_ID attr_id,
	SIValue new_value,
	GraphEntityType entity_type
) {
	if(attr_id == ATTRIBUTE_ALL) {
		int prop_count = ENTITY_PROP_COUNT(ge);
		for(int i = 0; i < prop_count; i++) {
			EntityProperty *prop = ENTITY_PROPS(ge) + i;
			// add entity update operation to undo log
			UndoOp op;
			SIValue clone = SI_CloneValue(prop->value);
			UndoLog_UpdateEntity(&op, ge, prop->id, clone, entity_type);
			_add_undo_op(&op);
		}
	} else {
		SIValue *orig_value = Entity_GetProperty(ge->entity, attr_id);
		// add entity update operation to undo log
		UndoOp op;
		SIValue clone = SI_CloneValue(*orig_value);
		UndoLog_UpdateEntity(&op, ge, attr_id, clone, entity_type);
		_add_undo_op(&op);
	}

	return Graph_UpdateEntity(gc->g, ge, attr_id, new_value, entity_type);
}

int UpdateEntity
(
	GraphContext *gc,
	GraphEntity *ge,
	Entity *props,        // attribute to update
	GraphEntityType entity_type
) {
	ASSERT(gc != NULL);
	ASSERT(ge != NULL);

	int updates = 0;
	uint prop_count = props->prop_count;
	for (uint i = 0; i < prop_count; i++) {
		EntityProperty *prop = props->properties + i;
		updates += _Update_Entity(gc, ge, prop->id, prop->value, entity_type);
	}

	if(entity_type == GETYPE_NODE) {
		_AddNodeToIndices(gc, (Node *)ge);
	} else {
		_AddEdgeToIndices(gc, (Edge *)ge);
	}

	return updates;
}
