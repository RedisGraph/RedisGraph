/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "effects.h"
#include "../query_ctx.h"
#include "../effects/effects.h"
#include "../graph/entities/attribute_set.h"

static size_t ComputeCreateNodeSize
(
	const Effect *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// label count
	// labels
	// attribute count
	// attributes (id,value) pair
	//--------------------------------------------------------------------------

	// effect operation
	Graph *g = QueryCtx_GetGraph();
	const EffectCreate *_op = (const EffectCreate*)op;

	// entity-type node/edge
	const Node *n = &_op->n;
	
	// get number of labels
	uint lbl_count;
	NODE_GET_LABELS(g, n, lbl_count);

	// get number of attributes
	const AttributeSet attrs = GraphEntity_GetAttributes((const GraphEntity*)n);
	ushort attr_count = ATTRIBUTE_SET_COUNT(attrs);

	//--------------------------------------------------------------------------
	// compute effect size
	//--------------------------------------------------------------------------

	size_t s = sizeof(EffectType)                 +  // effect type
		       sizeof(ushort)                     +  // label count
			   lbl_count * sizeof(LabelID)        +  // labels
			   sizeof(ushort)                     +  // attribute count
			   attr_count * sizeof(Attribute_ID);    // attribute IDs

	// compute attribute-set size
	for(ushort i = 0; i < attr_count; i++) {
		// compute attribute size
		Attribute_ID attr_id;
		SIValue attr = AttributeSet_GetIdx(attrs, i, &attr_id);
		s += SIValue_BinarySize(&attr);  // attribute value
	}

	return s;
}

static size_t ComputeCreateEdgeSize
(
	const Effect *op
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

	// effect operation
	Graph *g = QueryCtx_GetGraph();
	const EffectCreate *_op = &op->create;

	const Edge *e = &_op->e;

	// get number of attributes
	const AttributeSet attrs = GraphEntity_GetAttributes((const GraphEntity*)e);
	ushort attr_count = ATTRIBUTE_SET_COUNT(attrs);

	//--------------------------------------------------------------------------
	// compute effect size
	//--------------------------------------------------------------------------

	size_t s = sizeof(EffectType)                 +  // effect type
			   sizeof(ushort)                     +  // relationship count
			   sizeof(RelationID)                 +  // relationship
			   sizeof(NodeID)                     +  // src node ID
			   sizeof(NodeID)                     +  // dest node ID
			   sizeof(ushort)                     +  // attribute count
			   attr_count * sizeof(Attribute_ID);    // attribute IDs

	// compute attribute-set size
	for(ushort i = 0; i < attr_count; i++) {
		// compute attribute size
		Attribute_ID attr_id;
		SIValue attr = AttributeSet_GetIdx(attrs, i, &attr_id);
		s += SIValue_BinarySize(&attr);  // attribute value
	}

	return s;
}

// compute required update-effect size for undo-add-attribute operation
static size_t ComputeAttrAddSize
(
	const Effect *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// attribute name
	//--------------------------------------------------------------------------

	GraphContext *gc = QueryCtx_GetGraphCtx();
	const EffectAddAttribute *_op = &op->attribute;

	// compute effect byte size
	size_t s = sizeof(EffectType) +         // effect type
		       sizeof(size_t) +             // attribute name length
			   strlen(_op->attr_name) + 1;  // attribute name

	return s;
}

// compute required update-effect size for effect-set-labels operation
static size_t ComputeSetLabelSize
(
	const Effect *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------
	
	const EffectLabels *_op = &op->labels;

	size_t s = sizeof(EffectType) +                  // effect type
		       sizeof(EntityID)   +                  // node ID
		       sizeof(ushort)     +                  // number of labels
			   sizeof(LabelID) * _op->labels_count;  // labels IDs

	return s;
}

// compute required update-effect size for effect-remove-labels operation
static size_t ComputeRemoveLabelSize
(
	const Effect *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------
	
	// both remove labels and set labels share the same format
	return ComputeSetLabelSize(op);
}

static size_t ComputeSchemaAddSize
(
	const Effect *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    schema type
	//    schema name
	//--------------------------------------------------------------------------
	
	const EffectAddSchema *_op = &op->schema;

	size_t s = sizeof(EffectType) +           // effect type
		       sizeof(SchemaType) +           // schema type
			   sizeof(size_t)     +           // schema name length
			   strlen(_op->schema_name) + 1;  // schema name

	return s;
}

// compute required update-effect size for effect-edge-update operation
static size_t ComputeEdgeUpdateSize
(
	const EffectUpdate *op
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

	// compute effect byte size
	size_t s = sizeof(EffectType)                +  // effect type
			   sizeof(EntityID)                  +  // edge ID
			   sizeof(RelationID)                +  // relation ID
			   sizeof(NodeID)                    +  // src node ID
			   sizeof(NodeID)                    +  // dest node ID
			   sizeof(Attribute_ID)              +  // attribute ID
			   SIValue_BinarySize(&op->value);      // attribute value

	return s;
}

// compute required update-effect size for effect-node-update operation
static size_t ComputeNodeUpdateSize
(
	const EffectUpdate *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    entity ID
	//    attribute count (=n)
	//    attribute (id, value) pairs
	//--------------------------------------------------------------------------

	// compute effect byte size
	size_t s = sizeof(EffectType)                +  // effect type
			   sizeof(NodeID)                    +  // node ID
			   sizeof(Attribute_ID)              +  // attribute ID
			   SIValue_BinarySize(&op->value);      // attribute value

	return s;
}

// compute required effects buffer byte size from effect-log
size_t ComputeBufferSize
(
	const EffectLog effect_log
) {
	ASSERT(effect_log != NULL);

	size_t s = 0;  // effects-buffer required size in bytes
	uint n = EffectLog_Length(effect_log);  // number of effect entries

	// make sure effect-log is not empty
	ASSERT(n > 0);

	// account for effect version
	s += sizeof(uint8_t);

	// compute effect size from each effect operation
	for(uint i = 0; i < n; i++) {
		const Effect *e = effect_log + i;
		switch(e->type) {
			case EFFECT_DELETE_NODE:
				// DeleteNode effect size
				s += sizeof(EffectType) +
					 fldsiz(EffectDeleteNode, id);
				break;
			case EFFECT_DELETE_EDGE:
				// DeleteEdge effect size
				s += sizeof(EffectType)                   +
					 fldsiz(EffectDeleteEdge, id)         +
					 fldsiz(EffectDeleteEdge, relationID) +
					 fldsiz(EffectDeleteEdge, srcNodeID)  +
					 fldsiz(EffectDeleteEdge, destNodeID);
				break;
			case EFFECT_UPDATE_NODE:
				// Update effect size
				s += ComputeNodeUpdateSize(&e->update);
				break;
			case EFFECT_UPDATE_EDGE:
				// Update effect size
				s += ComputeEdgeUpdateSize(&e->update);
				break;
			case EFFECT_CREATE_NODE:
				s += ComputeCreateNodeSize(e);
				break;
			case EFFECT_CREATE_EDGE:
				s += ComputeCreateEdgeSize(e);
				break;
			case EFFECT_SET_LABELS:
				s += ComputeSetLabelSize(e);
				break;
			case EFFECT_REMOVE_LABELS:
				s += ComputeRemoveLabelSize(e);
				break;
			case EFFECT_ADD_SCHEMA:
				s += ComputeSchemaAddSize(e);
				break;
			case EFFECT_ADD_ATTRIBUTE:
				s += ComputeAttrAddSize(e);
				break;
			default:
				assert(false && "unknown effect");
				break;
		}
	}

	return s;
}

