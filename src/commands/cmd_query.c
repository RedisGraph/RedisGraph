/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_query.h"
#include "../RG.h"
#include "cmd_context.h"
#include "../ast/ast.h"
#include "../util/arr.h"
#include "../util/cron.h"
#include "../query_ctx.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../util/cache/cache.h"
#include "../execution_plan/execution_plan.h"
#include "execution_ctx.h"

static void _index_operation(RedisModuleCtx *ctx, GraphContext *gc, AST *ast,
							 ExecutionType exec_type) {
	Index *idx = NULL;
	const cypher_astnode_t *index_op = ast->root;
	if(exec_type == EXECUTION_TYPE_INDEX_CREATE) {
		// Retrieve strings from AST node
		const char *label = cypher_ast_label_get_name(cypher_ast_create_node_props_index_get_label(
														  index_op));
		const char *prop = cypher_ast_prop_name_get_value(cypher_ast_create_node_props_index_get_prop_name(
															  index_op, 0));
		QueryCtx_LockForCommit();
		if(GraphContext_AddIndex(&idx, gc, label, prop, IDX_EXACT_MATCH) == INDEX_OK) Index_Construct(idx);
		QueryCtx_UnlockCommit(NULL);
	} else if(exec_type == EXECUTION_TYPE_INDEX_DROP) {
		// Retrieve strings from AST node
		const char *label = cypher_ast_label_get_name(cypher_ast_drop_node_props_index_get_label(index_op));
		const char *prop = cypher_ast_prop_name_get_value(cypher_ast_drop_node_props_index_get_prop_name(
															  index_op, 0));
		QueryCtx_LockForCommit();
		int res = GraphContext_DeleteIndex(gc, label, prop, IDX_EXACT_MATCH);
		QueryCtx_UnlockCommit(NULL);

		if(res != INDEX_OK) {
			QueryCtx_SetError("ERR Unable to drop index on :%s(%s): no such index.", label, prop);
		}
	} else {
		QueryCtx_SetError("ERR Encountered unknown query execution type.");
	}
}

// Read configuration flags, returning REDIS_MODULE_ERR if flag parsing failed.
static int _read_flags(CommandCtx *command_ctx, bool *compact, long long *timeout) {
	ASSERT(command_ctx);
	ASSERT(compact);
	ASSERT(timeout);

	// set defaults
	*timeout = 0;      // no timeout
	*compact = false;  // verbose

	// GRAPH.QUERY <GRAPH_KEY> <QUERY>
	// make sure we've got more than 3 arguments
	if(command_ctx->argc <= 3) return REDISMODULE_OK;

	// scan arguments
	for(int i = 3; i < command_ctx->argc; i++) {
		const char *arg = RedisModule_StringPtrLen(command_ctx->argv[i], NULL);

		// compact result-set
		if(!strcasecmp(arg, "--compact")) {
			*compact = true;
			continue;
		}

		// query timeout
		if(!strcasecmp(arg, "timeout")) {
			int err = REDISMODULE_ERR;
			if(i < command_ctx->argc - 1) {
				i++; // Set the current argument to the timeout value.
				err = RedisModule_StringToLongLong(command_ctx->argv[i], timeout);
			}

			// Emit error on missing, negative, or non-numeric timeout values.
			if(err != REDISMODULE_OK || *timeout < 0) {
				QueryCtx_SetError("Failed to parse query timeout value");
				return REDISMODULE_ERR;
			}
		}
	}
	return REDISMODULE_OK;
}

inline static bool _readonly_cmd_mode(CommandCtx *ctx) {
	return strcasecmp(CommandCtx_GetCommandName(ctx), "graph.RO_QUERY") == 0;
}

void QueryTimedOut(void *pdata) {
	ASSERT(pdata);
	ExecutionPlan *plan = (ExecutionPlan *)pdata;
	ExecutionPlan_Drain(plan);

	/* Timer may have triggered after execution-plan ran to completion
	 * in which case the original query thread had called ExecutionPlan_Free
	 * decreasing the plan's ref count, but did not free the execution-plan
	 * it is our responsibility to call ExecutionPlan_Free
	 *
	 * Incase execution-plan timedout we'll call ExecutionPlan_Free
	 * to drop plan's ref count. */
	ExecutionPlan_Free(plan);
}

void Query_SetTimeOut(uint timeout, ExecutionPlan *plan) {
	// Increase execution plan ref count.
	ExecutionPlan_IncreaseRefCount(plan);
	Cron_AddTask(timeout, QueryTimedOut, plan);
}

