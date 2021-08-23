/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/shared/update_functions.h"
#include "../../../../resultset/resultset_statistics.h"

typedef struct {
	RT_OpBase op;
	raxIterator it;                 // Iterator for traversing update contexts
	Record *records;                // Updated records
	GraphContext *gc;
	rax *update_ctxs;               // Entities to update and their expressions
	bool updates_committed;         // True if we've already committed updates and are now in handoff mode.
	PendingUpdateCtx *updates;      // Enqueued updates
	ResultSetStatistics *stats;
} RT_OpUpdate;

RT_OpBase *RT_NewUpdateOp(const RT_ExecutionPlan *plan, rax *update_exps);
