/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "GraphBLAS/Include/GraphBLAS.h"
#include "value.h"

#define ATTRIBUTE_NOTFOUND USHRT_MAX

#define ENTITY_ID_ISLT(a,b) ((*a)<(*b))
#define INVALID_ENTITY_ID ((EntityID) -1l)

#define ENTITY_GET_ID(graphEntity) ((graphEntity)->entity ? (graphEntity)->entity->id : INVALID_ENTITY_ID)
#define ENTITY_PROP_COUNT(graphEntity) ((graphEntity)->entity->prop_count)
#define ENTITY_PROPS(graphEntity) ((graphEntity)->entity->properties)

// Defined in graph_entity.c
extern SIValue *PROPERTY_NOTFOUND;

typedef unsigned short Attribute_ID;
typedef GrB_Index EntityID;
typedef GrB_Index NodeID;
typedef GrB_Index EdgeID;

typedef enum GraphEntityType {
<<<<<<< HEAD
    GraphEntityType_NODE,
    GraphEntityType_EDGE
=======
   GETYPE_NODE,
   GETYPE_EDGE
>>>>>>> origin/master
} GraphEntityType;

typedef struct {
    Attribute_ID id;
    SIValue value;
} EntityProperty;

// Essence of a graph entity
// TODO: see if pragma pack 0 will cause memory access violation on ARM
typedef struct {
    EntityID id;                    // Unique id
    int prop_count;                 // Number of properties.
    EntityProperty *properties;     // Key value pair of attributes.
} Entity;

// Common denominator between nodes and edges
typedef struct {
    Entity *entity;
} GraphEntity;

// Adds property to entity
// returns - reference to newly added property
SIValue* GraphEntity_AddProperty(GraphEntity *e, Attribute_ID attr_id, SIValue value);

// Retrieves entity's property
// NOTE: If the key does not exist, we return the special
// constant value PROPERTY_NOTFOUND
SIValue* GraphEntity_GetProperty(const GraphEntity *e, Attribute_ID attr_id);

// Updates existing attribute value
void GraphEntity_SetProperty(const GraphEntity *e, Attribute_ID attr_id, SIValue value);

// Release all memory allocated by entity
void FreeEntity(Entity *e);

void GraphEntity_Print(const GraphEntity *e, GraphEntityType t, FILE *out);
