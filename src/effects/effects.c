/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "effects.h"
#include "../query_ctx.h"

// write attributes to stream
static void WriteAttributeSet
(
	FILE *stream,             // stream to write attributes into
	const AttributeSet attrs  // attribute set to write to stream
) {
	//--------------------------------------------------------------------------
	// write attribute count
	//--------------------------------------------------------------------------

	ushort attr_count = ATTRIBUTE_SET_COUNT(attrs);
	fwrite_assert(&attr_count, sizeof(attr_count), stream);

	//--------------------------------------------------------------------------
	// write attributes
	//--------------------------------------------------------------------------

	for(ushort i = 0; i < attr_count; i++) {
		// get current attribute name and value
		Attribute_ID attr_id;
		SIValue attr = AttributeSet_GetIdx(attrs, i, &attr_id);

		// write attribute ID
		fwrite_assert(&attr_id, sizeof(Attribute_ID), stream);

		// write attribute value
		SIValue_ToBinary(stream, &attr);
	}
}

// dump EffectNodeDelete into stream
static void DumpNodeDeleteEffect
(
	FILE *stream,     // stream
	const Effect *e   // effect
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//--------------------------------------------------------------------------
	
	// effect 
	const EffectDeleteNode *_e = &e->delete_node;

	// write node ID
	fwrite_assert(&_e->id, sizeof(_e->id), stream);
}

// dump EffectEdgeDelete into stream
static void DumpEdgeDeleteEffect
(
	FILE *stream,    // stream
	const Effect *e  // effect
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    edge ID
	//    relation ID
	//    src ID
	//    dest ID
	//--------------------------------------------------------------------------

	// effect
	const EffectDeleteEdge *_e = &e->delete_edge;

	// write edge ID, relation ID, edge src node ID and edge dest node ID
	size_t n = sizeof(_e->id)         +
		       sizeof(_e->relationID) +
			   sizeof(_e->srcNodeID)  +
			   sizeof(_e->destNodeID) ;
	fwrite_assert(_e, n, stream); 
}

// dump NodeUpdateEffect into stream
static void DumpNodeUpdateEffect
(
	FILE *stream,    // stream
	const Effect *e  // effect
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    entity ID
	//    attribute id
	//    attribute value
	//--------------------------------------------------------------------------

	// effect
	const EffectUpdate *_e = (EffectUpdate*)&e->update;

	//--------------------------------------------------------------------------
	// write entity ID
	//--------------------------------------------------------------------------

	fwrite_assert(&ENTITY_GET_ID(&_e->n), sizeof(EntityID), stream);

	//--------------------------------------------------------------------------
	// write attribute ID
	//--------------------------------------------------------------------------
	
	fwrite_assert(&_e->attr_id, sizeof(Attribute_ID), stream);

	//--------------------------------------------------------------------------
	// write attribute value
	//--------------------------------------------------------------------------

	SIValue_ToBinary(stream, &_e->value);
}

