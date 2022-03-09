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

// returns number of attribute within the set
#define ATTRIBUTE_SET_COUNT(attributes) ((attributes)->attr_count)

typedef unsigned short Attribute_ID;

typedef struct {
	Attribute_ID id;  // attribute identifier
	SIValue value;    // attribute value
} Attribute;

typedef struct {
	int attr_count;         // number of attributes
	Attribute attributes[];  // key value pair of attributes
} _AttributeSet;

typedef _AttributeSet* AttributeSet;

// retrieves a value from set
// NOTE: if the key does not exist
//       we return the special constant value ATTRIBUTE_NOTFOUND
SIValue *AttributeSet_Get
(
	AttributeSet set,     // set to retieve attribute from
	Attribute_ID attr_id  // attribute identifier
);

// retrieves a value from set by index
SIValue AttributeSet_GetIdx
(
	AttributeSet set,      // set to retieve attribute from
	int i,                 // index of the property
	Attribute_ID *attr_id  // attribute identifier
);

// adds an attribute to the set
// returns true if attribute was added otherwise false
void AttributeSet_Add
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // attribute value
);

// updates existing attribute, return true if attribute been updated
// otherwise false
bool AttributeSet_Update
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // new value
);

// clones attribute set
AttributeSet AttributeSet_Clone
(
	const AttributeSet set  // set to clone
);

// clears attribute set
void AttributeSet_Clear
(
	AttributeSet *set  // set to be cleared
);

// free attribute set
void AttributeSet_Free
(
	AttributeSet *set  // set to be freed
);

