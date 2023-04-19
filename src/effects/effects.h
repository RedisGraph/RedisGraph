/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../graph/graphcontext.h"

#define EFFECTS_VERSION 1  // current effects encoding/decoding version

// size of the field in the structure
#define	fldsiz(name, field) \
	(sizeof(((struct name *)0)->field))

// types of effects
typedef enum {
	EFFECT_UNKNOWN = 0,    // unknown effect
	EFFECT_UPDATE_NODE,    // node update
	EFFECT_UPDATE_EDGE,    // edge update
	EFFECT_CREATE_NODE,    // node creation
	EFFECT_CREATE_EDGE,    // edge creation
	EFFECT_DELETE_NODE,    // node deletion
	EFFECT_DELETE_EDGE,    // edge deletion
	EFFECT_SET_LABELS,     // set labels
	EFFECT_REMOVE_LABELS,  // remove labels
	EFFECT_ADD_SCHEMA,     // schema addition
	EFFECT_ADD_ATTRIBUTE,  // add attribute
} EffectType;

// effect node/edge creation
typedef struct EffectCreate EffectCreate;
struct EffectCreate {
	union {
		Node n;
		Edge e;
	};
};

// effect node deletion
typedef struct EffectDeleteNode EffectDeleteNode;
struct EffectDeleteNode {
	EntityID id;       // ID of deleted node
};

// effect edge deletion
typedef struct EffectDeleteEdge EffectDeleteEdge;
struct EffectDeleteEdge {
	EntityID id;        // ID of deleted edge
	int relationID;     // Relation ID
	NodeID srcNodeID;   // Source node ID
	NodeID destNodeID;  // Destination node ID
};

// effect graph entity update
typedef struct EffectUpdate EffectUpdate;
struct EffectUpdate {
	const GraphEntity *entity;    // entity to update
	GraphEntityType entity_type;  // node/edge
	Attribute_ID attr_id;         // attribute update
	SIValue value;                // attribute value
};

// effect added/removed labels to/from node
typedef struct EffectLabels EffectLabels;
struct EffectLabels {
	EntityID id;          // node ID
	LabelID* label_ids;   // set of labels IDs
	ushort labels_count;  // number of labels
};

// effect added schema
typedef struct EffectAddSchema EffectAddSchema;
struct EffectAddSchema {
	const char *schema_name; // name of schema
	SchemaType t;            // schema type
};

// effect added attribute
typedef struct EffectAddAttribute EffectAddAttribute;
struct EffectAddAttribute {
	const char *attr_name;  // name of added attribute
};

// unified effect representation
typedef struct {
	union {
		EffectCreate create;
		EffectDeleteNode delete_node;
		EffectDeleteEdge delete_edge;
		EffectUpdate update;
		EffectLabels labels;
		EffectAddSchema schema;
		EffectAddAttribute attribute;
	};
	EffectType type;  // effect type
} Effect;

// container for effect_list
typedef Effect *EffectLog;

//------------------------------------------------------------------------------
// effects
//------------------------------------------------------------------------------

// compute required effects buffer byte size from effect-log
size_t ComputeBufferSize
(
	const EffectLog effect_log
);

// create a list of effects from the effect-log
u_char *Effects_FromEffectLog
(
	EffectLog log,
	size_t *l
);

// applys effects encoded in buffer
void Effects_Apply
(
	GraphContext *gc,          // graph to operate on
	const char *effects_buff,  // encoded effects
	size_t l                   // size of buffer
);

// create a new effect-log
EffectLog EffectLog_New(void);

// returns number of entries in log
uint EffectLog_Length
(
	const EffectLog log  // log to query
);

//------------------------------------------------------------------------------
// EffectLog add operations
//------------------------------------------------------------------------------

// node creation effect
void EffectLog_CreateNode
(
	EffectLog *log,  // effect log
	const Node *node // node created
);

// edge creation effect
void EffectLog_CreateEdge
(
	EffectLog *log,  // effect log
	const Edge *edge // edge created
);

// node deletion effect
void EffectLog_DeleteNode
(
	EffectLog *log,  // effect log
	const Node *node // node deleted
);

// edge deletion effect
void EffectLog_DeleteEdge
(
	EffectLog *log,  // effect log
	const Edge *edge // edge deleted
);

// entity update effect
void EffectLog_UpdateEntity
(
	EffectLog *log,              // effect log
	const GraphEntity *entity,   // updated entity ID
	Attribute_ID attr_id,        // updated attribute ID
 	SIValue value,               // value
	GraphEntityType entity_type  // entity type
);

// node add label effect
void EffectLog_AddLabels
(
	EffectLog *log,              // effect log
	const Node *node,            // updated node
	const LabelID *label_ids,    // added labels
	size_t labels_count          // number of removed labels
);

// node remove label effect
void EffectLog_RemoveLabels
(
	EffectLog *log,              // effect log
	const Node *node,            // updated node
	const LabelID *label_ids,    // removed labels
	size_t labels_count          // number of removed labels
);

// schema addition effect
void EffectLog_AddSchema
(
	EffectLog *log,               // effect log
	const char *schema_name,      // id of the schema
	SchemaType t                  // type of the schema
);

// attribute addition effect
void EffectLog_AddAttribute
(
	EffectLog *log,              // effect log
	const char *attr             // attribute name
);

// free effect
void EffectLog_FreeEffect
(
	Effect *effect  // effect to free
);

// free EffectLog
void EffectLog_Free
(
	EffectLog log
);

