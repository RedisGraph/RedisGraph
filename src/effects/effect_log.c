/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "effects.h"

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
	EffectLog *log,   // effect log
	const Edge *edge  // edge deleted
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

	Effect op;

	if(entity_type == GETYPE_NODE) {
		op.type = EFFECT_UPDATE_NODE;
		op.update.n = *(Node *)entity;
	} else {
		op.type = EFFECT_UPDATE_EDGE;
		op.update.e = *(Edge *)entity;
	}

	op.update.value       = value;
	op.update.attr_id     = attr_id;
	op.update.entity_type = entity_type;

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

