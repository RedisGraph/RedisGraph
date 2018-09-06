/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <stdio.h>
#include "graph_entity.h"
#include "../util/qsort.h"

#define nodeIDislt(a,b) (a<b)

#define edgeIDislt(a,b) ( ( a->src < b->src ) ||\
    ( ( a->src == b->src ) && ( a->dest < b->dest ) ) ||\
    ( ( a->src == b->src ) && ( a->dest == b->dest ) && ( a->relation_type < b->relation_type ) ) )

SIValue *PROPERTY_NOTFOUND = &(SIValue){.intval = 0, .type = T_NULL};

/* Expecting e to be either *Node or *Edge */
void GraphEntity_Add_Properties(GraphEntity *e, int prop_count, char **keys, SIValue *values) {
	if(e->properties == NULL) {
		e->properties = malloc(sizeof(EntityProperty) * prop_count);
	} else {
		e->properties = realloc(e->properties, sizeof(EntityProperty) * (e->prop_count + prop_count));
	}
	
	for(int i = 0; i < prop_count; i++) {
		e->properties[e->prop_count + i].name = strdup(keys[i]);
		e->properties[e->prop_count + i].value = SI_Clone(values[i]);
	}

	e->prop_count += prop_count;
}

SIValue* GraphEntity_Get_Property(const GraphEntity *e, const char* key) {
	for(int i = 0; i < e->prop_count; i++) {
		if(strcmp(key, e->properties[i].name) == 0) {
			return &e->properties[i].value;
		}
	}
	return PROPERTY_NOTFOUND;
}

int SortAndUniqueEntities(EntityID *entities, size_t entityCount) {
    // Sort.
    QSORT(EntityID, entities, entityCount, ENTITY_ID_ISLT);

    int j = 0;  // Index to next unique entity
    for(int i = 0; i < entityCount - 1; i++) {
        if (entities[i] != entities[i + 1]) {
            entities[j++] = entities[i];
        }
    }
    // Copy the final element
    entities[j++] = entities[entityCount - 1];
    return j;
}

void FreeGraphEntity(GraphEntity *e) {
	if(e->properties != NULL) {
		for(int i = 0; i < e->prop_count; i++) {
			free(e->properties[i].name);
		}		
		free(e->properties);
	}
}
