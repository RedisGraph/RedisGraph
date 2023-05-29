/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../../value.h"

// indicates a none existing attribute ID
#define ATTRIBUTE_ID_NONE USHRT_MAX

// indicates all attributes for SET clauses that replace a property map
#define ATTRIBUTE_ID_ALL USHRT_MAX - 1

// returns number of attribute within the set
#define ATTRIBUTE_SET_COUNT(attributes) \
	(attributes == NULL ? 0 : ((attributes)->attr_count))

typedef unsigned short Attribute_ID;

// type of change performed on the attribute-set
typedef enum {
	CT_NONE,    // no change
	CT_ADD,     // attribute been added
	CT_UPDATE,  // attribute been updated
	CT_DEL      // attribute been deleted
} AttributeSetChangeType;

typedef struct {
	Attribute_ID id;  // attribute identifier
	SIValue value;    // attribute value
} Attribute;

typedef struct {
	uint16_t attr_count;     // number of attributes
	Attribute attributes[];  // key value pair of attributes
} _AttributeSet;

typedef _AttributeSet* AttributeSet;

// variable representing the empty attribute-set
static AttributeSet NULL_ATTRIBUTE_SET = NULL;

// retrieves a value from set
// NOTE: if the key does not exist
//       we return the special constant value ATTRIBUTE_NOTFOUND
SIValue *AttributeSet_Get
(
	const AttributeSet set,  // set to retieve attribute from
	Attribute_ID attr_id     // attribute identifier
);

// retrieves a value from set by index
SIValue AttributeSet_GetIdx
(
	const AttributeSet set,  // set to retieve attribute from
	uint16_t i,              // index of the property
	Attribute_ID *attr_id    // attribute identifier
);

// adds an attribute to the set without cloning the SIValue
void AttributeSet_AddNoClone
(
	AttributeSet *set,  // set to update
	Attribute_ID *ids,  // identifiers
	SIValue *values,    // values
	ushort n,           // number of values to add
	bool allowNull		// accept NULLs
);

// adds an attribute to the set (clones the value)
void AttributeSet_Add
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // attribute value
);

// add, remove or update an attribute
// this function allows NULL value to be added to the set
// returns the type of change performed
AttributeSetChangeType AttributeSet_Set_Allow_Null
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // attribute value
);

// updates existing attribute (without cloning)
// return true if attribute been updated
bool AttributeSet_UpdateNoClone
(
	AttributeSet *set,     // set to update
	Attribute_ID attr_id,  // attribute identifier
	SIValue value          // new value
);

// updates existing attribute
// return true if attribute been updated
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

// clones attribute set without si values
AttributeSet AttributeSet_ShallowClone
(
	const AttributeSet set  // set to clone
);

// persists all attributes within given set
void AttributeSet_PersistValues
(
	const AttributeSet set  // set to persist
);

// free attribute set
void AttributeSet_Free
(
	AttributeSet *set  // set to be freed
);

