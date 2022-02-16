/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"

// indicates a none existing attribute ID
#define ATTRIBUTE_ID_NONE USHRT_MAX
// indicates all attributes for SET clauses that replace a property map
#define ATTRIBUTE_ID_ALL USHRT_MAX - 1

typedef unsigned short Attribute_ID;

typedef struct {
	Attribute_ID id;  // attribute identifier
	SIValue value;    // attribute value
} Attribute;

// TODO: see if pragma pack 0 will cause memory access violation on ARM.
typedef struct {
	int attr_count;         // number of attributes
	Attribute *attributes;  // key value pair of attributes
} AttributeSet;

// clears attribute set
// returns number of attributes cleared
void AttributeSet_Clear
(
	AttributeSet *set  // set to be cleared
);

// adds an attribute to the set
// returns true if attribute was added to the set
// TODO: see if we can change from `bool` to `void`
bool AttributeSet_Add
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value,         // attribute value
	bool allow_null        // is NULL consider valid value
);

// retrieves a value from set
// NOTE: if the key does not exist
//       we return the special constant value ATTRIBUTE_NOTFOUND
SIValue *AttributeSet_Get
(
	const AttributeSet *set,  // set to retieve attribute from
	Attribute_ID attr_id      // attribute identifier
);

// updates existing attribute, return true if attribute been updated
bool AttributeSet_Update
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // new value
);

// clones attribute set
AttributeSet *AttributeSet_Clone
(
	const AttributeSet *set  // set to clone
);

// free attribute set
void AttributeSet_Free
(
	AttributeSet *set  // set to be freed
);

