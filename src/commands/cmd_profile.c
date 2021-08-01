/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../errors.h"
#include "cmd_context.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "execution_ctx.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../execution_plan/execution_plan.h"

// GraphQueryCtx stores the allocations required to execute a query.
typedef struct {
	GraphContext *graph_ctx;  // graph context
	RedisModuleCtx *rm_ctx;   // redismodule context
	QueryCtx *query_ctx;      // query context
	ExecutionCtx *exec_ctx;   // execution context
	CommandCtx *command_ctx;  // command context
	bool readonly_query;      // read only query
} GraphQueryCtx;

static GraphQueryCtx *GraphQueryCtx_New
(
	GraphContext *graph_ctx,
	RedisModuleCtx *rm_ctx,
	ExecutionCtx *exec_ctx,
	CommandCtx *command_ctx,
	bool readonly_query
) {
	GraphQueryCtx *ctx = rm_malloc(sizeof(GraphQueryCtx));

	ctx->rm_ctx          =  rm_ctx;
	ctx->exec_ctx        =  exec_ctx;
	ctx->graph_ctx       =  graph_ctx;
	ctx->query_ctx       =  QueryCtx_GetQueryCtx();
	ctx->command_ctx     =  command_ctx;
	ctx->readonly_query  =  readonly_query;

	return ctx;
}

void static inline GraphQueryCtx_Free(GraphQueryCtx *ctx) {
	ASSERT(ctx != NULL);
	rm_free(ctx);
}

/* _ExecuteQuery accepts a GraphQeuryCtx as an argument
 * it may be called directly by a reader thread or the Redis main thread,
 * or dispatched as a worker thread job. */
static void _ExecuteQuery(void *args) {
	ASSERT(args != NULL);

	GraphQueryCtx   *gq_ctx       =  args;
	QueryCtx        *query_ctx    =  gq_ctx->query_ctx;
	GraphContext    *gc           =  gq_ctx->graph_ctx;
	RedisModuleCtx  *rm_ctx       =  gq_ctx->rm_ctx;
	bool            readonly      =  gq_ctx->readonly_query;
	ExecutionCtx    *exec_ctx     =  gq_ctx->exec_ctx;
	CommandCtx      *command_ctx  =  gq_ctx->command_ctx;
	AST             *ast          =  exec_ctx->ast;
	ExecutionPlan   *plan         =  exec_ctx->plan;
	ExecutionType   exec_type     =  exec_ctx->exec_type;

	// if we have migrated to a writer thread,
	// update thread-local storage and track the CommandCtx
	if(command_ctx->thread == EXEC_THREAD_WRITER) {
		QueryCtx_SetTLS(query_ctx);
		CommandCtx_TrackCtx(command_ctx);
	}

	// instantiate the query ResultSet
	ResultSet *result_set = NewResultSet(rm_ctx, FORMATTER_NOP);
	if(exec_ctx->cached) ResultSet_CachedExecution(result_set); // indicate a cached execution

	QueryCtx_SetResultSet(result_set);

	// acquire the appropriate lock
	if(readonly) {
		Graph_AcquireReadLock(gc->g);
	} else {
		/* if this is a writer query `we need to re-open the graph key with write flag
		 * this notifies Redis that the key is "dirty" any watcher on that key will
		 * be notified */
		CommandCtx_ThreadSafeContextLock(command_ctx);
		{
			GraphContext_MarkWriter(rm_ctx, gc);
		}
		CommandCtx_ThreadSafeContextUnlock(command_ctx);
	}

	if(exec_type == EXECUTION_TYPE_QUERY) {  // query operation
		// set policy after lock acquisition,
		// avoid resetting policies between readers and writers
		Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

		ExecutionPlan_PreparePlan(plan);
		ExecutionPlan_Profile(plan);
		ExecutionPlan_Print(plan, rm_ctx);


		ExecutionPlan_Free(plan);
		exec_ctx->plan = NULL;
	} else {
		ASSERT("Unhandled query type" && false);
	}

	QueryCtx_ForceUnlockCommit();

	// send result-set back to client
	ResultSet_Reply(result_set);

	if(readonly) Graph_ReleaseLock(gc->g); // release read lock

	// log query to slowlog
	SlowLog *slowlog = GraphContext_GetSlowLog(gc);
	SlowLog_Add(slowlog, command_ctx->command_name, command_ctx->query,
				QueryCtx_GetExecutionTime(), NULL);

	// clean up
	ExecutionCtx_Free(exec_ctx);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // reset the QueryCtx and free its allocations
	ErrorCtx_Clear();
	ResultSet_Free(result_set);
	GraphQueryCtx_Free(gq_ctx);
}

static void _DelegateWriter(GraphQueryCtx *gq_ctx) {
	ASSERT(gq_ctx != NULL);

	//---------------------------------------------------------------------------
	// Migrate to writer thread
	//---------------------------------------------------------------------------

	// write queries will be executed on a dedicated writer thread,
	// clear this thread data
	ErrorCtx_Clear();
	QueryCtx_RemoveFromTLS();

	// untrack the CommandCtx
	CommandCtx_UntrackCtx(gq_ctx->command_ctx);

	// update execution thread to writer
	gq_ctx->command_ctx->thread = EXEC_THREAD_WRITER;

	// dispatch work to the writer thread
	int res = ThreadPools_AddWorkWriter(_ExecuteQuery, gq_ctx);
	ASSERT(res == 0);
}

void Graph_Profile(void *args) {
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx     = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc        = CommandCtx_GetGraphContext(command_ctx);

	CommandCtx_TrackCtx(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);

	QueryCtx_BeginTimer(); // Start query timing.

	// parse query parameters and build an execution plan or retrieve it from the cache
	ExecutionCtx *exec_ctx = ExecutionCtx_FromQuery(command_ctx->query);
	if(exec_ctx == NULL) goto cleanup;

	ExecutionType exec_type = exec_ctx->exec_type;

	if(exec_type == EXECUTION_TYPE_INDEX_CREATE ||
	   exec_type == EXECUTION_TYPE_INDEX_DROP) {
		RedisModule_ReplyWithError(ctx, "Can't profile index operations.");
		goto cleanup;
	}

	bool readonly = AST_ReadOnly(exec_ctx->ast->root);

	// populate the container struct for invoking _ExecuteQuery.
	GraphQueryCtx *gq_ctx = GraphQueryCtx_New(gc, ctx, exec_ctx, command_ctx,
											  readonly);

	// if 'thread' is redis main thread, continue running
	// if readonly is true we're executing on a worker thread from
	// the read-only threadpool
	if(readonly || command_ctx->thread == EXEC_THREAD_MAIN) {
		_ExecuteQuery(gq_ctx);
	} else {
		_DelegateWriter(gq_ctx);
	}

	return;

cleanup:
	// if there were any query compile time errors, report them
	if(ErrorCtx_EncounteredError()) ErrorCtx_EmitException();

	// Cleanup routine invoked after encountering errors in this function.
	ExecutionCtx_Free(exec_ctx);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
	ErrorCtx_Clear();
}
