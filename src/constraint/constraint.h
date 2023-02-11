/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../redismodule.h"
#include "../graph/graph.h"
#include "../graph/entities/graph_entity.h"
#include "../graph/entities/attribute_set.h"

// forward declaration of opaque constraint structure
typedef struct _Constraint *Constraint;

// Schema forward declaration
struct Schema;

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
	Attribute_ID *fields,    // enforced fields
	uint n_fields,           // number of fields
	const struct Schema *s,  // constraint schema
	ConstraintType t         // constraint type
);

// returns constraint's type
ConstraintType Constraint_GetType
(
	const Constraint c  // constraint to query
);

// tries to enforce constraint on all relevant entities
// sets constraint status to pending
void Constraint_Enforce
(
	const Constraint c,  // constraint to enforce
	Graph *g
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
	const Constraint c,  // constraint from which to get attributes
	uint *n              // length of returned array
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
	const Constraint c,   // constraint to enforce
	const GraphEntity *e  // enforced entity
);

// checks if constraint enforces attribute
bool Constraint_EnforceAttribute
(
	Constraint c,         // constraint to query
	Attribute_ID attr_id  // enforced attribute
);

// free constraint
void Constraint_Free
(
	Constraint c  // constraint to free
);

