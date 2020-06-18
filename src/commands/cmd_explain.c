/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_explain.h"
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
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc = CommandCtx_GetGraphContext(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);
	QueryCtx_BeginTimer(); // Start query timing.

	/* Retrieve the required execution items and information:
	 * 1. AST
	 * 2. Execution plan (if any)
	 * 3. Whether these items were cached or not */
	AST *ast = NULL;
	bool cached = false;
	ExecutionPlan *plan = NULL;
	ExecutionCtx exec_ctx = ExecutionCtx_FromQuery(command_ctx->query);
	
	ExecutionType exec_type = exec_ctx.exec_type;
	ast = exec_ctx.ast;
	plan = exec_ctx.plan;
	// See if there were any query compile time errors
	if(QueryCtx_EncounteredError()) {
		QueryCtx_EmitException();
		goto cleanup;
	}
	if(exec_type == EXECUTION_TYPE_INVALID) goto cleanup;

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
	ExecutionPlan_Print(plan, ctx); // Print the execution plan.

cleanup:
	if(lock_acquired) Graph_ReleaseLock(gc->g);

	AST_Free(ast);
	ExecutionPlan_Free(plan);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
}
