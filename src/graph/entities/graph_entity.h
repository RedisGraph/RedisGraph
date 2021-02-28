/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "attribute_set.h"
#include "../../value.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

#define ATTRIBUTE_NOTFOUND USHRT_MAX
// ATTRIBUTE_ALL indicates all properties for SET clauses that replace a property map
#define ATTRIBUTE_ALL USHRT_MAX - 1

#define ENTITY_ID_ISLT(a, b) ((*a) < (*b))
#define INVALID_ENTITY_ID -1l

#define ENTITY_GET_ID(graphEntity) (graphEntity)->id
#define ENTITY_PROPS(graphEntity) ((graphEntity)->entity->properties)

typedef GrB_Index NodeID;
typedef GrB_Index EdgeID;
typedef GrB_Index EntityID;
typedef unsigned short Attribute_ID;

/* Format a graph entity string according to the enum.
 * One can sum the enum values in order to print multiple value:
 * ENTITY_ID + ENTITY_LABELS_OR_RELATIONS will print both id and label. */

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

// Essence of a graph entity.
typedef struct {
	AttributeSet *sets;
} Entity;

// Common denominator between nodes and edges.
typedef struct {
	Entity *entity;
	EntityID id;
} GraphEntity;

/* Deletes all properties on the GraphEntity and returns
 * the number of deleted properties. */
int GraphEntity_ClearProperties(
	GraphEntity *e
);

AttributeSet GraphEntity_GetAttributeSet
(
	const GraphEntity *e
);

// return number of properties associated with entity
uint GraphEntity_PropCount
(
	const GraphEntity *e  // entity to get attribute count from
);

/* Adds property to entity
 * returns - reference to newly added property. */
void GraphEntity_AddProperty
(
	GraphEntity *e,
	Attribute_ID attr_id,
	SIValue value
);

/* Retrieves entity's property
 * NOTE: If the key does not exist, we return the special
 * constant value PROPERTY_NOTFOUND. */
SIValue *GraphEntity_GetProperty
(
	const GraphEntity *e,
	Attribute_ID attr_id
);

// updates existing attribute value, return true if property been updated
bool GraphEntity_SetProperty
(
	const GraphEntity *e,
	Attribute_ID attr_id,
	SIValue value
);

// prints the graph entity into a buffer, returns what is the string length,
// buffer can be re-allocated at need
void GraphEntity_ToString
(
	const GraphEntity *e,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten,
	GraphEntityStringFromat format,
	GraphEntityType entityType
);

// Returns true if the given graph entity has been deleted.
bool GraphEntity_IsDeleted
(
	const GraphEntity *e
);

// release all memory allocated by entity
void Entity_Free
(
	Entity *e
);

