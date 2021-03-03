/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "timeout.h"
#include "util/cron.h"

pthread_key_t _tlsTimeoutCtx; // Timeout context held in thread-local storage.

//------------------------------------------------------------------------------
// Timeout context initialization
//------------------------------------------------------------------------------

bool TimeoutCtx_Init(void) {
	int res = pthread_key_create(&_tlsTimeoutCtx, NULL);
	ASSERT(res == 0);

	return (res == 0);
}

//------------------------------------------------------------------------------
// Query timeout
//------------------------------------------------------------------------------

// Timeout handler function invoked by cron job.
void QueryTimedOut(void *pdata) {
	ASSERT(pdata);
	// Unpack arguments
	TimeoutCtx *ctx = pdata;
	ExecutionPlan *plan = ctx->plan;

	/* Drain the ExecutionPlan if the graph has not already
	 * been modified and the query has not finished. */
	if(!ctx->changes_committed && !ctx->query_completed) ExecutionPlan_Drain(plan);

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
void Timeout_SetTimeOut(uint timeout, ExecutionPlan *plan) {
	// increase execution plan ref count
	ExecutionPlan_IncreaseRefCount(plan);
	TimeoutCtx *timeout_ctx = rm_calloc(1, sizeof(TimeoutCtx));
	timeout_ctx->plan = plan;
	// add timeout context to TLS
	pthread_setspecific(_tlsTimeoutCtx, timeout_ctx);
	// add timeout context and callback to cron thread
	Cron_AddTask(timeout, QueryTimedOut, timeout_ctx);
}

void Timeout_QueryCompleted() {
	TimeoutCtx *ctx = pthread_getspecific(_tlsTimeoutCtx);
	ASSERT(ctx);

	ctx->query_completed = true;
	// Null-set the context key in TLS
	pthread_setspecific(_tlsTimeoutCtx, NULL);
}

void Timeout_ChangesCommitted() {
	TimeoutCtx *ctx = pthread_getspecific(_tlsTimeoutCtx);
	if(ctx != NULL) ctx->changes_committed = true;
}

