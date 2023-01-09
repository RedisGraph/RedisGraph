/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

typedef struct {
    Attribute_ID id;        // Attribute ID.
    char *attribute_name;   // attribute name
} ConstAttrData;

typedef struct _Constraint {
    ConstAttrData *attributes;     // array of attributes sorted by their ids which are part of this constraint
    char *label;                   // indexed label
	int label_id;                  // indexed label ID
    GraphEntityType entity_type;   // entity type (node/edge) indexed
} _Constraint;

// forward declaration
typedef _Constraint *Constraint;

// the ids array should be sorted
Constraint Constraint_new(Attribute_ID *ids, uint id_count, const char *label, int label_id, GraphEntityType type);

// Enforce the constraint on the given entity.
bool Constraint_enforce_entity(Constraint c, const AttributeSet attributes, RSIndex *idx);

// Enforce the constraints on the given entity.
bool Constraints_enforce_entity(Constraint c, const AttributeSet attributes, RSIndex *idx);


void Free_Constraint_Remove_Its_Index(Constraint c, const GraphContext *gc);

// is the field have constraint which enforce it
bool Has_Constraint_On_Attribute(const Constraint constraints, Attribute_ID attr_id);
