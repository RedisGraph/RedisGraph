/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_profile.h"
#include "cmd_context.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "execution_ctx.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../execution_plan/execution_plan.h"

void Graph_Profile(void *args) {
	bool lockAcquired = false;
	ResultSet *result_set = NULL;
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc = CommandCtx_GetGraphContext(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);

	QueryCtx_BeginTimer(); // Start query timing.

	/* Retrive the required execution items and information:
	* 1. AST
	* 2. Execution plan (if any)
	* 3. Whether these items were cached or not */
	AST *ast = NULL;
	ExecutionPlan *plan = NULL;
	bool cached = false;
	ExecutionCtx exec_ctx = ExecutionCtx_FromQuery(command_ctx->query);

	ast = exec_ctx.ast;
	plan = exec_ctx.plan;
	ExecutionType exec_type = exec_ctx.exec_type;
	// See if there were any query compile time errors
	if(QueryCtx_EncounteredError()) {
		QueryCtx_EmitException();
		goto cleanup;
	}
	if(exec_type == EXECUTION_TYPE_INVALID) goto cleanup;
	if(exec_type == EXECUTION_TYPE_INDEX_CREATE ||
	   exec_type == EXECUTION_TYPE_INDEX_DROP) {
		RedisModule_ReplyWithError(ctx, "Can't profile index operations.");
		goto cleanup;
	}

	bool readonly = AST_ReadOnly(ast->root);

	// Acquire the appropriate lock.
	if(readonly) {
		Graph_AcquireReadLock(gc->g);
	} else {
		Graph_WriterEnter(gc->g);  // Single writer.
		/* If this is a writer query `we need to re-open the graph key with write flag
		* this notifies Redis that the key is "dirty" any watcher on that key will
		* be notified. */
		CommandCtx_ThreadSafeContextLock(command_ctx);
		{
			GraphContext_MarkWriter(ctx, gc);
		}
		CommandCtx_ThreadSafeContextUnlock(command_ctx);
	}
	lockAcquired = true;

	result_set = NewResultSet(ctx, FORMATTER_NOP);
	// Indicate a cached execution.
	if(cached) ResultSet_CachedExecution(result_set);
	QueryCtx_SetResultSet(result_set);

	ExecutionPlan_PreparePlan(plan);
	ExecutionPlan_Profile(plan);
	QueryCtx_ForceUnlockCommit();
	ExecutionPlan_Print(plan, ctx);

cleanup:
	// Release the read-write lock
	if(lockAcquired) {
		if(readonly) Graph_ReleaseLock(gc->g);
		else Graph_WriterLeave(gc->g);
	}

	ResultSet_Free(result_set);
	AST_Free(ast);
	ExecutionPlan_Free(plan);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
}
