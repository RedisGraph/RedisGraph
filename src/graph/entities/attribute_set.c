/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <limits.h>

#include "RG.h"
#include "attribute_set.h"
#include "../../util/rmalloc.h"

// compute size of attribute set in bytes
#define ATTRIBUTESET_BYTE_SIZE(set) ((set) == NULL ? sizeof(_AttributeSet) : sizeof(_AttributeSet) + sizeof(Attribute) * (set)->attr_count)

// determine if set is empty
#define ATTRIBUTESET_EMPTY(set) (set) == NULL

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
	AttributeSet _set = *set;
	int attr_count = _set->attr_count;

	// locate attribute position
	for(int i = 0; i < attr_count; i++) {
		if(attr_id != _set->attributes[i].id) {
			continue;
		}

		// if this is the last attribute free the attribute-set
		if(_set->attr_count == 1) {
			AttributeSet_Free(set);
			return true;
		}

		// attribute located
		// free attribute value
		SIValue_Free(_set->attributes[i].value);

		// overwrite deleted attribute with the last
		// attribute and shrink set
		_set->attributes[i] = _set->attributes[attr_count - 1];

		// update attribute count
		_set->attr_count--;

		// compute new set size
		size_t n = ATTRIBUTESET_BYTE_SIZE(_set);
		*set = rm_realloc(_set, n);

		// attribute removed
		return true;
	}

	// unable to locate attribute
	return false;
}

// retrieves a value from set
// NOTE: if the key does not exist
//       we return the special constant value ATTRIBUTE_NOTFOUND
SIValue *AttributeSet_Get
(
	const AttributeSet set,  // set to retieve attribute from
	Attribute_ID attr_id     // attribute identifier
) {
	if(set == NULL) return ATTRIBUTE_NOTFOUND;

	if(attr_id == ATTRIBUTE_ID_NONE) return ATTRIBUTE_NOTFOUND;

	// TODO: benchmark, consider alternatives:
	// sorted set
	// array divided in two:
	// [attr_id_0, attr_id_1, attr_id_2, value_0, value_1, value_2]
	for(int i = 0; i < set->attr_count; i++) {
		Attribute *attr = set->attributes + i;
		if(attr_id == attr->id) {
			// note, unsafe as attribute-set can get reallocated
			// TODO: why do we return a pointer to value instead of a copy ?
			// especially when AttributeSet_GetIdx returns SIValue
			// note AttributeSet_Update operate on this pointer
			return &attr->value;
		}
	}

	return ATTRIBUTE_NOTFOUND;
}

// retrieves a value from set by index
SIValue AttributeSet_GetIdx
(
	const AttributeSet set,  // set to retieve attribute from
	int i,                   // index of the property
	Attribute_ID *attr_id    // attribute identifier
) {
	ASSERT(set != NULL);
	ASSERT(i < set->attr_count);
	ASSERT(attr_id != NULL);

	Attribute *attr = set->attributes + i;
	*attr_id = attr->id;

	return attr->value;
}

static AttributeSet AttributeSet_AddPrepare
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id   // attribute identifier
) {
	ASSERT(set != NULL);
	ASSERT(attr_id != ATTRIBUTE_ID_NONE);

	AttributeSet _set = *set;

	// make sure attribute isn't already in set
	ASSERT(AttributeSet_Get(_set, attr_id) == ATTRIBUTE_NOTFOUND);

	// allocate room for new attribute
	if(_set == NULL) {
		_set = rm_malloc(sizeof(_AttributeSet) + sizeof(Attribute));
		_set->attr_count = 1;
	} else {
		_set->attr_count++;
		size_t n = ATTRIBUTESET_BYTE_SIZE(_set);
		_set = rm_realloc(_set, n);
	}
	return _set;
}

// adds an attribute to the set without cloning the SIvalue
void AttributeSet_AddNoClone
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // attribute value
) {
	// validate value type
	// value must be a valid property type
	ASSERT(SI_TYPE(value) & SI_VALID_PROPERTY_VALUE);
	AttributeSet _set = AttributeSet_AddPrepare(set, attr_id);

	// set attribute
	Attribute *attr = _set->attributes + _set->attr_count - 1;
	attr->id = attr_id;
	attr->value = value;

	// update pointer
	*set = _set;
}

