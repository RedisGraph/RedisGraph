/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "attribute_set.h"
#include "../../value.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

#define ENTITY_ID_ISLT(a, b) ((*a) < (*b))
#define INVALID_ENTITY_ID -1l

#define ENTITY_GET_ID(graphEntity) (graphEntity)->id
#define ENTITY_PROP_COUNT(graphEntity) ((graphEntity)->attributes->attr_count)
#define ENTITY_PROPS(graphEntity) ((graphEntity)->attributes->attributes)

// Defined in graph_entity.c
extern SIValue *ATTRIBUTE_NOTFOUND;

typedef GrB_Index EdgeID;
typedef GrB_Index NodeID;
typedef GrB_Index EntityID;

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

// Common denominator between nodes and edges.
typedef struct {
	AttributeSet *attributes;
	EntityID id;
} GraphEntity;

// returns an SIArray of all keys in graph entity properties
SIValue GraphEntity_Keys
(
	const GraphEntity *e
);

// prints the graph entity into a buffer, returns what is the string length, buffer can be re-allocated at need
void GraphEntity_ToString
(
	const GraphEntity *e,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten,
	GraphEntityStringFromat format,
	GraphEntityType entityType
);

// returns true if the given graph entity has been deleted.
bool GraphEntity_IsDeleted
(
	const GraphEntity *e
);