void Graph_Query(void *args) {
	bool lockAcquired = false;
	ResultSet *result_set = NULL;
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc = CommandCtx_GetGraphContext(command_ctx);

	CommandCtx_TrackCtx(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);

	QueryCtx_BeginTimer(); // Start query timing.
	/* Retrive the required execution items and information:
	 * 1. AST
	 * 2. Execution plan (if any)
	 * 3. Whether these items were cached or not */
	AST *ast = NULL;
	bool cached = false;
	ExecutionPlan *plan = NULL;
	ExecutionCtx exec_ctx = ExecutionCtx_FromQuery(command_ctx->query);

	ast = exec_ctx.ast;
	plan = exec_ctx.plan;
	cached = exec_ctx.cached;
	ExecutionType exec_type = exec_ctx.exec_type;
	// See if there were any query compile time errors
	if(QueryCtx_EncounteredError()) {
		QueryCtx_EmitException();
		goto cleanup;
	}
	if(exec_type == EXECUTION_TYPE_INVALID) goto cleanup;

	bool readonly = AST_ReadOnly(ast->root);
	if(!readonly && _readonly_cmd_mode(command_ctx)) {
		QueryCtx_SetError("graph.RO_QUERY is to be executed only on read-only queries");
		QueryCtx_EmitException();
		goto cleanup;
	}

	bool compact;
	long long timeout;
	int res = _read_flags(command_ctx, &compact, &timeout);
	if(res == REDISMODULE_ERR) {
		// Emit error and exit if argument parsing failed.
		QueryCtx_EmitException();
		goto cleanup;
	}

	// Set the query timeout if one was specified.
	if(timeout != 0) {
		if(!readonly) {
			// Disallow timeouts on write operations to avoid leaving the graph in an inconsistent state.
			QueryCtx_SetError("Query timeouts may only be specified on read-only queries");
			QueryCtx_EmitException();
			goto cleanup;
		}

		Query_SetTimeOut(timeout, plan);
	}

	ResultSetFormatterType resultset_format = (compact) ? FORMATTER_COMPACT : FORMATTER_VERBOSE;

	// Acquire the appropriate lock.
	if(readonly) {
		Graph_AcquireReadLock(gc->g);
	} else {
		Graph_WriterEnter(gc->g);  // Single writer.
		/* If this is a writer query we need to re-open the graph key with write flag
		* this notifies Redis that the key is "dirty" any watcher on that key will
		* be notified. */
		CommandCtx_ThreadSafeContextLock(command_ctx);
		{
			GraphContext_MarkWriter(ctx, gc);
		}
		CommandCtx_ThreadSafeContextUnlock(command_ctx);
	}
	lockAcquired = true;

	// Set policy after lock acquisition, avoid resetting policies between readers and writers.
	Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);
	result_set = NewResultSet(ctx, resultset_format);
	// Indicate a cached execution.
	if(cached) ResultSet_CachedExecution(result_set);

	QueryCtx_SetResultSet(result_set);
	if(exec_type == EXECUTION_TYPE_QUERY) {  // query operation
		ExecutionPlan_PreparePlan(plan);
		result_set = ExecutionPlan_Execute(plan);

		// Emit error if query timed out.
		if(ExecutionPlan_Drained(plan)) QueryCtx_SetError("Query timed out");

		ExecutionPlan_Free(plan);
		plan = NULL;
	} else if(exec_type == EXECUTION_TYPE_INDEX_CREATE ||
			  exec_type == EXECUTION_TYPE_INDEX_DROP) {
		_index_operation(ctx, gc, ast, exec_type);
	} else {
		assert("Unhandled query type" && false);
	}
	QueryCtx_ForceUnlockCommit();
	ResultSet_Reply(result_set);    // Send result-set back to client.

	// Clean up.
cleanup:
	// Release the read-write lock
	if(lockAcquired) {
		// TODO In the case of a failing writing query, we may hold both locks:
		// "CREATE (a {num: 1}) MERGE ({v: a.num})"
		if(readonly) Graph_ReleaseLock(gc->g);
		else Graph_WriterLeave(gc->g);
	}

	// Log query to slowlog.
	SlowLog *slowlog = GraphContext_GetSlowLog(gc);
	SlowLog_Add(slowlog, command_ctx->command_name, command_ctx->query,
				QueryCtx_GetExecutionTime(), NULL);
	if(plan) ExecutionPlan_Free(plan);
	ResultSet_Free(result_set);
	AST_Free(ast);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
}

