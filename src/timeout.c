/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "timeout.h"
#include "util/cron.h"
#include "query_ctx.h"

//------------------------------------------------------------------------------
// Query timeout
//------------------------------------------------------------------------------

// Timeout handler function invoked by cron job.
void QueryTimedOut(void *pdata) {
	ASSERT(pdata);
	// Unpack arguments
	TimeoutCtx *ctx = pdata;
	ExecutionPlan *plan = ctx->plan;

	// drain the ExecutionPlan
	ExecutionPlan_Drain(plan);

	/* Timer may have triggered after execution-plan ran to completion
	 * in which case the original query thread had called ExecutionPlan_Free
	 * decreasing the plan's ref count, but did not free the execution-plan
	 * it is our responsibility to call ExecutionPlan_Free
	 *
	 * In case execution-plan timedout we'll call ExecutionPlan_Free
	 * to drop plan's ref count. */
	ExecutionPlan_Free(plan);
}

// set timeout for query execution
void Timeout_SetTimeout(uint timeout, ExecutionPlan *plan) {
	// increase execution plan ref count
	ExecutionPlan_IncreaseRefCount(plan);
	TimeoutCtx *timeout_ctx = rm_malloc(sizeof(TimeoutCtx));
	timeout_ctx->plan = plan;
	// add timeout context and callback to cron thread
	CronTask task = Cron_AddTask(timeout, QueryTimedOut, timeout_ctx);
	QueryCtx_SetTimeoutJob(task);
}

bool Timeout_ClearTimeout() {
	CronTask task = QueryCtx_GetTimeoutJob();
	if(task == NULL) return true; // this query had no timeout, return true
	bool removed = Cron_RemoveTask(task);
	QueryCtx_SetTimeoutJob(NULL);
	return removed;
}

