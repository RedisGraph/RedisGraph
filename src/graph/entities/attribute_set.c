/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "attribute_set.h"
#include "../../util/rmalloc.h"

// an attribute is a pair of attribute-identifier and a value
typedef struct {
	SIValue value;         // attribute value
	Attribute_ID id;       // attribute id
} Attribute;

struct AttributeSet_opaque {
	Version v;               // set version
	uint attribute_count;    // number of attributes in set
	Attribute attributes[];  // attributes in set
};

// compute number of bytes required to represent an attribute-set with
// 'attr_count' attributes
static size_t _AttributeSet_ComputeSize
(
	uint attr_count  // number of attributes in set
) {
	return sizeof(struct AttributeSet_opaque) + attr_count * sizeof(Attribute);
}

AttributeSet AttributeSet_New
(
	Version v  // version associated with set
) {
	AttributeSet set = rm_malloc(sizeof(*set));

	// initilize set
	set->v                =  v;
	set->attribute_count  =  0;

	return set;
}

AttributeSet AttributeSet_Clone
(
	AttributeSet set,  // set to clone
	Version v          // version of new attribute set
) {
	ASSERT(set != NULL);
	// clone version must be greater then original set version
	ASSERT(v > AttributeSet_GetVersion(set));

	// determine size of clone
	uint attr_count = AttributeSet_AttributeCount(set);
	size_t size = _AttributeSet_ComputeSize(attr_count);

	AttributeSet clone = rm_malloc(size);
	clone->v = v;
	
	// clone attributes
	for(int i = 0; i < attr_count; i++) {
		clone->attributes[i].id = set->attributes[i].id;
	   	clone->attributes[i].value = SI_CloneValue(set->attributes[i].value);
	}

	return clone;
}

inline uint AttributeSet_AttributeCount
(
	const AttributeSet set
) {
	ASSERT(set != NULL);
	return set->attribute_count;
}

inline Version AttributeSet_GetVersion
(
	const AttributeSet set
) {
	ASSERT(set != NULL);
	return set->v;
}

bool AttributeSet_Contains
(
	const AttributeSet set,  // set to search id in
	Attribute_ID id,         // attribute id to locate
	uint *idx                // [optional] attribute position within set
) {
	ASSERT(set != NULL);
	
	uint attr_count = AttributeSet_AttributeCount(set);
	
	// search for attribute with specified id in set
	for(uint i = 0; i < attr_count; i++) {
		if(set->attributes[i].id == id) {
			if(idx != NULL) *idx = i;
			return true;
		}
	}

	return false;
}

SIValue *AttributeSet_GetAttr
(
	const AttributeSet set,  // set to get attribute from
	Attribute_ID id          // ID of attribute to retrieve
) {
	ASSERT(set != NULL);

	uint idx;
	if(!AttributeSet_Contains(set, id, &idx)) return ATTRIBUTE_NOTFOUND;
	return &(set->attributes[idx].value);
}

// return attribute at position 'idx'
// setting both 'value' and 'id' if provided 
void AttributeSet_GetAttrIdx
(
	const AttributeSet set,  // set to retrieve attribute from
	uint idx,                // attribute position in set
	SIValue *value,          // [optional] attribute value
	Attribute_ID *id         // [optional] attribute ID
) {
	ASSERT(set != NULL);

	SIValue _v = SI_NullVal();
	Attribute_ID _id = ATTRIBUTE_NOTFOUND;

	if(idx < AttributeSet_AttributeCount(set)) {
		Attribute attr = set->attributes[idx];
		_id = attr.id;
		_v = attr.value;
	}

	if(id) *id = _id;
	if(value) *value = _v;
}

AttributeSet AttributeSet_SetAttr
(
	AttributeSet *set,  // attribute set update
	Attribute_ID id,    // ID of attribute being set
	SIValue v           // value of attribute
) {
	ASSERT(set != NULL);

	uint idx;
	AttributeSet s = *set;

	if(AttributeSet_Contains(s, id, &idx)) {
		// attribute in set, overwrite it
		SIValue_Free(s->attributes[idx].value);
		s->attributes[idx].value = SI_CloneValue(v); 
	} else {
		// attribute missing from set, add it
		// make room for new attribute
		uint attr_count = AttributeSet_AttributeCount(s) + 1;
		size_t size = _AttributeSet_ComputeSize(attr_count);
		*set = rm_realloc(s, size);
		s = *set;
		s->attribute_count = attr_count;
		s->attributes[attr_count-1].id = id;
		s->attributes[attr_count-1].value = SI_CloneValue(v);
	}

	return s;
}

AttributeSet AttributeSet_RemoveAttr
(
	AttributeSet *set,  // set to remove attribute from
	Attribute_ID id,    // attribute to remove
	bool *removed       // [optinal] set to true if attribute was removed
) {
	ASSERT(set != NULL);
	ASSERT(id != ATTRIBUTE_NOTFOUND);

	uint idx;
	AttributeSet s = *set;
	bool b_removed = false;

	// locate attribute
	if(AttributeSet_Contains(s, id, &idx)) {
		// replace attribute at position 'idx' with attribute at position
		// attr_count-1 (last attribute)
		SIValue_Free(s->attributes[idx].value);
		uint attr_count = AttributeSet_AttributeCount(s);

		// overwrite removed attribute with last attribute in set
		s->attributes[idx] = s->attributes[attr_count-1];

		// reallocate attribute set
		attr_count -= 1;
		size_t size = _AttributeSet_ComputeSize(attr_count);
		*set = rm_realloc(s, size);

		// update set attribute count
		s->attribute_count = attr_count;
		b_removed = true;
	}

	if(removed) *removed = b_removed;
	return s;
}

void AttributeSet_Free
(
	AttributeSet set  // set to free
) {
	ASSERT(set != NULL);

	// free each attribute in set
	uint attr_count = AttributeSet_AttributeCount(set);
	for(int i = 0; i < attr_count; i++) {
		SIValue attr = set->attributes[i].value;
		SIValue_Free(attr);
	}

	rm_free(set);
}

