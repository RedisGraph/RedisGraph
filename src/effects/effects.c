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

// convert EffectNodeDelete into a NodeDelete effect
static void EffectFromEffectNodeDelete
(
	FILE *stream,       // effects stream
	const Effect *op  // effect operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//--------------------------------------------------------------------------
	
	// effect operation
	const EffectDeleteNode *_op = &op->delete_node;

	// effect type
	EffectType t = EFFECT_DELETE_NODE;

	// write effect type
	fwrite_assert(&t, sizeof(t), stream); 

	// write node ID
	fwrite_assert(&_op->id, sizeof(_op->id), stream);
}

// convert EffectEdgeDelete into a EdgeDelete effect
static void EffectFromEffectEdgeDelete
(
	FILE *stream,       // effects stream
	const Effect *op  // effect operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    edge ID
	//    relation ID
	//    src ID
	//    dest ID
	//--------------------------------------------------------------------------

	// effect operation
	const EffectDeleteEdge *_op = &op->delete_edge;

	// effect type
	EffectType t = EFFECT_DELETE_EDGE;

	// write effect type
	fwrite_assert(&t, sizeof(t), stream); 

	// write edge ID
	fwrite_assert(&_op->id, sizeof(_op->id), stream); 

	// write relation ID
	fwrite_assert(&_op->relationID, sizeof(_op->relationID), stream); 

	// write edge src node ID
	fwrite_assert(&_op->srcNodeID, sizeof(_op->srcNodeID), stream); 

	// write edge dest node ID
	fwrite_assert(&_op->destNodeID, sizeof(_op->destNodeID), stream); 
}

// convert EffectUpdate into a Update effect
static void EffectFromNodeUpdate
(
	FILE *stream,             // effects stream
	const EffectUpdate *op  // effect operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    entity ID
	//    attribute id
	//    attribute value
	//--------------------------------------------------------------------------

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_UPDATE_NODE;
	fwrite_assert(&t, sizeof(EffectType), stream); 

	//--------------------------------------------------------------------------
	// write entity ID
	//--------------------------------------------------------------------------

	fwrite_assert(&ENTITY_GET_ID(op->entity), sizeof(EntityID), stream);

	//--------------------------------------------------------------------------
	// write attribute ID
	//--------------------------------------------------------------------------
	
	fwrite_assert(&op->attr_id, sizeof(Attribute_ID), stream);

	//--------------------------------------------------------------------------
	// write attribute value
	//--------------------------------------------------------------------------

	SIValue_ToBinary(stream, &op->value);
}

