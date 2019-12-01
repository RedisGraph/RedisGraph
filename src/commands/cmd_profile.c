/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_profile.h"
#include "cmd_context.h"
#include "../query_ctx.h"
#include "../graph/graph.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../execution_plan/execution_plan.h"

void Graph_Profile(void *args) {
	AST *ast = NULL;
	bool lockAcquired = false;
	ResultSet *result_set = NULL;
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc = CommandCtx_GetGraphContext(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);

	QueryCtx_BeginTimer(); // Start query timing.

	// Parse the query to construct an AST
	cypher_parse_result_t *parse_result = parse(command_ctx->query);
	if(parse_result == NULL) goto cleanup;

	bool readonly = AST_ReadOnly(parse_result);
	// If we are a replica and the query is read-only, no work needs to be done.
	if(readonly && command_ctx->replicated_command) goto cleanup;

	// Perform query validations
	if(AST_Validate(ctx, parse_result) != AST_VALID) goto cleanup;

	// Prepare the constructed AST for accesses from the module
	ast = AST_Build(parse_result);

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

	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX ||
	   root_type == CYPHER_AST_DROP_NODE_PROPS_INDEX) {
		RedisModule_ReplyWithError(ctx, "Can't profile index operations.");
		goto cleanup;
	} else if(root_type != CYPHER_AST_QUERY) {
		assert("Unhandled query type" && false);
	}

	result_set = NewResultSet(ctx, false);
	ExecutionPlan *plan = NewExecutionPlan(ctx, gc, result_set);
	if(plan) {
		ExecutionPlan_Profile(plan);
		ExecutionPlan_Print(plan, ctx);
		ExecutionPlan_Free(plan);
	}

cleanup:
	// Release the read-write lock
	if(lockAcquired) {
		if(readonly)Graph_ReleaseLock(gc->g);
		else Graph_WriterLeave(gc->g);
	}

	ResultSet_Free(result_set);
	AST_Free(ast);
	parse_result_free(parse_result);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
}

