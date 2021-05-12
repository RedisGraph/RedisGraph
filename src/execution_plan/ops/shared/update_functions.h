/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../execution_plan.h"
#include "../../../execution_plan/ops/shared/undo_log.h"

// context representing a single update to perform on an entity
typedef struct PendingUpdateCtx {
	GraphEntity *ge;       // entity to be updated
	int label_id;          // label ID if the updated entity is a node
	bool update_index;     // whether an index is affected by update
	SIValue new_value;     // constant value to set
	Attribute_ID attr_id;  // ID of attribute to update
} PendingUpdateCtx;

// commit all updates described in the array of pending updates
void CommitUpdates(GraphContext *gc, ResultSetStatistics *stats,
		PendingUpdateCtx *updates, bool is_rollback, UndoLogCtx *undo_log_ctx);

/* build pending updates in the 'updates' array to match all
 * AST-level updates described in the context
 * NULL values are allowed in SET clauses but not in MERGE clauses */
void EvalEntityUpdates(GraphContext *gc, PendingUpdateCtx **updates,
		const Record r, const EntityUpdateEvalCtx *ctx, bool allow_null, UndoLogCtx *undo_log_ctx);

