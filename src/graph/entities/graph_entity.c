/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdio.h>
#include <assert.h>
#include "graph_entity.h"
#include "../../util/rmalloc.h"

SIValue *PROPERTY_NOTFOUND = &(SIValue) {
	.longval = 0, .type = T_NULL
};

/* Removes entity's property. */
static void _GraphEntity_RemoveProperty(const GraphEntity *e, Attribute_ID attr_id) {
	// Quick return if attribute is missing.
	if(GraphEntity_GetProperty(e, attr_id) == PROPERTY_NOTFOUND) return;

	// Locate attribute position.
	int prop_count = e->entity->prop_count;
	for(int i = 0; i < prop_count; i++) {
		if(attr_id == e->entity->properties[i].id) {
			SIValue_Free(&(e->entity->properties[i].value));
			e->entity->prop_count--;

			if(e->entity->prop_count == 0) {
				/* Only attribute removed, free properties bag. */
				rm_free(e->entity->properties);
				e->entity->properties = NULL;
			} else {
				/* Overwrite deleted attribute with the last
				 * attribute and shrink properties bag. */
				e->entity->properties[i] = e->entity->properties[prop_count - 1];
				e->entity->properties = rm_realloc(e->entity->properties,
												   sizeof(EntityProperty) * e->entity->prop_count);
			}

			break;
		}
	}
}

/* Add a new property to entity */
SIValue *GraphEntity_AddProperty(GraphEntity *e, Attribute_ID attr_id, SIValue value) {
	if(e->entity->properties == NULL) {
		e->entity->properties = rm_malloc(sizeof(EntityProperty));
	} else {
		e->entity->properties = rm_realloc(e->entity->properties,
										   sizeof(EntityProperty) * (e->entity->prop_count + 1));
	}

	int prop_idx = e->entity->prop_count;
	e->entity->properties[prop_idx].id = attr_id;
	e->entity->properties[prop_idx].value = SI_CloneValue(value);
	e->entity->prop_count++;

	return &(e->entity->properties[prop_idx].value);
}

SIValue *GraphEntity_GetProperty(const GraphEntity *e, Attribute_ID attr_id) {
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
void GraphEntity_SetProperty(const GraphEntity *e, Attribute_ID attr_id, SIValue value) {
	assert(e);

	// Setting an attribute value to NULL removes that attribute.
	if(SIValue_IsNull(value)) {
		return _GraphEntity_RemoveProperty(e, attr_id);
	}

	SIValue *prop = GraphEntity_GetProperty(e, attr_id);
	assert(prop != PROPERTY_NOTFOUND);
	SIValue_Free(prop);
	*prop = SI_CloneValue(value);
}

void FreeEntity(Entity *e) {
	assert(e);
	if(e->properties != NULL) {
		for(int i = 0; i < e->prop_count; i++) SIValue_Free(&e->properties[i].value);
		rm_free(e->properties);
		e->properties = NULL;
	}
}
