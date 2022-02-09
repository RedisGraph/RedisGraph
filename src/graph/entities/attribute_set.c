/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "attribute_set.h"
#include "../../RG.h"
#include "../../errors.h"
#include "../../util/rmalloc.h"

// returned value for a missing attribute
SIValue *ATTRIBUTE_NOTFOUND = &(SIValue) {
	.longval = 0, .type = T_NULL
};

// removes an attribute from set
static bool _AttributeSet_Remove
(
	AttributeSet *set,
	Attribute_ID attr_id
) {
	// quick return if attribute is missing
	if(attr_id == ATTRIBUTE_ID_NONE) return false;

	// locate attribute position
	int attr_count = set->attr_count;
	for(int i = 0; i < attr_count; i++) {
		if(attr_id == set->attributes[i].id) {
			SIValue_Free(set->attributes[i].value);
			set->attr_count--;

			if(set->attr_count == 0) {
				// only attribute removed, free attribute bag
				rm_free(set->attributes);
				set->attributes = NULL;
			} else {
				// overwrite deleted attribute with the last
				// attribute and shrink attributes bag
				set->attributes[i] = set->attributes[prop_count - 1];
				set->attributes = rm_realloc(set->attributes,
					sizeof(Attribute) * set->attr_count);
			}

			// attribute was removed
			return true;
		}
	}

	// unable to locate attribute
	return false;
}

// clears attribute set
// returns number of attributes cleared
int AttributeSet_Clear
(
	AttributeSet *set  // set to be cleared
) {
	ASSERT(set != NULL);

	int attr_count = set->attr_count;
	for(int i = 0; i < attr_count; i++) {
		// free all allocated properties
		SIValue_Free(set->attributes[i].value);
	}
	e->attr_count = 0;

	// free and NULL-set the attribute bag
	rm_free(set->attributes);
	set->attributes = NULL;

	return attr_count;
}

// adds an attribute to the set
// returns true if attribute was added to the set
// TODO: see if we can change from `bool` to `void`
bool AttributeSet_Add
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value,         // attribute value
	bool allow_null        // is NULL consider valid value
) {
	ASSERT(set != NULL);

	// validate value type
	if(!(SI_TYPE(value) & SI_VALID_PROPERTY_VALUE) &&
			SIValue_IsNull(value)                  &&
			!allow_null) {
		return false;
	}

	// allocate room for attribute
	set->attr_count++;
	if(set->attributes == NULL) {
		set->attributes = rm_malloc(sizeof(Attribute));
	} else {
		set->attributes = rm_realloc(set->attributes,
			sizeof(Attribute) * set->attr_count);
	}

	Attribute *attr = set->attributes + set->attr_count;
	attr->id = attr_id;
	attr->value = SI_CloneValue(value);

	return true;
}

// retrieves a value from set
// NOTE: if the key does not exist
//       we return the special constant value ATTRIBUTE_NOTFOUND
SIValue *AttributeSet_Get
(
	const AttributeSet *set,  // set to retieve attribute from
	Attribute_ID attr_id      // attribute identifier
) {
	if(attr_id == ATTRIBUTE_ID_NONE) return ATTRIBUTE_NOTFOUND;
	if(set == NULL) {
 		// note that this exception may cause memory to be leaked in the caller
 		ErrorCtx_SetError("Attempted to access undefined attribute-set");
 		return ATTRIBUTE_NOTFOUND;
 	}

	for(int i = 0; i < set->attr_count; i++) {
		Attribute *attr = set->attributes + i;
		if(attr_id == set->attr->id) {
			// note, unsafe as attribute-set  can get reallocated
			return &attr->value;
		}
	}

	return ATTRIBUTE_NOTFOUND;
}

// updates existing attribute, return true if attribute been updated
bool AttributeSet_Update
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // new value
) {
	ASSERT(set != NULL);

	// setting an attribute value to NULL removes that attribute
	if(SIValue_IsNull(value)) return _AttributeSet_Remove(set, attr_id);

	SIValue *current = AttributeSet_Get(set, attr_id);
	ASSERT(current != ATTRIBUTE_NOTFOUND);

	// compare current value to new value, only update if current != new
	// TODO: only index entities that performed a successful update on an indexed attribute
	if(SIValue_Compare(*current, value, NULL) == 0) return false;

	// value != current, update entity
	SIValue_Free(*current);
	*current = SI_CloneValue(value);
	return true;
}

// clones attribute set
AttributeSet *AttributeSet_Clone
(
	const AttributeSet *set  // set to clone
) {
	ASSERT(set != NULL);

    AttributeSet *clone = rm_malloc(sizeof(AttributeSet));
	clone->attr_count   = set->attr_count;
	clone->attributes   = rm_malloc(sizeof(Attribute) * set->attr_count);
	
	for (uint i = 0; i < set->attr_count; i++) {
		Attribute *attr        = set->attributes   + i;
		Attribute *clone_attr  = clone->attributes + i;

		clone_attr->id = attr->id;
		clone_attr->value = SI_CloneValue(attr->value);
	}

    return clone;
}

// clears all attributes from set
void AttributeSet_FreeAttributes
(
	AttributeSet *set  // set to be cleared
) {
	ASSERT(set != NULL);

	// quick return if there are no attributes
	if(set->attributes == NULL) return;

	// free each attribute
	for(int i = 0; i < set->attr_count; i++) {
		Attribute *attr = set->attributes + i;
		SIValue_Free(attr->value);
	}
	rm_free(set->attributes);

	set->attr_count = 0;
	set->attributes = NULL;
}

// free attribute set
void AttributeSet_Free
(
	AttributeSet *set  // set to be freed
) {
	ASSERT(set != NULL);

	AttributeSet_FreeAttributes(set);
	rm_free(set);
}

