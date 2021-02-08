/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GRAPH_ENTITY_H_
#define GRAPH_ENTITY_H_

#include "../../value.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

#define ATTRIBUTE_NOTFOUND USHRT_MAX

#define ENTITY_ID_ISLT(a, b) ((*a) < (*b))
#define INVALID_ENTITY_ID -1l

#define ENTITY_GET_ID(graphEntity) (graphEntity)->id
#define ENTITY_PROP_COUNT(graphEntity) ((graphEntity)->entity->prop_count)
#define ENTITY_PROPS(graphEntity) ((graphEntity)->entity->properties)

// Defined in graph_entity.c
extern SIValue *PROPERTY_NOTFOUND;

typedef unsigned short Attribute_ID;
typedef GrB_Index EntityID;
typedef GrB_Index NodeID;
typedef GrB_Index EdgeID;

/*  Format a graph entity string according to the enum.
    One can sum the enum values in order to print multiple value:
    ENTITY_ID + ENTITY_LABELS_OR_RELATIONS will print both id and label. */

typedef enum {
	ENTITY_ID = 1,                       // print id only
	ENTITY_LABELS_OR_RELATIONS = 1 << 1, // print label or relationship type
	ENTITY_PROPERTIES = 1 << 2           // print properties
} GraphEntityStringFromat;

typedef enum GraphEntityType {
	GETYPE_UNKNOWN,
	GETYPE_NODE,
	GETYPE_EDGE
} GraphEntityType;

typedef struct {
	Attribute_ID id;
	SIValue value;
} EntityProperty;

// Essence of a graph entity.
// TODO: see if pragma pack 0 will cause memory access violation on ARM.
typedef struct {
	int prop_count;             // Number of properties.
	EntityProperty *properties; // Key value pair of attributes.
} Entity;

// Common denominator between nodes and edges.
typedef struct {
	Entity *entity;
	EntityID id;
} GraphEntity;

/* Adds property to entity
 * returns - reference to newly added property. */
bool GraphEntity_AddProperty(GraphEntity *e, Attribute_ID attr_id, SIValue value);

/* Retrieves entity's property
 * NOTE: If the key does not exist, we return the special
 * constant value PROPERTY_NOTFOUND. */
SIValue *GraphEntity_GetProperty(const GraphEntity *e, Attribute_ID attr_id);

/* Updates existing attribute value, return true if property been updated. */
bool GraphEntity_SetProperty(const GraphEntity *e, Attribute_ID attr_id, SIValue value);

/* Prints the graph entity into a buffer, returns what is the string length, buffer can be re-allocated at need. */
void GraphEntity_ToString(const GraphEntity *e, char **buffer, size_t *bufferLen,
						  size_t *bytesWritten,
						  GraphEntityStringFromat format, GraphEntityType entityType);

/* Prints the graph entity to a buffer in JSON format. */
void GraphEntity_ToJSON(GraphEntity *ge, char **buf, size_t *bufferLen, size_t *bytesWritten,
						GraphEntityType type);

// Returns true if the given graph entity has been deleted.
bool GraphEntity_IsDeleted(const GraphEntity *e);

/* Release all memory allocated by entity */
void FreeEntity(Entity *e);

#endif

