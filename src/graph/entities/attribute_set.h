/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <limits.h>
#include "../../value.h"

extern SIValue *ATTRIBUTE_NOTFOUND; // defined in attribute_set.c

#define ATTRIBUTE_UNKNOWN USHRT_MAX
typedef uint Version;
typedef unsigned short Attribute_ID;
typedef struct AttributeSet_opaque *AttributeSet;

// create a new attribute set, with given version 'v'
AttributeSet AttributeSet_New
(
	Version v  // version associated with set
);

// clones given attribute set, sets clone's version to 'v'
// note: clone version must be > than original set
AttributeSet AttributeSet_Clone
(
	AttributeSet set,  // set to clone
	Version v          // version of new attribute set
);

// returns number of attributes in 'set'
uint AttributeSet_AttributeCount
(
	const AttributeSet set
);

// returns version of given 'set'
Version AttributeSet_GetVersion
(
	const AttributeSet set
);

// returns true if 'id' is in 'set', if 'idx' isn't NULL sets 'idx'
// to attribute position
bool AttributeSet_Contains
(
	const AttributeSet set,  // set to search id in
	Attribute_ID id,         // attribute id to locate
	uint *idx                // [optional] attribute position within set
);

// retrieve attribute from set, returns a pointer to value if 'id' is in 'set'
// otherwise ATTRIBUTE_NOTFOUND is returned
SIValue *AttributeSet_GetAttr
(
	const AttributeSet set,  // set to get attribute from
	Attribute_ID id          // ID of attribute to retrieve
);

// return attribute at position 'idx'
// setting both 'value' and 'id' if provided
void AttributeSet_GetAttrIdx
(
	const AttributeSet set,  // set to retrieve attribute from
	uint idx,                // attribute position in set
	SIValue *value,          // [optional] attribute value
	Attribute_ID *id         // [optional] attribute ID
);

// sets attribute with given 'id' to specified value 'v'
// reallocates 'set' if attribute is new, otherwise overwrite existing one
AttributeSet AttributeSet_SetAttr
(
	AttributeSet *set,  // attribute set update
	Attribute_ID id,    // ID of attribute being set
	SIValue v           // value of attribute
);

AttributeSet AttributeSet_RemoveAttr
(
	AttributeSet *set,  // set to remove attribute from
	Attribute_ID id,    // attribute to remove
	bool *removed       // [optional] set to true if attribute was removed
);

// remove all attributes in the given set
AttributeSet AttributeSet_Clear
(
	AttributeSet *set,  // set to remove all attributes from
	uint *removed       // [optional] number of attributes removed
);

// free attribute set, each stored attribute is freed
void AttributeSet_Free
(
	AttributeSet set  // set to free
);

