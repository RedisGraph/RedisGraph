/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../graph/entities/attribute_set.h"
#include "../graph/entities/graph_entity.h"
#include "../redismodule.h"
#include "redisearch_api.h"
#include "../util/arr.h"

typedef struct {
    Attribute_ID id;        // Attribute ID.
    char *attribute_name;   // attribute name
} AttrInfo;

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
    AttrInfo *attributes;     // array of attributes sorted by their ids which are part of this constraint
    char *label;                   // indexed label
	int label_id;                  // indexed label ID
    GraphEntityType entity_type;   // entity type (node/edge) indexed
    ConstraintStatus status;       // constraint status
    uint _Atomic pending_changes;  // number of pending changes
} _Constraint;

// forward declaration
typedef _Constraint *Constraint;

// returns constraint attributes
const AttrInfo *Constraint_GetAttributes(const Constraint c);

// the ids array should be sorted
Constraint Constraint_new(AttrInfo *attrData, uint id_count, const char *label, int label_id, GraphEntityType type);

// Set constraint status to active.
void Constraint_Activate(Constraint c);

int Constraint_PendingChanges
(
	const Constraint c  // constraint to inquery
);

// increment number of pending changes
void Constraint_IncPendingChanges
(
	Constraint c
);

// decrement number of pending changes
void Constraint_DecPendingChanges
(
	Constraint c
);

void Constraint_free(Constraint c);

// Enforce the constraint on the given entity.
bool Constraint_enforce_entity(Constraint c, const AttributeSet attributes, RSIndex *idx);

// Enforce the constraints on the given entity.
bool Constraints_enforce_entity(Constraint *c, const AttributeSet attributes, RSIndex *idx, uint32_t *ind);

struct GraphContext; // forward declaration
void Constraint_Drop_Index(Constraint c, struct GraphContext *gc, bool should_drop_constraint);

// is the field have constraint which enforce it
bool Has_Constraint_On_Attribute(Constraint *constraints, Attribute_ID attr_id);

int Graph_Constraint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

// returns constraint graph entity type
GraphEntityType Constraint_GraphEntityType
(
	const Constraint c
);
