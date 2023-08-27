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

typedef struct {
	dict *node_updates;        // Enqueued node updates
	dict *edge_updates;        // Enqueued edge updates
	GrB_Matrix add_labels;     // matrix of labels to add
	GrB_Matrix remove_labels;  // matrix of labels to remove
	Schema **reserved_labels;   // labels to be reserved
} GraphUpdateCtx;

// commit all updates described in the array of pending updates
void CommitUpdates
(
	GraphContext *gc,           // graph context
	GraphUpdateCtx *update_ctx  // update context
);

// build pending updates in the 'updates' array to match all
// AST-level updates described in the context
// NULL values are allowed in SET clauses but not in MERGE clauses
void EvalEntityUpdates
(
	GraphContext *gc,                 // graph context
	GraphUpdateCtx *update_ctx,       // update context
	const Record r,                   // current record
	const EntityUpdateEvalCtx *ctx,   // update context
	bool allow_null                   // allow NULL values in SET clauses
);

void PendingUpdateCtx_Free
(
	PendingUpdateCtx *ctx
);

void GraphUpdateCtx_Init
(
	GraphUpdateCtx *ctx
);

Schema *GraphUpdateCtx_ReserveLabel
(
	GraphUpdateCtx *ctx,
	GraphContext *gc,
	const char *label
);

bool GraphUpdateCtx_HasUpdates
(
	GraphUpdateCtx *ctx
);

void GraphUpdateCtx_Reset
(
	GraphUpdateCtx *ctx
);

void GraphUpdateCtx_Free
(
	GraphUpdateCtx *ctx
);
