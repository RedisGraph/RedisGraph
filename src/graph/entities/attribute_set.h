/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

#define ATTRIBUTE_NOTFOUND USHRT_MAX
// ATTRIBUTE_ALL indicates all properties for SET clauses that replace a property map
#define ATTRIBUTE_ALL USHRT_MAX - 1

typedef unsigned short Attribute_ID;

typedef struct {
	Attribute_ID id;
	SIValue value;
} EntityProperty;

// Essence of a graph entity.
// TODO: see if pragma pack 0 will cause memory access violation on ARM.
typedef struct {
	int prop_count;             // Number of properties.
	EntityProperty *properties; // Key value pair of attributes.
} AttributeSet;

// deletes all properties on the GraphEntity and returns
// the number of deleted properties
int AttributeSet_ClearProperties
(
	AttributeSet *attributes
);

// adds property to entity
// returns - reference to newly added property
bool AttributeSet_AddProperty
(
	AttributeSet *attributes,
	Attribute_ID attr_id,
	SIValue value,
	bool allow_null
);

// retrieves entity's property
// NOTE: If the key does not exist, we return the special
// constant value PROPERTY_NOTFOUND
SIValue *AttributeSet_GetProperty
(
	const AttributeSet *attributes,
	Attribute_ID attr_id
);

// updates existing attribute value, return true if property been updated
bool AttributeSet_SetProperty
(
	AttributeSet *e,
	Attribute_ID attr_id,
	SIValue value
);

// clones the given attribute set
AttributeSet *AttributeSet_Clone
(
	const AttributeSet *attributes
);

// release all memory allocated for attribute set properties
void AttributeSet_FreeProperties
(
	AttributeSet *e
);

// release all memory allocated by attribute set
void AttributeSet_Free
(
	AttributeSet *e
);
