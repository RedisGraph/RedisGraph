/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../../execution_plan.h"
#include "../../../util/dict.h"

// context representing a single update to perform on an entity
typedef struct {
	GraphEntity *ge;            // entity to be updated
	AttributeSet attributes;    // attributes to update
} PendingUpdateCtx;

// commit all updates described in the array of pending updates
void CommitUpdates
(
	GraphContext *gc,          // graph context
	dict *updates,             // array of pending updates
	GrB_Matrix add_labels,     // matrix of labels to add
	GrB_Matrix remove_labels,  // matrix of labels to remove
	EntityType type            // type of entity to update
);

// build pending updates in the 'updates' array to match all
// AST-level updates described in the context
// NULL values are allowed in SET clauses but not in MERGE clauses
void EvalEntityUpdates
(
	GraphContext *gc,                 // graph context
	dict *node_updates,               // array of pending updates for nodes
	dict *edge_updates,               // array of pending updates for edges
	GrB_Matrix *add_labels,           // matrix of labels to add
	GrB_Matrix *remove_labels,        // matrix of labels to remove
	const Record r,                   // current record
	const EntityUpdateEvalCtx *ctx,   // update context
	bool allow_null                   // allow NULL values in SET clauses
);

void PendingUpdateCtx_Free
(
	PendingUpdateCtx *ctx
);

