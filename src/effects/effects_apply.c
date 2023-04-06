/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "effects.h"
#include "../graph/graph_hub.h"

#include <stdio.h>

// read effect type from stream
static inline EffectType ReadEffectType
(
	FILE *stream  // effects stream
) {
	EffectType t = EFFECT_UNKNOWN;  // default to unknown effect type

	// read EffectType off of stream
	fread_assert(&t, sizeof(EffectType), stream);

	return t;
}

static AttributeSet ReadAttributeSet
(
	FILE *stream
) {
	//--------------------------------------------------------------------------
	// effect format:
	// attribute count
	// attributes (id,value) pair
	//--------------------------------------------------------------------------

	//--------------------------------------------------------------------------
	// read attribute count
	//--------------------------------------------------------------------------

	ushort attr_count;
	fread_assert(&attr_count, sizeof(attr_count), stream);

	//--------------------------------------------------------------------------
	// read attributes
	//--------------------------------------------------------------------------

	SIValue values[attr_count];
	Attribute_ID ids[attr_count];

	for(ushort i = 0; i < attr_count; i++) {
		// read attribute ID
		fread_assert(ids + i, sizeof(Attribute_ID), stream);
		
		// read attribute value
		values[i] = SIValue_FromBinary(stream);
	}

	AttributeSet attr_set = NULL;
	AttributeSet_AddNoClone(&attr_set, ids, values, attr_count, false);

	return attr_set;
}

static void ApplyCreateNode
(
	FILE *stream,     // effects stream
	GraphContext *gc  // graph to operate on
) {
	//--------------------------------------------------------------------------
	// effect format:
	// label count
	// labels
	// attribute count
	// attributes (id,value) pair
	//--------------------------------------------------------------------------

	//--------------------------------------------------------------------------
	// read label count
	//--------------------------------------------------------------------------

	ushort lbl_count;
	fread_assert(&lbl_count, sizeof(lbl_count), stream);

	//--------------------------------------------------------------------------
	// read labels
	//--------------------------------------------------------------------------

	LabelID labels[lbl_count];
	for(ushort i = 0; i < lbl_count; i++) {
		fread_assert(labels + i, sizeof(LabelID), stream);
	}

	//--------------------------------------------------------------------------
	// read attributes
	//--------------------------------------------------------------------------

	AttributeSet attr_set = ReadAttributeSet(stream);

	//--------------------------------------------------------------------------
	// create node
	//--------------------------------------------------------------------------

	Node n;
	CreateNode(gc, &n, labels, lbl_count, attr_set, false);
}

