/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_hub.h"
#include "../query_ctx.h"
#include "../undo_log/undo_log.h"

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
static inline uint _AddProperties
(
	GraphEntity *e,
	AttributeSet *attr
) {
	int failed_updates = 0;
	AttributeSet _set = *attr;
	for(int i = 0; i < ATTRIBUTE_SET_COUNT(_set); i++) {
		Attribute *a = _set->attributes + i;
		bool updated = GraphEntity_AddProperty(e, a->id, a->value);
		if(!updated) failed_updates++;
	}

	return _set->attr_count - failed_updates;
}

uint CreateNode
(
	GraphContext *gc,
	Node *n,
	LabelID *labels,
	uint label_count,
	AttributeSet *props
) {
	ASSERT(gc != NULL);
	ASSERT(n != NULL);

	Graph_CreateNode(gc->g, n, labels, label_count);
	uint properties_set = _AddProperties((GraphEntity *)n, props);

	// add node labels
	for(uint i = 0; i < label_count; i++) {
		Schema *s = GraphContext_GetSchemaByID(gc, labels[i], SCHEMA_NODE);
		ASSERT(s);

		if(Schema_HasIndices(s)) Schema_AddNodeToIndices(s, n);
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
	AttributeSet *props
) {
	ASSERT(gc != NULL);
	ASSERT(e != NULL);

	Graph_CreateEdge(gc->g, src, dst, r, e);
	uint properties_set = _AddProperties((GraphEntity *)e, props);

	Schema *s = GraphContext_GetSchema(gc, e->relationship, SCHEMA_EDGE);
	// all schemas have been created in the edge blueprint loop or earlier
	ASSERT(s != NULL);

	if(s && Schema_HasIndices(s)) Schema_AddEdgeToIndices(s, e);

	// add edge creation operation to undo log
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_CreateEdge(&query_ctx->undo_log, *e);

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
	LabelID *labels_clone = rm_malloc(sizeof(LabelID) * label_count);
	for (uint i = 0; i < label_count; i++) {
		labels_clone[i] = labels[i];
	}
	
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_DeleteNode(&query_ctx->undo_log, n, labels_clone, label_count);

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
	QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
	UndoLog_DeleteEdge(&query_ctx->undo_log, e);

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
	if(attr_id == ATTRIBUTE_ID_ALL) {
		AttributeSet *set = ENTITY_ATTRIBUTE_SET(ge);
		for(int i = 0; i < ATTRIBUTE_SET_COUNT(*set); i++) {
			Attribute_ID id;
			// add entity update operation to undo log
			SIValue clone = SI_CloneValue(AttributeSet_GetIdx(*set, i, &id));
			QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
			UndoLog_UpdateEntity(&query_ctx->undo_log, ge, id, clone, entity_type);
		}
	} else {
		SIValue *orig_value = GraphEntity_GetProperty(ge, attr_id);
		// add entity update operation to undo log
		SIValue clone = SI_CloneValue(*orig_value);
		QueryCtx *query_ctx = QueryCtx_GetQueryCtx();
		UndoLog_UpdateEntity(&query_ctx->undo_log, ge, attr_id, clone, entity_type);
	}

	return Graph_UpdateEntity(ge, attr_id, new_value, entity_type);
}

int UpdateEntity
(
	GraphContext *gc,
	GraphEntity *ge,
	AttributeSet *attr,        // attribute to update
	GraphEntityType entity_type
) {
	ASSERT(gc != NULL);
	ASSERT(ge != NULL);

	int updates = 0;
	AttributeSet _set = *attr;
	for (uint i = 0; i < ATTRIBUTE_SET_COUNT(_set); i++) {
		Attribute *prop = _set->attributes + i;
		updates += _Update_Entity(gc, ge, prop->id, prop->value, entity_type);
	}

	if(entity_type == GETYPE_NODE) {
		_AddNodeToIndices(gc, (Node *)ge);
	} else {
		_AddEdgeToIndices(gc, (Edge *)ge);
	}

	return updates;
}
