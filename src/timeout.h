/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "execution_plan/execution_plan.h"

typedef struct {
	ExecutionPlan *plan;    // the query's ExecutionPlan
} TimeoutCtx;

// instantiate the thread-local TimeoutCtx on module load
bool TimeoutCtx_Init(void);

// set timeout for query execution
void Timeout_SetTimeout(uint timeout, ExecutionPlan *plan);

// clear timeout job, returning true if successful or the job did not exist
bool Timeout_ClearTimeout(void);

