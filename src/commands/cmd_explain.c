/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "../errors.h"
#include "cmd_context.h"
#include "../query_ctx.h"
#include "execution_ctx.h"
#include "../index/index.h"
#include "../util/rmalloc.h"
#include "../execution_plan/execution_plan.h"

/* Builds an execution plan but does not execute it
 * reports plan back to the client
 * Args:
 * argv[1] graph name
 * argv[2] query */
void Graph_Explain(void *args) {
	bool lock_acquired = false;
	CommandCtx     *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx         = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext   *gc          = CommandCtx_GetGraphContext(command_ctx);
	ExecutionCtx   *exec_ctx    = NULL;

	QueryCtx_SetGlobalExecutionCtx(command_ctx);
	CommandCtx_TrackCtx(command_ctx);

	if(strcmp(command_ctx->query, "") == 0) {
		ErrorCtx_SetError("Error: empty query.");
		goto cleanup;
	}

	QueryCtx_BeginTimer(); // Start query timing.

	/* Retrieve the required execution items and information:
	 * 1. Execution plan
	 * 2. Whether these items were cached or not */
	bool           cached     =  false;
	ExecutionPlan  *plan      =  NULL;
	exec_ctx  =  ExecutionCtx_FromQuery(command_ctx->query);
	if(exec_ctx == NULL) goto cleanup;

	plan = exec_ctx->plan;
	ExecutionType exec_type = exec_ctx->exec_type;

	if(exec_type == EXECUTION_TYPE_INDEX_CREATE) {
		RedisModule_ReplyWithSimpleString(ctx, "Create Index");
		goto cleanup;
	} else if(exec_type == EXECUTION_TYPE_INDEX_DROP) {
		RedisModule_ReplyWithSimpleString(ctx, "Drop Index");
		goto cleanup;
	}

	Graph_AcquireReadLock(gc->g);
	lock_acquired = true;

	ExecutionPlan_PreparePlan(plan);
	ExecutionPlan_Init(plan);       // Initialize the plan's ops.

	if(ErrorCtx_EncounteredError()) goto cleanup;

	ExecutionPlan_Print(plan, ctx); // Print the execution plan.

cleanup:
	if(ErrorCtx_EncounteredError()) ErrorCtx_EmitException();
	if(lock_acquired) Graph_ReleaseLock(gc->g);
	ExecutionCtx_Free(exec_ctx);
	GraphContext_DecreaseRefCount(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
	ErrorCtx_Clear();
}

