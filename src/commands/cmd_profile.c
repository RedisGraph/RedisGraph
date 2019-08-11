/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_profile.h"
#include "cmd_context.h"
#include "../graph/graph.h"
#include "../parser/parser.h"
#include "../util/simple_timer.h"
#include "../execution_plan/execution_plan.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

void _MGraph_Profile(void *args) {
	CommandCtx *qctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(qctx);
	ResultSet *result_set = NULL;
	bool lockAcquired = false;
	AST *ast = NULL;

	// Parse the query to construct an AST
	cypher_parse_result_t *parse_result = parse(qctx->query);
	if(parse_result == NULL) goto cleanup;

	bool readonly = AST_ReadOnly(parse_result);
	// If we are a replica and the query is read-only, no work needs to be done.
	if(readonly && qctx->replicated_command) goto cleanup;

	// Perform query validations
	if(AST_Validate(ctx, parse_result) != AST_VALID) goto cleanup;

	// Prepare the constructed AST for accesses from the module
	ast = AST_Build(parse_result);

	// Try to access the GraphContext
	CommandCtx_ThreadSafeContextLock(qctx);
	GraphContext *gc = GraphContext_Retrieve(ctx, qctx->graphName, readonly);
	if(!gc) {
		if(!AST_ContainsClause(ast, CYPHER_AST_CREATE) &&
		   !AST_ContainsClause(ast, CYPHER_AST_MERGE)) {
			CommandCtx_ThreadSafeContextUnlock(qctx);
			RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
			goto cleanup;
		}
		assert(!readonly);
		gc = GraphContext_New(ctx, qctx->graphName, GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);
		if(!gc) {
			CommandCtx_ThreadSafeContextUnlock(qctx);
			RedisModule_ReplyWithError(ctx, "Graph name already in use as a Redis key.");
			goto cleanup;
		}
		// TODO: free graph if no entities were created.
	}

	CommandCtx_ThreadSafeContextUnlock(qctx);

	// Acquire the appropriate lock.
	if(readonly) Graph_AcquireReadLock(gc->g);
	else Graph_WriterEnter(gc->g);  // Single writer.
	lockAcquired = true;

	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_CREATE_NODE_PROP_INDEX || root_type == CYPHER_AST_DROP_NODE_PROP_INDEX) {
		RedisModule_ReplyWithError(ctx, "Can't profile index operations.");
		goto cleanup;
	} else if(root_type != CYPHER_AST_QUERY) {
		assert("Unhandled query type" && false);
	}

	result_set = NewResultSet(ctx, false);
	ExecutionPlan *plan = NewExecutionPlan(ctx, gc, result_set);
	ExecutionPlan_Profile(plan);
	ExecutionPlan_Print(plan, ctx);
	ExecutionPlan_Free(plan);

cleanup:
	// Release the read-write lock
	if(lockAcquired) {
		if(readonly)Graph_ReleaseLock(gc->g);
		else Graph_WriterLeave(gc->g);
	}

	ResultSet_Free(result_set);
	AST_Free(ast);
	if(parse_result) cypher_parse_result_free(parse_result);
	CommandCtx_Free(qctx);
}

/* Profiles query
 * Args:
 * argv[1] graph name
 * argv[2] query to profile */
int MGraph_Profile(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	if(argc < 3) return RedisModule_WrongArity(ctx);

	/* Determin query execution context
	 * queries issued within a LUA script or multi exec block must
	 * run on Redis main thread, others can run on different threads. */
	CommandCtx *context;
	int flags = RedisModule_GetContextFlags(ctx);
	bool is_replicated = RedisModule_GetContextFlags(ctx) & REDISMODULE_CTX_FLAGS_REPLICATED;
	if(flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
		// Run query on Redis main thread.
		context = CommandCtx_New(ctx, NULL, argv[1], argv[2], argv, argc, is_replicated);
		_MGraph_Profile(context);
	} else {
		// Run query on a dedicated thread.
		RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
		context = CommandCtx_New(NULL, bc, argv[1], argv[2], argv, argc, is_replicated);
		thpool_add_work(_thpool, _MGraph_Profile, context);
	}

	// Replicate the command to slaves and AOF.
	// If the query is read-only, slaves will do nothing after parsing.
	// TODO is this necessary for Profile queries?
	RedisModule_ReplicateVerbatim(ctx);
	return REDISMODULE_OK;
}
