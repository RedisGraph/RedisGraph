/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "execution_plan/execution_plan.h"

typedef struct {
	ExecutionPlan *plan;    // The query's ExecutionPlan
	bool query_completed;   // Whether the query has finished successfully.
	bool changes_committed; // Whether the graph has been locked for commits.
} TimeoutCtx;

// Instantiate the thread-local TimeoutCtx on module load.
bool TimeoutCtx_Init(void);

// Set timeout for query execution.
void Timeout_SetTimeOut(uint timeout, ExecutionPlan *plan);

// Mark the query as having completed.
void Timeout_QueryCompleted(void);

// Mark the query as having committed changes.
void Timeout_ChangesCommitted(void);
