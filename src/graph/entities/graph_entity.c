/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <stdio.h>
#include <assert.h>
#include "graph_entity.h"
#include "../../util/rmalloc.h"

SIValue *PROPERTY_NOTFOUND = &(SIValue){.intval = 0, .type = T_NULL};

/* Add a new property to entity */
SIValue* GraphEntity_Add_Property(GraphEntity *e, Attribute_ID attr_id, SIValue value) {
	if(e->entity->properties == NULL) {
		e->entity->properties = rm_malloc(sizeof(EntityProperty));
	} else {
		e->entity->properties = rm_realloc(e->entity->properties, sizeof(EntityProperty) * (e->entity->prop_count + 1));
	}

	int prop_idx = e->entity->prop_count;
	e->entity->properties[prop_idx].id = attr_id;
	e->entity->properties[prop_idx].value = value;
	e->entity->prop_count++;
	
	return &(e->entity->properties[prop_idx].value);
}

SIValue* GraphEntity_Get_Property(const GraphEntity *e, Attribute_ID attr_id) {
	if(attr_id == ATTRIBUTE_NOTFOUND) return PROPERTY_NOTFOUND;

	for(int i = 0; i < e->entity->prop_count; i++) {
		if(attr_id == e->entity->properties[i].id) {
			// Note, unsafe as entity properties can get reallocated.
			return &(e->entity->properties[i].value);
		}
	}

	return PROPERTY_NOTFOUND;
}

// Updates existing property value.
void GraphEntity_Set_Property(const GraphEntity *e, Attribute_ID attr_id, SIValue value) {
	SIValue *prop = GraphEntity_Get_Property(e, attr_id);
	assert(prop != PROPERTY_NOTFOUND);
	*prop = value;
}

void FreeEntity(Entity *e) {
	assert(e);
	if(e->properties != NULL) {
		for(int i = 0; i < e->prop_count; i++) SIValue_Free(&e->properties[i].value);
		rm_free(e->properties);
		e->properties = NULL;
	}
}
