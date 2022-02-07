/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "attribute_set.h"
#include "../../RG.h"
#include "../../errors.h"
#include "../../util/rmalloc.h"

SIValue *PROPERTY_NOTFOUND = &(SIValue) {
	.longval = 0, .type = T_NULL
};

// removes entity's property
static bool _AttributeSet_RemoveProperty
(
	AttributeSet *e,
	Attribute_ID attr_id
) {
	// Quick return if attribute is missing.
	if(attr_id == ATTRIBUTE_NOTFOUND) return false;

	// Locate attribute position.
	int prop_count = e->prop_count;
	for(int i = 0; i < prop_count; i++) {
		if(attr_id == e->properties[i].id) {
			SIValue_Free(e->properties[i].value);
			e->prop_count--;

			if(e->prop_count == 0) {
				/* Only attribute removed, free properties bag. */
				rm_free(e->properties);
				e->properties = NULL;
			} else {
				/* Overwrite deleted attribute with the last
				 * attribute and shrink properties bag. */
				e->properties[i] = e->properties[prop_count - 1];
				e->properties = rm_realloc(e->properties,
					sizeof(EntityProperty) * e->prop_count);
			}

			return true;
		}
	}

	return false;
}

int AttributeSet_ClearProperties
(
	AttributeSet *e
) {
	ASSERT(e);

	int prop_count = e->prop_count;
	for(int i = 0; i < prop_count; i++) {
		// free all allocated properties
		SIValue_Free(e->properties[i].value);
	}
	e->prop_count = 0;

	// free and NULL-set the properties bag.
	rm_free(e->properties);
	e->properties = NULL;

	return prop_count;
}

/* Add a new property to entity */
bool AttributeSet_AddProperty
(
	AttributeSet *e,
	Attribute_ID attr_id,
	SIValue value,
	bool allow_null
) {
	ASSERT(e);
	if(!(SI_TYPE(value) & SI_VALID_PROPERTY_VALUE) && SIValue_IsNull(value) && !allow_null) return false;

	if(e->properties == NULL) {
		e->properties = rm_malloc(sizeof(EntityProperty));
	} else {
		e->properties = rm_realloc(e->properties,
			sizeof(EntityProperty) * (e->prop_count + 1));
	}

	int prop_idx = e->prop_count;
	EntityProperty *prop = e->properties + prop_idx;
	prop->id = attr_id;
	prop->value = SI_CloneValue(value);
	e->prop_count++;

	return true;
}

SIValue *AttributeSet_GetProperty
(
	const AttributeSet *e,
	Attribute_ID attr_id
) {
	if(attr_id == ATTRIBUTE_NOTFOUND) return PROPERTY_NOTFOUND;
	if(e == NULL) {
 		/* The internal entity pointer should only be NULL if the entity
 		 * is in an intermediate state, such as a node scheduled for creation.
 		 * Note that this exception may cause memory to be leaked in the caller. */
 		ErrorCtx_SetError("Attempted to access undefined property");
 		return PROPERTY_NOTFOUND;
 	}

	for(int i = 0; i < e->prop_count; i++) {
		if(attr_id == e->properties[i].id) {
			// Note, unsafe as entity properties can get reallocated.
			return &(e->properties[i].value);
		}
	}

	return PROPERTY_NOTFOUND;
}

// updates existing property value
bool AttributeSet_SetProperty
(
	AttributeSet *e,
	Attribute_ID attr_id,
	SIValue value
) {
	ASSERT(e);

	// Setting an attribute value to NULL removes that attribute.
	if(SIValue_IsNull(value)) return _AttributeSet_RemoveProperty(e, attr_id);

	SIValue *current = AttributeSet_GetProperty(e, attr_id);
	ASSERT(current != PROPERTY_NOTFOUND);

	// compare current value to new value, only update if current != new
	if(SIValue_Compare(*current, value, NULL) == 0) return false;

	// value != current, update entity
	SIValue_Free(*current);
	*current = SI_CloneValue(value);
	return true;
}
// clones the given attribute set
AttributeSet *AttributeSet_Clone
(
	const AttributeSet *attributes
) {
    AttributeSet *clone = rm_malloc(sizeof(AttributeSet));
	clone->prop_count   = attributes->prop_count;
	clone->properties   = rm_malloc(sizeof(EntityProperty) * attributes->prop_count);
	
    for (uint i = 0; i < attributes->prop_count; i++) {
		EntityProperty *prop       = attributes->properties + i;
		EntityProperty *clone_prop = clone->properties + i;
		clone_prop->id             = prop->id;
		clone_prop->value          = SI_CloneValue(prop->value);
	}

    return clone;
}

void AttributeSet_FreeProperties
(
	AttributeSet *e
) {
	ASSERT(e);
	if(e->properties != NULL) {
		for(int i = 0; i < e->prop_count; i++) SIValue_Free(e->properties[i].value);
		rm_free(e->properties);
		e->properties = NULL;
		e->prop_count = 0;
	}
}

void AttributeSet_Free
(
	AttributeSet * e
) {
	ASSERT(e);

	AttributeSet_FreeProperties(e);
	rm_free(e);
}
