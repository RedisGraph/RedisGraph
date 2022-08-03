/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../execution_plan.h"

// context representing a single update to perform on an entity
typedef struct {
	GraphEntity *ge;       // entity to be updated
	AttributeSet attributes;
	rax *labels;  // TODO: rax seems like an overkill, maybe the removal of duplicates need to be done at the AST level ?
} PendingUpdateCtx;

// commit all updates described in the array of pending updates
void CommitUpdates
(
	GraphContext *gc,
	ResultSetStatistics *stats,
	PendingUpdateCtx *updates,
	EntityType type
);

/* build pending updates in the 'updates' array to match all
 * AST-level updates described in the context
 * NULL values are allowed in SET clauses but not in MERGE clauses */
void EvalEntityUpdates
(
	GraphContext *gc,
	PendingUpdateCtx **node_updates,
	PendingUpdateCtx **edge_updates,
	const Record r,
	const EntityUpdateEvalCtx *ctx,
	bool allow_null
);