// adds an attribute to the set
void AttributeSet_Add
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // attribute value
) {
	// validate value type
	// value must be a valid property type
	ASSERT(SI_TYPE(value) & SI_VALID_PROPERTY_VALUE);
	AttributeSet _set = AttributeSet_AddPrepare(set, attr_id);

	// set attribute
	Attribute *attr = _set->attributes + _set->attr_count - 1;
	attr->id = attr_id;
	attr->value = SI_CloneValue(value);

	// update pointer
	*set = _set;
}

// adds or updates an attribute to the set null value allowed
void AttributeSet_Set_Allow_Null
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // attribute value
) {
	ASSERT(set != NULL);
	ASSERT(attr_id != ATTRIBUTE_ID_NONE);

	AttributeSet _set = *set;

	// validate value type
	// value must be a valid property type
	ASSERT(SI_TYPE(value) & (SI_VALID_PROPERTY_VALUE | T_NULL));

	// update the attribute if it is already presented in the set
	if(AttributeSet_Get(_set, attr_id) != ATTRIBUTE_NOTFOUND) {
		AttributeSet_Update(&_set, attr_id, value);
		// update pointer
		*set = _set;
		return;
	}

	// allocate room for new attribute
	_set = AttributeSet_AddPrepare(set, attr_id);

	// set attribute
	Attribute *attr = _set->attributes + _set->attr_count - 1;
	attr->id = attr_id;
	attr->value = SI_CloneValue(value);

	// update pointer
	*set = _set;
}

// updates existing attribute, return true if attribute been updated
bool AttributeSet_UpdateNoClone
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // new value
) {
	ASSERT(set != NULL);
	ASSERT(attr_id != ATTRIBUTE_ID_NONE);

	// setting an attribute value to NULL removes that attribute
	if(unlikely(SIValue_IsNull(value))) {
		return _AttributeSet_Remove(set, attr_id);
	}

	SIValue *current = AttributeSet_Get(*set, attr_id);
	ASSERT(current != ATTRIBUTE_NOTFOUND);
	ASSERT(SIValue_Compare(*current, value, NULL) != 0);

	// value != current, update entity
	SIValue_Free(*current);  // free previous value
	*current = value;

	return true;
}

// updates existing attribute, return true if attribute been updated
bool AttributeSet_Update
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // new value
) {
	ASSERT(set != NULL);
	ASSERT(attr_id != ATTRIBUTE_ID_NONE);

	// setting an attribute value to NULL removes that attribute
	if(unlikely(SIValue_IsNull(value))) {
		return _AttributeSet_Remove(set, attr_id);
	}

	SIValue *current = AttributeSet_Get(*set, attr_id);
	ASSERT(current != ATTRIBUTE_NOTFOUND);

	// compare current value to new value, only update if current != new
	if(unlikely(SIValue_Compare(*current, value, NULL) == 0)) {
		return false;
	}

	// value != current, update entity
	SIValue_Free(*current);  // free previous value
	*current = SI_CloneValue(value);

	return true;
}

// clones attribute set
AttributeSet AttributeSet_Clone
(
	const AttributeSet set  // set to clone
) {
	if(set == NULL) return NULL;

	size_t n = ATTRIBUTESET_BYTE_SIZE(set);
	AttributeSet clone  = rm_malloc(n);
	clone->attr_count   = set->attr_count;
	
	for (ushort i = 0; i < set->attr_count; i++) {
		Attribute *attr        = set->attributes   + i;
		Attribute *clone_attr  = clone->attributes + i;

		clone_attr->id = attr->id;
		clone_attr->value = SI_CloneValue(attr->value);
	}

    return clone;
}

// free attribute set
void AttributeSet_Free
(
	AttributeSet *set  // set to be freed
) {
	ASSERT(set != NULL);

	AttributeSet _set = *set;

	if(_set == NULL) return;

	// free all allocated properties
	for(int i = 0; i < _set->attr_count; i++) {
		SIValue_Free(_set->attributes[i].value);
	}

	rm_free(_set);
	*set = NULL;
}
