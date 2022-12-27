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

typedef enum ConstraintStatus {
    CT_ACTIVE = 0,
    CT_PENDING,
    CT_FAILED
} ConstraintStatus;

typedef enum {
	CT_UNIQUE,
	CT_MANDATORY
} ConstraintType;

typedef struct _Constraint {
    ConstAttrData *attributes;     // array of attributes sorted by their ids which are part of this constraint
    char *label;                   // indexed label
	int label_id;                  // indexed label ID
    GraphEntityType entity_type;   // entity type (node/edge) indexed
    ConstraintStatus status;       // constraint status
} _Constraint;

// forward declaration
typedef _Constraint *Constraint;

// returns constraint attributes
const ConstAttrData *Constraint_GetAttributes(const Constraint c);

// the ids array should be sorted
Constraint Constraint_new(Attribute_ID *ids, uint id_count, const char *label, int label_id, GraphEntityType type);

// Enforce the constraint on the given entity.
bool Constraint_enforce_entity(Constraint c, const AttributeSet attributes, RSIndex *idx);

// Enforce the constraints on the given entity.
bool Constraints_enforce_entity(Constraint c, const AttributeSet attributes, RSIndex *idx);


void Constraint_Drop_Index(Constraint c, const GraphContext *gc);

// is the field have constraint which enforce it
bool Has_Constraint_On_Attribute(const Constraint constraints, Attribute_ID attr_id);

int Graph_Constraint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
