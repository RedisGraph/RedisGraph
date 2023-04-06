/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "effects.h"
#include "../query_ctx.h"
#include "../undo_log/undo_log.h"

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

// convert UndoNodeDelete into a NodeDelete effect
static void EffectFromUndoNodeDelete
(
	FILE *stream,     // effects stream
	const UndoOp *op  // undo operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//--------------------------------------------------------------------------
	
	// undo operation
	const UndoDeleteNodeOp *_op = (const UndoDeleteNodeOp*)op;

	// effect type
	EffectType t = EFFECT_DELETE_NODE;

	// write effect type
	fwrite_assert(&t, sizeof(t), stream); 

	// write node ID
	fwrite_assert(&_op->id, sizeof(_op->id), stream);
}

// convert UndoEdgeDelete into a EdgeDelete effect
static void EffectFromUndoEdgeDelete
(
	FILE *stream,     // effects stream
	const UndoOp *op  // undo operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    edge ID
	//    relation ID
	//    src ID
	//    dest ID
	//--------------------------------------------------------------------------

	// undo operation
	const UndoDeleteEdgeOp *_op = (const UndoDeleteEdgeOp*)op;

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

// convert UndoUpdate into a Update effect
static void EffectFromNodeUpdate
(
	FILE *stream,           // effects stream
	const UndoUpdateOp *op  // undo operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    entity ID
	//    attribute id
	//    attribute value
	//--------------------------------------------------------------------------
	
	// entity type node
	Node *n = (Node*)&op->n;

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_UPDATE_NODE;
	fwrite_assert(&t, sizeof(EffectType), stream); 

	//--------------------------------------------------------------------------
	// write entity ID
	//--------------------------------------------------------------------------

	fwrite_assert(&ENTITY_GET_ID(n), sizeof(EntityID), stream);

	//--------------------------------------------------------------------------
	// write attribute ID
	//--------------------------------------------------------------------------

	GraphContext *gc = QueryCtx_GetGraphCtx();
	fwrite_assert(&op->attr_id, sizeof(Attribute_ID), stream);

	//--------------------------------------------------------------------------
	// write attribute value
	//--------------------------------------------------------------------------

	SIValue *v = GraphEntity_GetProperty((GraphEntity*)n, op->attr_id);
	if(v == ATTRIBUTE_NOTFOUND) {
		// attribute been deleted
		*v = SI_NullVal();
	}

	SIValue_ToBinary(stream, v);
}

// convert UndoUpdate into a Update effect
static void EffectFromEdgeUpdate
(
	FILE *stream,           // effects stream
	const UndoUpdateOp *op  // undo operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    edge ID
	//    relation ID
	//    src ID
	//    dest ID
	//    attribute id
	//    attribute value
	//--------------------------------------------------------------------------

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = GraphContext_GetGraph(gc);

	// entity type edge
	Edge *e = (Edge*)&op->e;

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_UPDATE_EDGE;
	fwrite_assert(&t, sizeof(EffectType), stream);

	//--------------------------------------------------------------------------
	// write edge ID
	//--------------------------------------------------------------------------

	fwrite_assert(&ENTITY_GET_ID(e), sizeof(EntityID), stream);

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

	SIValue *v = GraphEntity_GetProperty((GraphEntity*)e, op->attr_id);
	if(v == ATTRIBUTE_NOTFOUND) {
		// attribute been deleted
		*v = SI_NullVal();
	}

	SIValue_ToBinary(stream, v);
}

// convert UndoUpdate into a Update effect
static void EffectFromUndoUpdate
(
	FILE *stream,     // effects stream
	const UndoOp *op  // undo operation to convert
) {
	// undo operation
	const UndoUpdateOp *_op = (const UndoUpdateOp*)op;

	if(_op->entity_type == GETYPE_NODE) {
		EffectFromNodeUpdate(stream, _op);
	} else {
		EffectFromEdgeUpdate(stream, _op);
	}
}

// convert UndoCreateNodeOp into a CreateNode effect
static void EffectFromUndoNodeCreate
(
	FILE *stream,      // effects stream
	const UndoOp *op   // undo operation to convert
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
	const UndoCreateOp *_op = (const UndoCreateOp*)op;
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

// convert UndoCreateEdgeOp into a CreateEdge effect
static void EffectFromUndoEdgeCreate
(
	FILE *stream,     // effects stream
	const UndoOp *op  // undo operation to convert
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
	const UndoCreateOp *_op = (const UndoCreateOp*)op;
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

// convert UndoAttrAdd into a AddAttr effect
static void EffectFromUndoAttrAdd
(
	FILE *stream,     // effects stream
	const UndoOp *op  // undo operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// attribute name
	//--------------------------------------------------------------------------

	const UndoAddAttributeOp *_op = (const UndoAddAttributeOp*)op;

	//--------------------------------------------------------------------------
	// write effect type
	//--------------------------------------------------------------------------

	EffectType t = EFFECT_ADD_ATTRIBUTE;
	fwrite_assert(&t, sizeof(EffectType), stream);

	//--------------------------------------------------------------------------
	// write attribute name
	//--------------------------------------------------------------------------

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Attribute_ID attr_id = _op->attribute_id;
	const char *attr_name = GraphContext_GetAttributeString(gc, attr_id);
	fwrite_string(attr_name, stream);
}

// convert UndoUpdate into a Update effect
static void EffectFromUndoSetRemoveLabels
(
	FILE *stream,     // effects stream
	const UndoOp *op, // undo operation to convert
	EffectType t      // effect type SET/REMOVE LABELS
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------
	
	ASSERT(t == EFFECT_SET_LABELS || t == EFFECT_REMOVE_LABELS);

	const UndoLabelsOp *_op = (const UndoLabelsOp*)op;
	ushort lbl_count = _op->labels_count;

	// write effect type
	fwrite_assert(&t, sizeof(EffectType), stream); 

	// write node ID
	const Node *n = &_op->node;
	EntityID id = ENTITY_GET_ID(n);
	fwrite_assert(&id, sizeof(id), stream); 
	
	// write labels count
	fwrite_assert(&lbl_count, sizeof(lbl_count), stream); 
	
	// write label IDs
	fwrite_assert(_op->label_ids, sizeof(LabelID) * lbl_count, stream);
}

// convert UndoAddSchema into a AddSchema effect
static void EffectFromUndoSchemaAdd
(
	FILE *stream,     // effects stream
	const UndoOp *op  // undo operation to convert
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    schema type
	//    schema name
	//--------------------------------------------------------------------------
	
	GraphContext *gc = QueryCtx_GetGraphCtx();
	const UndoAddSchemaOp *_op = (const UndoAddSchemaOp *)op;
	Schema *schema = GraphContext_GetSchemaByID(gc, _op->schema_id, _op->t);
	ASSERT(schema != NULL);

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

	const char *schema_name = Schema_GetName(schema);
	fwrite_string(schema_name, stream);
}

// convert undo-operation into an effect
// and write effect to stream
static void EffectFromUndoOp
(
	FILE *stream,     // effects stream
	const UndoOp *op  // undo op to convert into an effect
) {
	// validations
	ASSERT(op     != NULL);  // undo-op can't be NULL
	ASSERT(stream != NULL);  // effects stream can't be NULL

	// encode undo-op as an effect
	switch(op->type) {
		case UNDO_DELETE_NODE:
			EffectFromUndoNodeDelete(stream, op);
			break;
		case UNDO_DELETE_EDGE:
			EffectFromUndoEdgeDelete(stream, op);
			break;
		case UNDO_UPDATE:
			EffectFromUndoUpdate(stream, op);
			break;
		case UNDO_CREATE_NODE:
			EffectFromUndoNodeCreate(stream, op);
			break;
		case UNDO_CREATE_EDGE:
			EffectFromUndoEdgeCreate(stream, op);
			break;
		case UNDO_ADD_ATTRIBUTE:
			EffectFromUndoAttrAdd(stream, op);
			break;
		case UNDO_SET_LABELS:
			EffectFromUndoSetRemoveLabels(stream, op, EFFECT_SET_LABELS);
			break;
		case UNDO_REMOVE_LABELS:
			EffectFromUndoSetRemoveLabels(stream, op, EFFECT_REMOVE_LABELS);
			break;
		case UNDO_ADD_SCHEMA:
			EffectFromUndoSchemaAdd(stream, op);
			break;
		default:
			assert(false && "unknown effect");
			break;
	}
}

// create a list of effects from the undo-log
u_char *Effects_FromUndoLog
(
	UndoLog log,  // undo-log to convert into effects buffer
	size_t *len   // size of generated effects buffer
) {
	// validations
	ASSERT(log != NULL);  // undo-log can't be NULL

	// expecting at least one undo operation
	uint n = UndoLog_Length(log);
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
		UndoOp *op = log + i;

		// convert undo-op into an effect and write it to stream
		EffectFromUndoOp(stream, op);

		// free undo-op
		UndoLog_FreeOp(op);
	}

	// we should have reached end of buffer
	ASSERT(ftell(stream) == buff_size);

	// close stream
	fclose(stream);

	// clear undo-log
	UndoLog_Clear(log);

	// set output length and return buffer
	*len = buff_size;
	return buffer;
}

