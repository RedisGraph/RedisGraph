/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../IR/execution_plan/execution_plan.h"
#include "shared/update_functions.h"
#include "../../runtime/resultset/resultset_statistics.h"

typedef struct {
	OpBase op;
	raxIterator it;                 // Iterator for traversing update contexts
	Record *records;                // Updated records
	GraphContext *gc;
	rax *update_ctxs;               // Entities to update and their expressions
	bool updates_committed;         // True if we've already committed updates and are now in handoff mode.
	PendingUpdateCtx *node_updates; // Enqueued node updates
	PendingUpdateCtx *edge_updates; // Enqueued edge updates
	ResultSetStatistics *stats;
} OpUpdate;

OpBase *NewUpdateOp(const ExecutionPlan *plan, rax *update_exps);

