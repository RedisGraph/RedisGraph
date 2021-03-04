/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "execution_plan/execution_plan.h"

typedef struct {
	ExecutionPlan *plan;    // the query's ExecutionPlan
	bool query_completed;   // whether the query has finished successfully
	bool changes_committed; // whether the graph has been locked for commits
} TimeoutCtx;

// instantiate the thread-local TimeoutCtx on module load
bool TimeoutCtx_Init(void);

// set timeout for query execution
void Timeout_SetTimeout(uint timeout, ExecutionPlan *plan);

// clear timeout job
void Timeout_ClearTimeout(void);