// dump EdgeUpdateEffect into stream
static void DumpEdgeUpdateEffect
(
	FILE *stream,    // stream
	const Effect *e  // effect
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    edge ID
	//    relation ID
	//    src ID
	//    dest ID
	//    attribute count (=n)
	//    attributes (id,value) pair
	//--------------------------------------------------------------------------

	// effect
	const EffectUpdate *_e = (EffectUpdate*)&e->update;

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = GraphContext_GetGraph(gc);

	// entity type edge
	Edge *edge = (Edge *)&_e->e;

	//--------------------------------------------------------------------------
	// write edge ID
	//--------------------------------------------------------------------------

	fwrite_assert(&ENTITY_GET_ID(edge), sizeof(EntityID), stream);

	//--------------------------------------------------------------------------
	// write relation ID
	//--------------------------------------------------------------------------

	RelationID r = EDGE_GET_RELATION_ID(edge, g);
	fwrite_assert(&r, sizeof(RelationID), stream);

	//--------------------------------------------------------------------------
	// write src ID
	//--------------------------------------------------------------------------

	NodeID s = Edge_GetSrcNodeID(edge);
	fwrite_assert(&s, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// write dest ID
	//--------------------------------------------------------------------------

	NodeID d = Edge_GetDestNodeID(edge);
	fwrite_assert(&d, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// write attribute ID
	//--------------------------------------------------------------------------

	fwrite_assert(&_e->attr_id, sizeof(Attribute_ID), stream);

	//--------------------------------------------------------------------------
	// write attribute value
	//--------------------------------------------------------------------------

	SIValue_ToBinary(stream, &_e->value);
}

// dump CreateNodeEffect into stream
static void DumpNodeCreateEffect
(
	FILE *stream,    // stream
	const Effect *e  // effect
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// label count
	// labels
	// attribute count
	// attributes (id,value) pair
	//--------------------------------------------------------------------------
	
	Graph *g = QueryCtx_GetGraph();
	const EffectCreate *_e = &e->create;
	const Node *n = &_e->n;

	//--------------------------------------------------------------------------
	// write label count
	//--------------------------------------------------------------------------

	ushort lbl_count;
	NODE_GET_LABELS(g, n, lbl_count);
	fwrite_assert(&lbl_count, sizeof(lbl_count), stream);

	//--------------------------------------------------------------------------
	// write labels
	//--------------------------------------------------------------------------

	if(lbl_count > 0) {
		fwrite_assert(labels, sizeof(LabelID) * lbl_count, stream);
	}

	//--------------------------------------------------------------------------
	// write attribute set
	//--------------------------------------------------------------------------

	const AttributeSet attrs = GraphEntity_GetAttributes((const GraphEntity*)n);
	WriteAttributeSet(stream, attrs);
}

// dump CreateEdgeEffect into stream
static void DumpEdgeCreateEffect
(
	FILE *stream,    // stream
	const Effect *e  // effect
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
	
	Graph *g = QueryCtx_GetGraph();
	const EffectCreate *_e = &e->create;
	const Edge *edge = &_e->e;

	//--------------------------------------------------------------------------
	// write relationship type
	//--------------------------------------------------------------------------

	ushort rel_count = 1;
	fwrite_assert(&rel_count, sizeof(rel_count), stream);

	RelationID rel_id = Edge_GetRelationID(edge);
	fwrite_assert(&rel_id, sizeof(RelationID), stream);

	//--------------------------------------------------------------------------
	// write src node ID
	//--------------------------------------------------------------------------
	
	NodeID src_id = Edge_GetSrcNodeID(edge);
	fwrite_assert(&src_id, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// write dest node ID
	//--------------------------------------------------------------------------

	NodeID dest_id = Edge_GetDestNodeID(edge);
	fwrite_assert(&dest_id, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// write attribute set 
	//--------------------------------------------------------------------------

	const AttributeSet attrs = GraphEntity_GetAttributes((GraphEntity*)edge);
	WriteAttributeSet(stream, attrs);
}

// dump EffectUpdate into stream
static void DumpSetRemoveLabelsEffect
(
	FILE *stream,    // effects stream
	const Effect *e  // effect operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------
	
	const EffectLabels *_e = &e->labels;

	// write node ID
	fwrite_assert(&_e->id, sizeof(_e->id), stream); 
	
	// write labels count
	ushort lbl_count = _e->labels_count;
	fwrite_assert(&lbl_count, sizeof(lbl_count), stream); 
	
	// write label IDs
	fwrite_assert(_e->label_ids, sizeof(LabelID) * lbl_count, stream);
}

// dump AddSchemaEffect into stream
static void DumpSchemaAddEffect
(
	FILE *stream,    // stream
	const Effect *e  // effect
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    schema type
	//    schema name
	//--------------------------------------------------------------------------
	
	const EffectAddSchema *_e = &e->schema;

	//--------------------------------------------------------------------------
	// write schema type
	//--------------------------------------------------------------------------

	fwrite_assert(&_e->t, sizeof(_e->t), stream);

	//--------------------------------------------------------------------------
	// write schema name
	//--------------------------------------------------------------------------

	fwrite_string(_e->schema_name, stream);
}

// dump EffectAttrAdd into stream
static void DumpAttrAddEffect
(
	FILE *stream,    // stream
	const Effect *e  // effect
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// attribute name
	//--------------------------------------------------------------------------

	const EffectAddAttribute *_e = &e->attribute;

	//--------------------------------------------------------------------------
	// write attribute name
	//--------------------------------------------------------------------------

	fwrite_string(_e->attr_name, stream);
}


// dumps effect to stream
static void DumpEffect
(
	FILE *stream,     // effects stream
	const Effect *e   // effect to dump
) {
	// validations
	ASSERT(e      != NULL);  // effect can't be NULL
	ASSERT(stream != NULL);  // effects stream can't be NULL

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	fwrite_assert(&e->type, sizeof(EffectType), stream); 

	// encode effect
	switch(e->type) {
		case EFFECT_DELETE_NODE:
			DumpNodeDeleteEffect(stream, e);
			break;
		case EFFECT_DELETE_EDGE:
			DumpEdgeDeleteEffect(stream, e);
			break;
		case EFFECT_UPDATE_NODE:
			DumpNodeUpdateEffect(stream, e);
			break;
		case EFFECT_UPDATE_EDGE:
			DumpEdgeUpdateEffect(stream, e);
			break;
		case EFFECT_CREATE_NODE:
			DumpNodeCreateEffect(stream, e);
			break;
		case EFFECT_CREATE_EDGE:
			DumpEdgeCreateEffect(stream, e);
			break;
		case EFFECT_SET_LABELS:
			DumpSetRemoveLabelsEffect(stream, e);
			break;
		case EFFECT_REMOVE_LABELS:
			DumpSetRemoveLabelsEffect(stream, e);
			break;
		case EFFECT_ADD_SCHEMA:
			DumpSchemaAddEffect(stream, e);
			break;
		case EFFECT_ADD_ATTRIBUTE:
			DumpAttrAddEffect(stream, e);
			break;
		default:
			assert(false && "unknown effect");
			break;
	}
}

EffectLog EffectLog_New(void) {
	return (EffectLog)array_new(Effect, 0);
}

// returns number of entries in log
uint EffectLog_Length
(
	const EffectLog log  // log to query
) {
	ASSERT(log != NULL);
	return array_len(log);
}

// dump effects into buffer
u_char *Effects_Dump
(
	const EffectLog log,  // effect-log to dump
	size_t *len           // size of generated buffer
) {
	// validations
	ASSERT(log != NULL);
	ASSERT(len != NULL);

	// expecting at least one effect operation
	uint n = EffectLog_Length(log);
	if(n == 0) {
		return NULL;
	}

	//--------------------------------------------------------------------------
	// determine required effects buffer size
	//--------------------------------------------------------------------------

	size_t buff_size = ComputeBufferSize(log);
	ASSERT(buff_size > 0);

	// allocate effects buffer and treat it as a stream
	u_char *buffer = rm_malloc(sizeof(u_char) * buff_size);
	FILE *stream = fmemopen(buffer, buff_size, "w");

	//--------------------------------------------------------------------------
	// encode effects header
	//--------------------------------------------------------------------------

	// effects version
	uint8_t v = EFFECTS_VERSION;
	fwrite_assert(&v, sizeof(uint8_t), stream);

	//--------------------------------------------------------------------------
	// encode effects
	//--------------------------------------------------------------------------

	// dump effects to stream
	for(uint i = 0; i < n; i++) {
		const Effect *e = log + i;
		DumpEffect(stream, e);
	}

	// we should have reached end of buffer
	ASSERT(ftell(stream) == buff_size);

	// close stream
	fclose(stream);

	// set output length and return buffer
	*len = buff_size;
	return buffer;
}

// add an enffect to effect-log
static inline void _EffectLog_AddEffect
(
	EffectLog *log,  // effect log
	Effect *op       // effect
) {
	ASSERT(op  != NULL);
	ASSERT(log != NULL && *log != NULL);

	array_append(*log, *op);
}

//------------------------------------------------------------------------------
// effects creation
//------------------------------------------------------------------------------

// node creation effect
void EffectLog_CreateNode
(
	EffectLog *log,   // effect log
	const Node *node  // node created
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(node != NULL);

	Effect op;

	op.type     = EFFECT_CREATE_NODE;
	op.create.n = *node;

	_EffectLog_AddEffect(log, &op);
}

// edge creation effect
void EffectLog_CreateEdge
(
	EffectLog *log,   // effect log
	const Edge *edge  // edge created
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(edge != NULL);

	Effect op;

	op.type     = EFFECT_CREATE_EDGE;
	op.create.e = *edge;

	_EffectLog_AddEffect(log, &op);
}

// node deletion effect
void EffectLog_DeleteNode
(
	EffectLog *log,   // effect log
	const Node *node  // node deleted
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(node != NULL);

	Effect op;

	op.type           = EFFECT_DELETE_NODE;
	op.delete_node.id = ENTITY_GET_ID(node);

	_EffectLog_AddEffect(log, &op);
}

// edge deletion effect
void EffectLog_DeleteEdge
(
	EffectLog *log,  // effect log
	const Edge *edge // edge deleted
) {
	ASSERT(log != NULL && *log != NULL);
	ASSERT(edge != NULL);

	Effect op;

	op.type                   = EFFECT_DELETE_EDGE;
	op.delete_edge.id         = ENTITY_GET_ID(edge);
	op.delete_edge.srcNodeID  = Edge_GetSrcNodeID(edge);
	op.delete_edge.destNodeID = Edge_GetDestNodeID(edge);
	op.delete_edge.relationID = Edge_GetRelationID(edge);

	_EffectLog_AddEffect(log, &op);
}

// entity update effect
void EffectLog_UpdateEntity
(
	EffectLog *log,              // effect log
	const GraphEntity *entity,   // updated entity ID
	Attribute_ID attr_id,        // updated attribute ID
	SIValue value,               // value
	GraphEntityType entity_type  // entity type
) {
	ASSERT(log != NULL && *log != NULL);

	EffectType t = entity_type == GETYPE_NODE
		? EFFECT_UPDATE_NODE : EFFECT_UPDATE_EDGE;

	Effect op;

	op.type               = t;
	op.update.value       = value;
	op.update.attr_id     = attr_id;
	op.update.entity_type = entity_type;

	if(entity_type == GETYPE_NODE) {
		op.update.n = *(Node *)entity;
	} else {
		op.update.e = *(Edge *)entity;
	}

	_EffectLog_AddEffect(log, &op);
}

// node add label effect
static void _EffectLog_ModifyLabels
(
	EffectLog *log,            // effect log
	EffectType t,              // add/remove labels
	const Node *node,          // updated node
	const LabelID *label_ids,  // added labels
	size_t labels_count        // number of removed labels
) {
	Effect op;

	// TODO: is it necessary to copy `label_ids` ?
	size_t n = sizeof(LabelID) * labels_count;

	op.type                = t;
	op.labels.id           = ENTITY_GET_ID(node);
	op.labels.label_ids    = rm_malloc(n);
	op.labels.labels_count = labels_count;

	memcpy(op.labels.label_ids, label_ids, n);

	_EffectLog_AddEffect(log, &op);
}

// node add label effect
void EffectLog_AddLabels
(
	EffectLog *log,            // effect log
	const Node *node,          // updated node
	const LabelID *label_ids,  // added labels
	size_t labels_count        // number of removed labels
) {
	ASSERT(log != NULL);
	ASSERT(node != NULL);
	ASSERT(label_ids != NULL);
	ASSERT(labels_count > 0);

	_EffectLog_ModifyLabels(log, EFFECT_SET_LABELS, node, label_ids,
			labels_count);
}

// node remove label effect
void EffectLog_RemoveLabels
(
	EffectLog *log,            // effect log
	const Node *node,          // updated node
	const LabelID *label_ids,  // removed labels
	size_t labels_count        // number of removed labels
) {
	ASSERT(log != NULL);
	ASSERT(node != NULL);
	ASSERT(label_ids != NULL);
	ASSERT(labels_count > 0);

	_EffectLog_ModifyLabels(log, EFFECT_REMOVE_LABELS, node, label_ids,
			labels_count);
}

// schema addition effect
void EffectLog_AddSchema
(
	EffectLog *log,           // effect log
	const char *schema_name,  // id of the schema
	SchemaType t              // type of the schema
) {
	ASSERT(log != NULL);
	ASSERT(schema_name != NULL);

	Effect op;

	op.type               = EFFECT_ADD_SCHEMA;
	op.schema.t           = t;
	op.schema.schema_name = schema_name;

	_EffectLog_AddEffect(log, &op);
}

void EffectLog_AddAttribute
(
	EffectLog *log,   // effect log
	const char *attr  // attribute name
) {
	ASSERT(log != NULL);
	ASSERT(attr != NULL);

	Effect op;

	op.type = EFFECT_ADD_ATTRIBUTE;
	op.attribute.attr_name = attr;

	_EffectLog_AddEffect(log, &op);
}

void EffectLog_FreeEffect
(
	Effect *op
) {
	ASSERT(op != NULL);

	switch(op->type) {
		// free fall all the way down!
		case EFFECT_UNKNOWN:
		case EFFECT_UPDATE_NODE:
		case EFFECT_UPDATE_EDGE:
		case EFFECT_CREATE_NODE:
		case EFFECT_CREATE_EDGE:
		case EFFECT_DELETE_NODE:
		case EFFECT_DELETE_EDGE:
		case EFFECT_SET_LABELS:
		case EFFECT_REMOVE_LABELS:
		case EFFECT_ADD_SCHEMA:
		case EFFECT_ADD_ATTRIBUTE:
			break;
		default:
			ASSERT(false);
	}
}

void EffectLog_Free
(
	EffectLog log
) {
	ASSERT(log != NULL);

	// free each effect
	uint n = EffectLog_Length(log);
	for (uint i = 0; i < n; i++) {
		Effect *op = log + i;
		EffectLog_FreeEffect(op);
	}

	array_free(log);
}