static void ApplyCreateEdge
(
	FILE *stream,     // effects stream
	GraphContext *gc  // graph to operate on
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// relationship count
	// relationships
	// src node ID
	// dest node ID
	// attribute count
	// attributes (id,value) pair
	//--------------------------------------------------------------------------

	//--------------------------------------------------------------------------
	// read relationship type count
	//--------------------------------------------------------------------------

	ushort rel_count;
	fread_assert(&rel_count, sizeof(rel_count), stream);
	ASSERT(rel_count == 1);

	//--------------------------------------------------------------------------
	// read relationship type
	//--------------------------------------------------------------------------

	RelationID r;
	fread_assert(&r, sizeof(r), stream);

	//--------------------------------------------------------------------------
	// read src node ID
	//--------------------------------------------------------------------------

	NodeID src_id;
	fread_assert(&src_id, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// read dest node ID
	//--------------------------------------------------------------------------

	NodeID dest_id;
	fread_assert(&dest_id, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// read attributes
	//--------------------------------------------------------------------------

	AttributeSet attr_set = ReadAttributeSet(stream);

	//--------------------------------------------------------------------------
	// create edge
	//--------------------------------------------------------------------------

	Edge e;
	CreateEdge(gc, &e, src_id, dest_id, r, attr_set, false);
}

static void ApplyLabels
(
	FILE *stream,     // effects stream
	GraphContext *gc, // graph to operate on
	bool add          // add or remove labels
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------
	
	//--------------------------------------------------------------------------
	// read node ID
	//--------------------------------------------------------------------------

	EntityID id;
	fread_assert(&id, sizeof(id), stream);

	//--------------------------------------------------------------------------
	// get updated node
	//--------------------------------------------------------------------------

	Node  n;
	Graph *g = gc->g;

	Graph_GetNode(g, id, &n);

	//--------------------------------------------------------------------------
	// read labels count
	//--------------------------------------------------------------------------

	ushort lbl_count;
	fread_assert(&lbl_count, sizeof(lbl_count), stream);
	ASSERT(lbl_count > 0);

	// TODO: move to LabelID
	uint n_add_labels          = 0;
	uint n_remove_labels       = 0;
	const char **add_labels    = NULL;
	const char **remove_labels = NULL;
	const char *lbl[lbl_count];

	// assign lbl to the appropriate array
	if(add) {
		add_labels = lbl;
		n_add_labels = lbl_count;
	} else {
		remove_labels = lbl;
		n_remove_labels = lbl_count;
	}

	//--------------------------------------------------------------------------
	// read labels
	//--------------------------------------------------------------------------

	for(ushort i = 0; i < lbl_count; i++) {
		LabelID l;
		fread_assert(&l, sizeof(LabelID), stream);
		Schema *s = GraphContext_GetSchemaByID(gc, l, SCHEMA_NODE);
		ASSERT(s != NULL);
		lbl[i] = Schema_GetName(s);
	}

	//--------------------------------------------------------------------------
	// update node labels
	//--------------------------------------------------------------------------

	uint labels_added_count;
	uint labels_removed_count;
	UpdateNodeLabels(gc, &n, add_labels, remove_labels, n_add_labels,
			n_remove_labels, &labels_added_count, &labels_removed_count, false);
}

static void ApplyAddSchema
(
	FILE *stream,     // effects stream
	GraphContext *gc  // graph to operate on
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    schema type
	//    schema name
	//--------------------------------------------------------------------------

	// read schema type
	SchemaType t;
	fread_assert(&t, sizeof(t), stream);

	// read schema name
	// read string length
	size_t l;
	fread_assert(&l, sizeof(l), stream);

	// read string
	char schema_name[l];
	fread_assert(schema_name, l, stream);

	// create schema
	AddSchema(gc, schema_name, t, false);
}

static void ApplyAddAttribute
(
	FILE *stream,     // effects stream
	GraphContext *gc  // graph to operate on
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// attribute name
	//--------------------------------------------------------------------------
	
	// read attribute name length
	size_t l;
	fread_assert(&l, sizeof(l), stream);

	// read attribute name
	const char attr[l];
	fread_assert(attr, l, stream);

	// TODO: debug make sure attr isn't part of the graph
	
	// add attribute
	FindOrAddAttribute(gc, attr, false);
}

// process Update_Edge effect
static void ApplyUpdateEdge
(
	FILE *stream,     // effects stream
	GraphContext *gc  // graph to operate on
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    edge ID
	//    relation ID
	//    src ID
	//    dest ID
	//    attribute id
	//    attribute value
	//--------------------------------------------------------------------------
	
	bool res;
	Edge e;                // edge to delete
	Node s;                // edge src node
	Node t;                // edge dest node
	SIValue v;             // updated value
	uint props_set;        // number of attributes updated
	uint props_removed;    // number of attributes removed
	Attribute_ID attr_id;  // entity ID

	NodeID     s_id = INVALID_ENTITY_ID;       // edge src node ID
	NodeID     t_id = INVALID_ENTITY_ID;       // edge dest node ID
	RelationID r_id = GRAPH_UNKNOWN_RELATION;  // edge rel-type

	Graph *g = gc->g;
	EntityID id = INVALID_ENTITY_ID;

	//--------------------------------------------------------------------------
	// read edge ID
	//--------------------------------------------------------------------------

	fread_assert(&id, sizeof(EntityID), stream);
	ASSERT(id != INVALID_ENTITY_ID);

	//--------------------------------------------------------------------------
	// read relation ID
	//--------------------------------------------------------------------------

	fread_assert(&r_id, sizeof(RelationID), stream);
	ASSERT(r_id >= 0);

	//--------------------------------------------------------------------------
	// read src ID
	//--------------------------------------------------------------------------

	fread_assert(&s_id, sizeof(NodeID), stream);
	ASSERT(s_id != INVALID_ENTITY_ID);

	//--------------------------------------------------------------------------
	// read dest ID
	//--------------------------------------------------------------------------

	fread_assert(&t_id, sizeof(NodeID), stream);
	ASSERT(t_id != INVALID_ENTITY_ID);

	//--------------------------------------------------------------------------
	// read attribute ID
	//--------------------------------------------------------------------------

	fread_assert(&attr_id, sizeof(Attribute_ID), stream);
	ASSERT(attr_id != ATTRIBUTE_ID_ALL && attr_id != ATTRIBUTE_ID_NONE);

	//--------------------------------------------------------------------------
	// read attribute value
	//--------------------------------------------------------------------------

	v = SIValue_FromBinary(stream);
	ASSERT(SI_TYPE(v) & (SI_VALID_PROPERTY_VALUE | T_NULL));

	//--------------------------------------------------------------------------
	// fetch updated entity
	//--------------------------------------------------------------------------

	// get src node, dest node and edge from the graph
	res = Graph_GetNode(g, s_id, &s);
	ASSERT(res != 0);
	res = Graph_GetNode(g, t_id, &t);
	ASSERT(res != 0);
	res = Graph_GetEdge(g, id, &e);
	ASSERT(res != 0);

	// set edge relation, src and destination node
	Edge_SetSrcNode(&e, &s);
	Edge_SetDestNode(&e, &t);
	Edge_SetRelationID(&e, r_id);

	// construct update attribute-set
	AttributeSet set = NULL;
	AttributeSet_AddNoClone(&set, &attr_id, &v, 1, true);

	// perform update
	UpdateEntityProperties(gc, (GraphEntity*)&e, set, GETYPE_EDGE, &props_set,
			&props_removed, false);

	// clean up
	AttributeSet_Free(&set);
}

// process UpdateNode effect
static void ApplyUpdateNode
(
	FILE *stream,     // effects stream
	GraphContext *gc  // graph to operate on
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    entity ID
	//    attribute ID
	//    attribute value
	//--------------------------------------------------------------------------

	SIValue v;             // updated value
	uint props_set;        // number of attributes updated
	uint props_removed;    // number of attributes removed
	Attribute_ID attr_id;  // entity ID

	Graph *g = gc->g;
	EntityID id = INVALID_ENTITY_ID;

	//--------------------------------------------------------------------------
	// read node ID
	//--------------------------------------------------------------------------

	fread_assert(&id, sizeof(EntityID), stream);

	//--------------------------------------------------------------------------
	// read attribute ID
	//--------------------------------------------------------------------------

	fread_assert(&attr_id, sizeof(Attribute_ID), stream);

	//--------------------------------------------------------------------------
	// read attribute value
	//--------------------------------------------------------------------------

	v = SIValue_FromBinary(stream);

	//--------------------------------------------------------------------------
	// fetch updated entity
	//--------------------------------------------------------------------------

	Node n;
	int res = Graph_GetNode(g, id, &n);

	// make sure entity was found
	UNUSED(res);
	ASSERT(res == true);

	// construct update attribute-set
	AttributeSet set = NULL;
	AttributeSet_AddNoClone(&set, &attr_id, &v, 1, true);

	// perform update
	UpdateEntityProperties(gc, (GraphEntity*)&n, set, GETYPE_NODE, &props_set,
			&props_removed, false);

	// clean up
	AttributeSet_Free(&set);
}

// process DeleteNode effect
static void ApplyDeleteNode
(
	FILE *stream,     // effects stream
	GraphContext *gc  // graph to operate on
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    node ID
	//--------------------------------------------------------------------------
	
	Node n;            // node to delete
	EntityID id;       // node ID
	Graph *g = gc->g;  // graph to delete node from

	// read node ID off of stream
	fread_assert(&id, sizeof(EntityID), stream);

	// retrieve node from graph
	int res = Graph_GetNode(g, id, &n);
	ASSERT(res != 0);

	// delete node
	DeleteNodes(gc, &n, 1, false);
}

// process DeleteNode effect
static void ApplyDeleteEdge
(
	FILE *stream,     // effects stream
	GraphContext *gc  // graph to operate on
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    edge ID
	//    relation ID
	//    src ID
	//    dest ID
	//--------------------------------------------------------------------------

	Edge e;  // edge to delete
	Node s;  // edge src node
	Node t;  // edge dest node

	EntityID id   = INVALID_ENTITY_ID;       // edge ID
	int      r_id = GRAPH_UNKNOWN_RELATION;  // edge rel-type
	NodeID   s_id = INVALID_ENTITY_ID;       // edge src node ID
	NodeID   t_id = INVALID_ENTITY_ID;       // edge dest node ID

	int res;
	UNUSED(res);

	Graph *g = gc->g;  // graph to delete edge from

	// read edge ID
	fread_assert(&id, fldsiz(UndoDeleteEdgeOp, id), stream);

	// read relation ID
	fread_assert(&r_id, fldsiz(UndoDeleteEdgeOp, relationID), stream);

	// read src node ID
	fread_assert(&s_id, fldsiz(UndoDeleteEdgeOp, srcNodeID), stream);

	// read dest node ID
	fread_assert(&t_id, fldsiz(UndoDeleteEdgeOp, destNodeID), stream);

	// get src node, dest node and edge from the graph
	res = Graph_GetNode(g, s_id, (Node*)&s);
	ASSERT(res != 0);
	res = Graph_GetNode(g, t_id, (Node*)&t);
	ASSERT(res != 0);
	res = Graph_GetEdge(g, id, (Edge*)&e);
	ASSERT(res != 0);

	// set edge relation, src and destination node
	Edge_SetSrcNode(&e, &s);
	Edge_SetDestNode(&e, &t);
	Edge_SetRelationID(&e, r_id);

	// delete edge
	DeleteEdges(gc, &e, 1, false);
}

// returns false in case of effect encode/decode version mismatch
static bool ValidateVersion
(
	FILE *stream  // effects stream
) {
	ASSERT(stream != NULL);

	// read version
	uint8_t v;
	fread_assert(&v, sizeof(uint8_t), stream);

	if(v != EFFECTS_VERSION) {
		// unexpected effects version
		RedisModule_Log(NULL, "warning",
				"GRAPH.EFFECT version mismatch expected: %d got: %d",
				EFFECTS_VERSION, v);
		return false;
	}

	return true;
}

// applys effects encoded in buffer
void Effects_Apply
(
	GraphContext *gc,          // graph to operate on
	const char *effects_buff,  // encoded effects
	size_t l                   // size of buffer
) {
	// validations
	ASSERT(l > 0);  // buffer can't be empty
	ASSERT(effects_buff != NULL);  // buffer can't be NULL

	// read buffer in a stream fashion
	FILE *stream = fmemopen((void*)effects_buff, l, "r");

	// validate effects version
	if(ValidateVersion(stream) == false) {
		// replica/primary out of sync
		exit(1);
	}

	// lock graph for writing
	Graph *g = GraphContext_GetGraph(gc);
	Graph_AcquireWriteLock(g);

	// as long as there's data in stream
	while(ftell(stream) < l) {
		// read effect type
		EffectType t = ReadEffectType(stream);
		switch(t) {
			case EFFECT_DELETE_NODE:
				ApplyDeleteNode(stream, gc);
				break;
			case EFFECT_DELETE_EDGE:
				ApplyDeleteEdge(stream, gc);
				break;
			case EFFECT_UPDATE_NODE:
				ApplyUpdateNode(stream, gc);
				break;
			case EFFECT_UPDATE_EDGE:
				ApplyUpdateEdge(stream, gc);
				break;
			case EFFECT_CREATE_NODE:    
				ApplyCreateNode(stream, gc);
				break;
			case EFFECT_CREATE_EDGE:
				ApplyCreateEdge(stream, gc);
				break;
			case EFFECT_SET_LABELS:
				ApplyLabels(stream, gc, true);
				break;
			case EFFECT_REMOVE_LABELS: 
				ApplyLabels(stream, gc, false);
				break;
			case EFFECT_ADD_SCHEMA:
				ApplyAddSchema(stream, gc);
				break;
			case EFFECT_ADD_ATTRIBUTE:
				ApplyAddAttribute(stream, gc);
				break;
			default:
				assert(false && "unknown effect type");
				break;
		}
	}

	// release write lock
	Graph_ReleaseLock(g);

	// close stream
	fclose(stream);
}

