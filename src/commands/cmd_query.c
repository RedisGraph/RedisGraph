/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "../errors.h"
#include "cmd_context.h"
#include "../ast/ast.h"
#include "../util/arr.h"
#include "../util/cron.h"
#include "../query_ctx.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../util/cache/cache.h"
#include "../util/thpool/pools.h"
#include "../execution_plan/execution_plan.h"
#include "execution_ctx.h"

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

static void _index_operation(RedisModuleCtx *ctx, GraphContext *gc, AST *ast,
							 ExecutionType exec_type) {
	Index *idx = NULL;
	const cypher_astnode_t *index_op = ast->root;
	if(exec_type == EXECUTION_TYPE_INDEX_CREATE) {
		// Retrieve strings from AST node
		bool index_added = false;
		unsigned int nprops = cypher_ast_create_node_props_index_nprops(index_op);
		const char *label = cypher_ast_label_get_name(cypher_ast_create_node_props_index_get_label(
												index_op));
		// add index for each property
		QueryCtx_LockForCommit();
		for(unsigned int i = 0; i < nprops; i++) {
			const char *prop = cypher_ast_prop_name_get_value(cypher_ast_create_node_props_index_get_prop_name(
																index_op, i));
			index_added |= (GraphContext_AddIndex(&idx, gc, label, prop, IDX_EXACT_MATCH) == INDEX_OK);
		}
		// populate the index only when at least one attribute was introduced
		if(index_added) Index_Construct(idx);
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

//------------------------------------------------------------------------------
// Query timeout
//------------------------------------------------------------------------------

// timeout handler
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

// set timeout for query execution
void Query_SetTimeOut(uint timeout, ExecutionPlan *plan) {
	// increase execution plan ref count
	ExecutionPlan_IncreaseRefCount(plan);
	Cron_AddTask(timeout, QueryTimedOut, plan);
}

inline static bool _readonly_cmd_mode(CommandCtx *ctx) {
	return strcasecmp(CommandCtx_GetCommandName(ctx), "graph.RO_QUERY") == 0;
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
	bool compact = command_ctx->compact;
	ResultSetFormatterType resultset_format = (compact) ? FORMATTER_COMPACT : FORMATTER_VERBOSE;
	ResultSet *result_set = NewResultSet(rm_ctx, resultset_format);
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
		Graph_WriterEnter(gc->g);  // single writer
	}

	if(exec_type == EXECUTION_TYPE_QUERY) {  // query operation
		// set policy after lock acquisition,
		// avoid resetting policies between readers and writers
		Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

		ExecutionPlan_PreparePlan(plan);
		result_set = ExecutionPlan_Execute(plan);

		// Emit error if query timed out.
		if(ExecutionPlan_Drained(plan)) ErrorCtx_SetError("Query timed out");

		ExecutionPlan_Free(plan);
		exec_ctx->plan = NULL;
	} else if(exec_type == EXECUTION_TYPE_INDEX_CREATE ||
			  exec_type == EXECUTION_TYPE_INDEX_DROP) {
		_index_operation(rm_ctx, gc, ast, exec_type);
	} else {
		ASSERT("Unhandled query type" && false);
	}

	QueryCtx_ForceUnlockCommit();

	// send result-set back to client
	ResultSet_Reply(result_set);

	if(readonly) Graph_ReleaseLock(gc->g); // release read lock
	else Graph_WriterLeave(gc->g);

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

void Graph_Query(void *args) {
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx     = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc        = CommandCtx_GetGraphContext(command_ctx);

	CommandCtx_TrackCtx(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);

	QueryCtx_BeginTimer(); // start query timing

	// parse query parameters and build an execution plan or retrieve it from the cache
	ExecutionCtx *exec_ctx = ExecutionCtx_FromQuery(command_ctx->query);
	if(exec_ctx == NULL) goto cleanup;

	bool readonly = AST_ReadOnly(exec_ctx->ast->root);

	// write query executing via GRAPH.RO_QUERY isn't allowed
	if(!readonly && _readonly_cmd_mode(command_ctx)) {
		ErrorCtx_SetError("graph.RO_QUERY is to be executed only on read-only queries");
		goto cleanup;
	}

	// set the query timeout if one was specified
	if(command_ctx->timeout != 0) {
		// disallow timeouts on write operations to avoid leaving the graph in an inconsistent state
		if(readonly) Query_SetTimeOut(command_ctx->timeout, exec_ctx->plan);
	}

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

