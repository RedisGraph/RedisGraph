/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../redismodule.h"
#include "../graph/entities/graph_entity.h"
#include "../graph/entities/attribute_set.h"

// forward declaration of opaque constraint structure
typedef _Constraint *Constraint;

// different states a constraint can be at
// starting as pending and transitioning to either active or failed
typedef enum ConstraintStatus {
    CT_ACTIVE = 0,
    CT_PENDING,
    CT_FAILED
} ConstraintStatus;

// type of constraint
// we're currently supporting two types of constraints
// 1. unique constraint
// 2. mandatory constraint
typedef enum {
	CT_UNIQUE,
	CT_MANDATORY
} ConstraintType;

// create a new constraint
Constraint Constraint_New
(
	Attribute_ID *fields, // enforced fields
	uint n_fields,        // number of fields
	const Schema *s,      // constraint schema
	ConstraintType t      // constraint type
);

// returns constraint status
ConstraintStatus Constraint_GetStatus
(
	const Constraint c
);

// set constraint status
// status can change from:
// 1. CT_PENDING to CT_ACTIVE
// 2. CT_PENDING to CT_FAILED
void Constraint_SetStatus
(
	Constraint c,            // constraint to update
	ConstraintStatus status  // new status
);

// returns a shallow copy of constraint attributes
const Attribute_ID *Constraint_GetAttributes
(
	const Constraint c  // constraint from which to get attributes
);

// returns number of pending changes
int Constraint_PendingChanges
(
	const Constraint c  // constraint to inquery
);

// increment number of pending changes
void Constraint_IncPendingChanges
(
	Constraint c  // constraint to update
);

// decrement number of pending changes
void Constraint_DecPendingChanges
(
	Constraint c  // constraint to update
);

// enforce constraint on entity
// returns true if entity satisfies the constraint
// false otherwise
bool Constraint_EnforceEntity
(
	const Constraint *c,  // constraint to enforce
	const GraphEntity *e  // enforced entity
);

void Constraint_Drop_Index
(
	Constraint c,
	GraphContext *gc,
	bool should_drop_constraint
);

// is the field have constraint which enforce it
bool Has_Constraint_On_Attribute(Constraint *constraints, Attribute_ID attr_id);

int Graph_Constraint(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

// returns constraint graph entity type
GraphEntityType Constraint_GraphEntityType
(
	const Constraint c
);

void Constraint_free(Constraint c);

