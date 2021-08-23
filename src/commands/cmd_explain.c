/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
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
	bool              cached     =  false;
	RT_ExecutionPlan  *plan      =  NULL;
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

	ExecutionPlan_Print(plan->plan_desc, ctx); // Print the execution plan.

cleanup:
	if(ErrorCtx_EncounteredError()) ErrorCtx_EmitException();
	ExecutionCtx_Free(exec_ctx);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
	ErrorCtx_Clear();
}