// convert EffectUpdate into a Update effect
static void EffectFromEdgeUpdate
(
	FILE *stream,             // effects stream
	const EffectUpdate *op  // effect operation to convert
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

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = GraphContext_GetGraph(gc);

	// entity type edge
	Edge *e = (Edge*)&op->entity;

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_UPDATE_EDGE;
	fwrite_assert(&t, sizeof(EffectType), stream);

	//--------------------------------------------------------------------------
	// write edge ID
	//--------------------------------------------------------------------------

	fwrite_assert(&ENTITY_GET_ID(op->entity), sizeof(EntityID), stream);

	//--------------------------------------------------------------------------
	// write relation ID
	//--------------------------------------------------------------------------

	RelationID r = EDGE_GET_RELATION_ID(e, g);
	fwrite_assert(&r, sizeof(RelationID), stream);

	//--------------------------------------------------------------------------
	// write src ID
	//--------------------------------------------------------------------------

	NodeID s = Edge_GetSrcNodeID(e);
	fwrite_assert(&s, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// write dest ID
	//--------------------------------------------------------------------------

	NodeID d = Edge_GetDestNodeID(e);
	fwrite_assert(&d, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// write attribute ID
	//--------------------------------------------------------------------------

	fwrite_assert(&op->attr_id, sizeof(Attribute_ID), stream);

	//--------------------------------------------------------------------------
	// write attribute value
	//--------------------------------------------------------------------------

	SIValue_ToBinary(stream, &op->value);
}

// convert EffectCreateNodeOp into a CreateNode effect
static void EffectFromEffectNodeCreate
(
	FILE *stream,        // effects stream
	const Effect *op   // effect operation to convert
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
	const EffectCreate *_op = &op->create;
	const Node *n = &_op->n;

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	EffectType et = EFFECT_CREATE_NODE;
	fwrite_assert(&et, sizeof(EffectType), stream); 

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

// convert EffectCreateEdgeOp into a CreateEdge effect
static void EffectFromEffectEdgeCreate
(
	FILE *stream,       // effects stream
	const Effect *op  // effect operation to convert
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
	const EffectCreate *_op = &op->create;
	const Edge *e = &_op->e;

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	EffectType et = EFFECT_CREATE_EDGE;
	fwrite_assert(&et, sizeof(EffectType), stream); 

	//--------------------------------------------------------------------------
	// write relationship type
	//--------------------------------------------------------------------------

	ushort rel_count = 1;
	fwrite_assert(&rel_count, sizeof(rel_count), stream);

	RelationID rel_id = Edge_GetRelationID(e);
	fwrite_assert(&rel_id, sizeof(RelationID), stream);

	//--------------------------------------------------------------------------
	// write src node ID
	//--------------------------------------------------------------------------
	
	NodeID src_id = Edge_GetSrcNodeID(e);
	fwrite_assert(&src_id, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// write dest node ID
	//--------------------------------------------------------------------------

	NodeID dest_id = Edge_GetDestNodeID(e);
	fwrite_assert(&dest_id, sizeof(NodeID), stream);

	//--------------------------------------------------------------------------
	// write attribute set 
	//--------------------------------------------------------------------------

	const AttributeSet attrs = GraphEntity_GetAttributes((const GraphEntity*)e);
	WriteAttributeSet(stream, attrs);
}

// convert EffectAttrAdd into a AddAttr effect
static void EffectFromEffectAttrAdd
(
	FILE *stream,     // effects stream
	const Effect *op  // effect operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// attribute name
	//--------------------------------------------------------------------------

	const EffectAddAttribute *_op = &op->attribute;

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_ADD_ATTRIBUTE;
	fwrite_assert(&t, sizeof(EffectType), stream);

	//--------------------------------------------------------------------------
	// write attribute name
	//--------------------------------------------------------------------------

	const char *attr_name = _op->attr_name;
	fwrite_string(attr_name, stream);
}

// convert EffectUpdate into a Update effect
static void EffectFromEffectSetRemoveLabels
(
	FILE *stream,       // effects stream
	const Effect *op, // effect operation to convert
	EffectType t        // effect type SET/REMOVE LABELS
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------
	
	ASSERT(t == EFFECT_SET_LABELS || t == EFFECT_REMOVE_LABELS);

	const EffectLabels *_op = &op->labels;
	ushort lbl_count = _op->labels_count;

	// write effect type
	fwrite_assert(&t, sizeof(EffectType), stream); 

	// write node ID
	fwrite_assert(&_op->id, sizeof(_op->id), stream); 
	
	// write labels count
	fwrite_assert(&lbl_count, sizeof(lbl_count), stream); 
	
	// write label IDs
	fwrite_assert(_op->label_ids, sizeof(LabelID) * lbl_count, stream);
}

// convert EffectAddSchema into a AddSchema effect
static void EffectFromEffectSchemaAdd
(
	FILE *stream,     // effects stream
	const Effect *op  // effect operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    schema type
	//    schema name
	//--------------------------------------------------------------------------
	
	const EffectAddSchema *_op = &op->schema;

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_ADD_SCHEMA;
	fwrite_assert(&t, sizeof(EffectType), stream);

	//--------------------------------------------------------------------------
	// write schema type
	//--------------------------------------------------------------------------

	fwrite_assert(&_op->t, sizeof(_op->t), stream);

	//--------------------------------------------------------------------------
	// write schema name
	//--------------------------------------------------------------------------

	const char *schema_name = _op->schema_name;
	fwrite_string(schema_name, stream);
}

// convert effect-operation into an effect
// and write effect to stream
static void EffectFromEffect
(
	FILE *stream,       // effects stream
	const Effect *op  // effect op to convert into an effect
) {
	// validations
	ASSERT(op     != NULL);  // effect-op can't be NULL
	ASSERT(stream != NULL);  // effects stream can't be NULL

	// encode effect-op as an effect
	switch(op->type) {
		case EFFECT_DELETE_NODE:
			EffectFromEffectNodeDelete(stream, op);
			break;
		case EFFECT_DELETE_EDGE:
			EffectFromEffectEdgeDelete(stream, op);
			break;
		case EFFECT_UPDATE_NODE:
			EffectFromNodeUpdate(stream, &op->update);
			break;
		case EFFECT_UPDATE_EDGE:
			EffectFromEdgeUpdate(stream, &op->update);
			break;
		case EFFECT_CREATE_NODE:
			EffectFromEffectNodeCreate(stream, op);
			break;
		case EFFECT_CREATE_EDGE:
			EffectFromEffectEdgeCreate(stream, op);
			break;
		case EFFECT_SET_LABELS:
			EffectFromEffectSetRemoveLabels(stream, op, EFFECT_SET_LABELS);
			break;
		case EFFECT_REMOVE_LABELS:
			EffectFromEffectSetRemoveLabels(stream, op, EFFECT_REMOVE_LABELS);
			break;
		case EFFECT_ADD_SCHEMA:
			EffectFromEffectSchemaAdd(stream, op);
			break;
		case EFFECT_ADD_ATTRIBUTE:
			EffectFromEffectAttrAdd(stream, op);
			break;
		default:
			assert(false && "unknown effect");
			break;
	}
}

// create a list of effects from the effect-log
u_char *Effects_FromEffectLog
(
	EffectLog log,  // effect-log to convert into effects buffer
	size_t *len   // size of generated effects buffer
) {
	// validations
	ASSERT(log != NULL);  // effect-log can't be NULL

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

	for(uint i = 0; i < n; i++) {
		Effect *op = log + i;

		// convert effect-op into an effect and write it to stream
		EffectFromEffect(stream, op);

		// free effect
		EffectLog_FreeEffect(op);
	}

	// we should have reached end of buffer
	ASSERT(ftell(stream) == buff_size);

	// close stream
	fclose(stream);

	// set output length and return buffer
	*len = buff_size;
	return buffer;
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

// add an operation to effect log
static inline void _EffectLog_AddOperation
(
	EffectLog *log,  // effect log
	Effect *op       // effect
) {
	ASSERT(op  != NULL);
	ASSERT(log != NULL && *log != NULL);

	array_append(*log, *op);
}

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

	_EffectLog_AddOperation(log, &op);
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

	_EffectLog_AddOperation(log, &op);
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

	_EffectLog_AddOperation(log, &op);
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

	_EffectLog_AddOperation(log, &op);
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
	op.update.entity      = entity;
	op.update.attr_id     = attr_id;
	op.update.entity_type = entity_type;

	_EffectLog_AddOperation(log, &op);
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

	Effect op;

	size_t n = sizeof(LabelID) * labels_count;

	op.type                = EFFECT_SET_LABELS;
	op.labels.id           = ENTITY_GET_ID(node);
	op.labels.label_ids    = rm_malloc(n);
	op.labels.labels_count = labels_count;

	memcpy(op.labels.label_ids, label_ids, n);

	_EffectLog_AddOperation(log, &op);
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

	Effect op;

	size_t n = sizeof(LabelID) * labels_count;

	op.type                = EFFECT_REMOVE_LABELS;
	op.labels.id           = ENTITY_GET_ID(node);
	op.labels.label_ids    = rm_malloc(n);
	op.labels.labels_count = labels_count;

	memcpy(op.labels.label_ids, label_ids, n);

	_EffectLog_AddOperation(log, &op);
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

	_EffectLog_AddOperation(log, &op);
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
	_EffectLog_AddOperation(log, &op);
}

void EffectLog_FreeEffect
(
	Effect *op
) {
	ASSERT(op != NULL);

	switch(op->type) {
		case EFFECT_UPDATE_NODE:
		case EFFECT_UPDATE_EDGE:
			break;
		case EFFECT_CREATE_NODE:
			break;
		case EFFECT_CREATE_EDGE:
			break;
		case EFFECT_DELETE_NODE:
			break;
		case EFFECT_DELETE_EDGE:
			break;
		case EFFECT_SET_LABELS:
		case EFFECT_REMOVE_LABELS:
			break;
		case EFFECT_ADD_SCHEMA:
			break;
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
	uint count = array_len(log);
	for (uint i = 0; i < count; i++) {
		Effect *op = log + i;
		EffectLog_FreeEffect(op);
	}

	array_free(log);
}

