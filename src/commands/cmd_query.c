/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_query.h"
#include "../RG.h"
#include "../errors.h"
#include "cmd_context.h"
#include "../ast/ast.h"
#include "../util/arr.h"
#include "../util/cron.h"
#include "../query_ctx.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../util/cache/cache.h"
#include "../util/thpool/thpool.h"
#include "../execution_plan/execution_plan.h"
#include "execution_ctx.h"

extern threadpool _workerpool; // Declared in module.c

// Query_InnerCtx stores the allocations required to execute a query.
typedef struct {
	GraphContext *gc;
	RedisModuleCtx *ctx;
	QueryCtx *query_ctx;
	ExecutionCtx *exec_ctx;
	CommandCtx *command_ctx;
	bool readonly;
} Query_InnerCtx;

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
			ErrorCtx_SetError("ERR Unable to drop index on :%s(%s): no such index.", label, prop);
		}
	} else {
		ErrorCtx_SetError("ERR Encountered unknown query execution type.");
	}
}

/* _ExecuteQuery accepts a Query_InnerCtx as an argument.
 * It may be called directly by a reader thread or the Redis main thread,
 * or dispatched as a worker thread job. */
static void _ExecuteQuery(void *args) {
	Query_InnerCtx *inner_ctx = args;
	GraphContext *gc = inner_ctx->gc;
	bool readonly = inner_ctx->readonly;
	RedisModuleCtx *ctx = inner_ctx->ctx;
	ExecutionCtx *exec_ctx = inner_ctx->exec_ctx;
	CommandCtx *command_ctx = inner_ctx->command_ctx;

	AST *ast = exec_ctx->ast;
	ExecutionPlan *plan = exec_ctx->plan;
	ExecutionType exec_type = exec_ctx->exec_type;

	// If we have migrated to a writer thread, update thread-local storage and track the CommandCtx.
	if(!readonly) {
		QueryCtx_SetInTLS(inner_ctx->query_ctx);
		command_ctx->writer_thread = true;
		CommandCtx_TrackCtx(command_ctx);
	}

	// Instantiate the query's ResultSet.
	bool compact = command_ctx->compact;
	ResultSetFormatterType resultset_format = (compact) ? FORMATTER_COMPACT : FORMATTER_VERBOSE;
	ResultSet *result_set = NewResultSet(ctx, resultset_format);
	// Indicate a cached execution.
	if(exec_ctx->cached) ResultSet_CachedExecution(result_set);

	QueryCtx_SetResultSet(result_set);

	// Set policy after lock acquisition, avoid resetting policies between readers and writers.
	Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

	if(exec_type == EXECUTION_TYPE_QUERY) {  // query operation
		ExecutionPlan_PreparePlan(plan);
		result_set = ExecutionPlan_Execute(plan);

		// Emit error if query timed out.
		if(ExecutionPlan_Drained(plan)) ErrorCtx_SetError("Query timed out");

		ExecutionPlan_Free(plan);
		exec_ctx->plan = NULL;
	} else if(exec_type == EXECUTION_TYPE_INDEX_CREATE ||
			  exec_type == EXECUTION_TYPE_INDEX_DROP) {
		_index_operation(ctx, gc, ast, exec_type);
	} else {
		ASSERT("Unhandled query type" && false);
	}
	QueryCtx_ForceUnlockCommit();
	ResultSet_Reply(result_set);    // Send result-set back to client.

	// Release the read-write lock.
	if(readonly) Graph_ReleaseLock(gc->g);

	// Log query to slowlog.
	SlowLog *slowlog = GraphContext_GetSlowLog(gc);
	SlowLog_Add(slowlog, command_ctx->command_name, command_ctx->query,
				QueryCtx_GetExecutionTime(), NULL);

	// Clean up.
	ExecutionCtx_Free(exec_ctx);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
	ErrorCtx_Clear();
	ResultSet_Free(result_set);
	rm_free(inner_ctx);
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
	 * In case execution-plan timedout we'll call ExecutionPlan_Free
	 * to drop plan's ref count. */
	ExecutionPlan_Free(plan);
}

void Query_SetTimeOut(uint timeout, ExecutionPlan *plan) {
	// Increase execution plan ref count.
	ExecutionPlan_IncreaseRefCount(plan);
	Cron_AddTask(timeout, QueryTimedOut, plan);
}

void Graph_Query(void *args) {
	bool readonly           = true;
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx     = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc        = CommandCtx_GetGraphContext(command_ctx);

	CommandCtx_TrackCtx(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);

	QueryCtx_BeginTimer(); // Start query timing.

	// Parse query parameters and build an execution plan or retrieve it from the cache.
	ExecutionCtx *exec_ctx = ExecutionCtx_FromQuery(command_ctx->query);

	// If there were any query compile time errors, report them.
	if(ErrorCtx_EncounteredError()) {
		ErrorCtx_EmitException();
		goto cleanup;
	}
	if(exec_ctx->exec_type == EXECUTION_TYPE_INVALID) goto cleanup;

	readonly = AST_ReadOnly(exec_ctx->ast->root);
	if(!readonly && _readonly_cmd_mode(command_ctx)) {
		ErrorCtx_SetError("graph.RO_QUERY is to be executed only on read-only queries");
		ErrorCtx_EmitException();
		goto cleanup;
	}

	// Set the query timeout if one was specified.
	if(command_ctx->timeout != 0) {
		if(!readonly) {
			// Disallow timeouts on write operations to avoid leaving the graph in an inconsistent state.
			ErrorCtx_SetError("Query timeouts may only be specified on read-only queries");
			ErrorCtx_EmitException();
			goto cleanup;
		}

		Query_SetTimeOut(command_ctx->timeout, exec_ctx->plan);
	}

	// Populate the container struct for invoking _ExecuteQuery.
	Query_InnerCtx *inner_ctx = rm_malloc(sizeof(Query_InnerCtx));
	inner_ctx->gc = gc;
	inner_ctx->ctx = ctx;
	inner_ctx->readonly = readonly;
	inner_ctx->exec_ctx = exec_ctx;
	inner_ctx->command_ctx = command_ctx;
	inner_ctx->query_ctx = QueryCtx_GetQueryCtx();

	int flags = RedisModule_GetContextFlags(ctx);
	bool execute_on_main_thread = (flags & (REDISMODULE_CTX_FLAGS_MULTI |
											REDISMODULE_CTX_FLAGS_LUA |
											REDISMODULE_CTX_FLAGS_LOADING));

	if(readonly || execute_on_main_thread) {
		if(readonly) Graph_AcquireReadLock(gc->g);
		_ExecuteQuery(inner_ctx);
	} else {
		// Write queries will be executed on a different thread, clear this thread's QueryCtx.
		QueryCtx_RemoveFromTLS();
		// Untrack the CommandCtx.
		CommandCtx_UntrackCtx(command_ctx);
		// Dispatch work to the writer thread.
		thpool_add_work(_workerpool, _ExecuteQuery, inner_ctx);
	}
	return;

cleanup:
	// Cleanup routine invoked after encountering errors in this function.
	ExecutionCtx_Free(exec_ctx);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
	ErrorCtx_Clear();
}

