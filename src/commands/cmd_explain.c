/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_explain.h"
#include "cmd_context.h"
#include "../query_ctx.h"
#include "../index/index.h"
#include "../util/rmalloc.h"
#include "../execution_plan/execution_plan.h"

/* Builds an execution plan but does not execute it
 * reports plan back to the client
 * Args:
 * argv[1] graph name
 * argv[2] query */
void Graph_Explain(void *args) {
	AST *ast = NULL;
	bool lock_acquired = false;
	ExecutionPlan *plan = NULL;
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc = CommandCtx_GetGraphContext(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);
	const char *query = command_ctx->query;

	// Parse the query to construct an AST
	cypher_parse_result_t *parse_result = parse(command_ctx->query);
	if(parse_result == NULL) goto cleanup;

	// Perform query validations
	if(AST_Validate(ctx, parse_result) != AST_VALID) goto cleanup;

	// Prepare the constructed AST for accesses from the module
	ast = AST_Build(parse_result);

	// Handle replies for index creation/deletion
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX) {
		RedisModule_ReplyWithSimpleString(ctx, "Create Index");
		goto cleanup;
	} else if(root_type == CYPHER_AST_DROP_NODE_PROPS_INDEX) {
		RedisModule_ReplyWithSimpleString(ctx, "Drop Index");
		goto cleanup;
	}

	Graph_AcquireReadLock(gc->g);
	lock_acquired = true;

	plan = NewExecutionPlan(NULL);
	if(plan) ExecutionPlan_Print(plan, ctx);

cleanup:
	if(lock_acquired) Graph_ReleaseLock(gc->g);
	if(plan) ExecutionPlan_Free(plan);

	AST_Free(ast);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	parse_result_free(parse_result);
}

