/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "effects.h"
#include "../query_ctx.h"
#include "../undo_log/undo_log.h"

static size_t ComputeCreateNodeSize
(
	const UndoOp *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// label count
	// labels
	// attribute count
	// attributes (id,value) pair
	//--------------------------------------------------------------------------

	// undo operation
	Graph *g = QueryCtx_GetGraph();
	const UndoCreateOp *_op = (const UndoCreateOp*)op;

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
	const UndoOp *op
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

	// undo operation
	Graph *g = QueryCtx_GetGraph();
	const UndoCreateOp *_op = (const UndoCreateOp*)op;

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
	const UndoOp *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	// effect type
	// attribute name
	//--------------------------------------------------------------------------

	GraphContext *gc = QueryCtx_GetGraphCtx();
	const UndoAddAttributeOp *_op = (const UndoAddAttributeOp*)op;

	// get added attribute name
	Attribute_ID attr_id  = _op->attribute_id;
	const char *attr_name = GraphContext_GetAttributeString(gc, attr_id);

	// compute effect byte size
	size_t s = sizeof(EffectType) +
		       sizeof(size_t) +
			   strlen(attr_name) + 1;

	return s;
}

// compute required update-effect size for undo-set-labels operation
static size_t ComputeSetLabelSize
(
	const UndoOp *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    node ID
	//    labels count
	//    label IDs
	//--------------------------------------------------------------------------
	
	const UndoLabelsOp *_op = (const UndoLabelsOp*)op;

	size_t s = sizeof(EffectType) +
		       sizeof(EntityID)   +
		       sizeof(ushort)     +
			   sizeof(LabelID) * _op->labels_count;

	return s;
}

// compute required update-effect size for undo-remove-labels operation
static size_t ComputeRemoveLabelSize
(
	const UndoOp *op
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
	const UndoOp *op
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

	size_t s = sizeof(EffectType) +
		       sizeof(SchemaType) +
			   sizeof(size_t)     +
			   strlen(Schema_GetName(schema)) + 1;

	return s;
}

// compute required update-effect size for undo-edge-update operation
static size_t ComputeEdgeUpdateSize
(
	const UndoUpdateOp *op
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

	const Edge *e = &op->e;

	// get updated value
	SIValue *v = GraphEntity_GetProperty((GraphEntity*)e, op->attr_id);

	// compute effect byte size
	size_t s = sizeof(EffectType)                +  // effect type
			   sizeof(EntityID)                  +  // edge ID
			   sizeof(RelationID)                +  // relation ID
			   sizeof(NodeID)                    +  // src node ID
			   sizeof(NodeID)                    +  // dest node ID
			   sizeof(Attribute_ID)              +  // attribute ID
			   SIValue_BinarySize(v);               // attribute value

	return s;
}

// compute required update-effect size for undo-node-update operation
static size_t ComputeNodeUpdateSize
(
	const UndoUpdateOp *op
) {
	//--------------------------------------------------------------------------
	// effect format:
	//    effect type
	//    entity ID
	//    attribute id
	//    attribute value
	//--------------------------------------------------------------------------

	const Node *n = &op->n;
	
	// get updated value
	SIValue *v = GraphEntity_GetProperty((GraphEntity*)n, op->attr_id);

	// compute effect byte size
	size_t s = sizeof(EffectType)                +  // effect type
			   sizeof(NodeID)                    +  // node ID
			   sizeof(Attribute_ID)              +  // attribute ID
			   SIValue_BinarySize(v);               // attribute value

	return s;
}

// compute required update-effect size for undo-update operation
static size_t ComputeUpdateSize
(
	const UndoOp *op
) {
	// undo operation
	const UndoUpdateOp *_op = (const UndoUpdateOp*)op;

	// entity-type node/edge
	if(_op->entity_type == GETYPE_NODE) {
		return ComputeNodeUpdateSize(_op);
	} else {
		return ComputeEdgeUpdateSize(_op);
	}
}

// compute required effects buffer byte size from undo-log
size_t ComputeBufferSize
(
	const UndoLog undolog
) {
	ASSERT(undolog != NULL);

	size_t s = 0;  // effects-buffer required size in bytes
	uint n = UndoLog_Length(undolog);  // number of undo entries

	// make sure undo-log is not empty
	ASSERT(n > 0);

	// account for effect version
	s += sizeof(uint8_t);

	// compute effect size from each undo operation
	for(uint i = 0; i < n; i++) {
		const UndoOp *op = undolog + i;
		switch(op->type) {
			case UNDO_DELETE_NODE:
				// DeleteNode effect size
				s += sizeof(EffectType) +
					 fldsiz(UndoDeleteNodeOp, id);
				break;
			case UNDO_DELETE_EDGE:
				// DeleteEdge effect size
				s += sizeof(EffectType)                   +
					 fldsiz(UndoDeleteEdgeOp, id)         +
					 fldsiz(UndoDeleteEdgeOp, relationID) +
					 fldsiz(UndoDeleteEdgeOp, srcNodeID)  +
					 fldsiz(UndoDeleteEdgeOp, destNodeID);
				break;
			case UNDO_UPDATE:
				// Update effect size
				s += ComputeUpdateSize(op);
				break;
			case UNDO_CREATE_NODE:
				s += ComputeCreateNodeSize(op);
				break;
			case UNDO_CREATE_EDGE:
				s += ComputeCreateEdgeSize(op);
				break;
			case UNDO_ADD_ATTRIBUTE:
				s += ComputeAttrAddSize(op);
				break;
			case UNDO_SET_LABELS:
				s += ComputeSetLabelSize(op);
				break;
			case UNDO_REMOVE_LABELS:
				s += ComputeRemoveLabelSize(op);
				break;
			case UNDO_ADD_SCHEMA:
				s += ComputeSchemaAddSize(op);
				break;
			default:
				assert(false && "unknown undo operation");
				break;
		}
	}

	return s;
}

