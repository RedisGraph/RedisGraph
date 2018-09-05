/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef GRAPH_ENTITY_H_
#define GRAPH_ENTITY_H_

#include "../value.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

#define ENTITY_ID_ISLT(a,b) ((*a)<(*b))
#define INVALID_ENTITY_ID -1l

// Defined in graph_entity.c
extern SIValue *PROPERTY_NOTFOUND;
typedef GrB_Index EntityID;
typedef GrB_Index NodeID;
typedef GrB_Index EdgeID;

typedef struct {
    char *name;
    SIValue value;
} EntityProperty;

typedef struct {
    NodeID id;                    /* unique id (might be empty) */
    int prop_count;
    EntityProperty *properties;
} GraphEntity;

/* Adds a properties to entity
 * prop_count - number of new properties to add 
 * keys - array of properties keys 
 * values - array of properties values */
void GraphEntity_Add_Properties(GraphEntity *e, int prop_count, char **keys, SIValue *values);

/* Retrieves entity's property
 * NOTE: If the key does not exist, we return the special
 * constant value PROPERTY_NOTFOUND. */
SIValue* GraphEntity_Get_Property(const GraphEntity *e, const char* key);

/* Release all memory allocated by entity */
void FreeGraphEntity(GraphEntity *e);

#endif
