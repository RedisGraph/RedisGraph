/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "shared/update_functions.h"
#include "../../resultset/resultset_statistics.h"

typedef struct {
	OpBase op;
	GraphContext *gc;
	ResultSetStatistics *stats;

	rax *update_ctxs;               // Entities to update and their expressions.
	PendingUpdateCtx *updates;      // Enqueued updates
	Record *records;                // Updated records, used only when query hands off records after updates.
	bool updates_commited;          // True if we've already committed updates and are now in handoff mode.
} OpUpdate;

OpBase *NewUpdateOp(const ExecutionPlan *plan, rax *update_exps);

