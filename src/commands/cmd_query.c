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

// forward declaration
void Graph_Query(void *args);
static void _ExecuteQuery(void *args);

// GraphQueryCtx stores the allocations required to execute a query.
typedef struct {
	GraphContext *graph_ctx;  // graph context
	RedisModuleCtx *rm_ctx;   // redismodule context
	QueryCtx *query_ctx;      // query context
	ExecutionCtx *exec_ctx;   // execution context
	CommandCtx *command_ctx;  // command context
	bool readonly_query;      // read only query
} GraphQueryCtx;

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

static void _RestartQueryAsWriter(GraphQueryCtx *gq_ctx) {
	ASSERT(gq_ctx != NULL);

	GraphContext  *gc           =  gq_ctx->graph_ctx;
	CommandCtx    *command_ctx  =  gq_ctx->command_ctx;

	GraphContext_IncreaseRefCount(gc);
	CommandCtx_UntrackCtx(command_ctx);
	ThreadPools_AddWorkWriter(Graph_Query, command_ctx);
}

// check to see if query performs only merge operations
// if it does we also require that the merge operation doesn't include a
// ON MATCH directive
// for example: MERGE ({v:1})
//              MATCH (n) WHERE n.x > 2 MERGE (n)-[:R]->({v:1})
//              MERGE (n:L {x:2}) ON CREATE n.v = 1
static bool _optimistic_merge_query(ExecutionCtx *exec_ctx) {
	// quick return if this is an index create/delete query
	if(exec_ctx->exec_type != EXECUTION_TYPE_QUERY) return false;

	AST *ast = exec_ctx->ast;
	// forbiden clauses
	// return false if query contains one of the following clauses
	cypher_astnode_type_t t[3] = {
		CYPHER_AST_SET,
		CYPHER_AST_CREATE,
		CYPHER_AST_DELETE
	};

	// check if query contains a MERGE clause
	if(!AST_ContainsClause(ast, CYPHER_AST_MERGE)) return false;

	if(AST_TreeContainsType(ast->root, CYPHER_AST_ON_MATCH)) return false;

	// verify query doesn't contains any of the forbiden clauses
	for(int i = 0; i < 3; i++) if(AST_ContainsClause(ast, t[i])) return false;

	return true;
}

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

//------------------------------------------------------------------------------
// Query timeout
//------------------------------------------------------------------------------

// timeout handler
void QueryTimedOut(void *pdata) {
	ASSERT(pdata);
	ExecutionPlan *plan = (ExecutionPlan *)pdata;
	ExecutionPlan_Abort(plan->root, EXEC_PLAN_ABORT_TIMEOUT);

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

	GraphQueryCtx   *gq_ctx            =  args;
	GraphContext    *gc                =  gq_ctx->graph_ctx;
	RedisModuleCtx  *rm_ctx            =  gq_ctx->rm_ctx;
	bool            readonly           =  gq_ctx->readonly_query;
	bool            execution_aborted  =  false;
	ExecutionCtx    *exec_ctx          =  gq_ctx->exec_ctx;
	QueryCtx        *query_ctx         =  gq_ctx->query_ctx;
	CommandCtx      *command_ctx       =  gq_ctx->command_ctx;
	AST             *ast               =  exec_ctx->ast;
	ExecutionPlan   *plan              =  exec_ctx->plan;
	ExecutionType   exec_type          =  exec_ctx->exec_type;
	// if we have migrated to a writer thread,
	// update thread-local storage and track the CommandCtx
	if(command_ctx->thread == EXEC_THREAD_WRITER) {
		QueryCtx_SetTLS(query_ctx);
		CommandCtx_TrackCtx(command_ctx);
	}

	//--------------------------------------------------------------------------
	// instantiate ResultSet
	//--------------------------------------------------------------------------
	bool compact = command_ctx->compact;
	ResultSetFormatterType resultset_format =
		(compact) ? FORMATTER_COMPACT : FORMATTER_VERBOSE;
	ResultSet *result_set = NewResultSet(rm_ctx, resultset_format);
	ResultSet_CachedExecution(result_set, exec_ctx->cached);

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

	switch(exec_type) {
		case EXECUTION_TYPE_QUERY:  // query operation
			// set policy after lock acquisition,
			// avoid resetting policies between readers and writers
			Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

			ExecutionPlan_PreparePlan(plan);
			result_set = ExecutionPlan_Execute(plan);

			// see if execution was aborted
			// 1. timeout reached
			// 2. optimistic read wants to write
			ExecutionPlan_AbortReason abort_reason;
			execution_aborted = ExecutionPlan_Aborted(plan, &abort_reason);

			ExecutionPlan_Free(plan);
			exec_ctx->plan = NULL;

			// handle execution plan abort
			if(execution_aborted) {
				switch(abort_reason) {
					case EXEC_PLAN_ABORT_TIMEOUT:
						// emit error if query timed out
						ErrorCtx_SetError("Query timed out");
						break;
					case EXEC_PLAN_ABORT_OPTIMISTIC_READ:
						ASSERT(readonly);
						Graph_ReleaseLock(gc->g);
						_RestartQueryAsWriter(gq_ctx);
						command_ctx = NULL;
						goto cleanup;
						break;
					case EXEC_PLAN_ABORT_NONE:
						ASSERT("Unexpected abort reason");
						break;
					default:
						ASSERT("Unexpected abort reason");
						break;
				}
			}

			break;

		case EXECUTION_TYPE_INDEX_CREATE:
		case EXECUTION_TYPE_INDEX_DROP:
			_index_operation(rm_ctx, gc, ast, exec_type);
			break;
		default:
			ASSERT("Unhandled query type" && false);
			break;
	}

	QueryCtx_ForceUnlockCommit();

	if(readonly) Graph_ReleaseLock(gc->g); // release read lock
	else Graph_WriterLeave(gc->g);

	// send result-set back to client
	ResultSet_Reply(result_set);

	// log query to slowlog
	SlowLog *slowlog = GraphContext_GetSlowLog(gc);
	SlowLog_Add(slowlog, command_ctx->command_name, command_ctx->query,
			QueryCtx_GetExecutionTime(), NULL);

cleanup:
	ResultSet_Free(result_set);
	ExecutionCtx_Free(exec_ctx);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // reset the QueryCtx and free its allocations
	ErrorCtx_Clear();
	GraphQueryCtx_Free(gq_ctx);
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
	AST *ast = exec_ctx->ast;

	// if there were any query compile time errors, report them
	if(ErrorCtx_EncounteredError()) {
		ErrorCtx_EmitException();
		goto cleanup;
	}
	if(exec_ctx->exec_type == EXECUTION_TYPE_INVALID) goto cleanup;

	bool readonly = AST_ReadOnly(ast->root);

	// write query executing via GRAPH.RO_QUERY isn't allowed
	if(!readonly && _readonly_cmd_mode(command_ctx)) {
		ErrorCtx_SetError("graph.RO_QUERY is to be executed only on read-only queries");
		ErrorCtx_EmitException();
		goto cleanup;
	}

	// set the query timeout if one was specified
	if(command_ctx->timeout != 0) {
		// disallow timeouts on write operations to avoid leaving the graph in an inconsistent state
		if(readonly) Query_SetTimeOut(command_ctx->timeout, exec_ctx->plan);
	}

	// populate the container struct for invoking _ExecuteQuery.
	GraphQueryCtx *gq_ctx = NULL;

	if(ThreadPools_AmWriter()) {
		// already on a writer thread
		gq_ctx = GraphQueryCtx_New(gc, ctx, exec_ctx, command_ctx, false);
		_ExecuteQuery(gq_ctx);
	} else if(command_ctx->thread == EXEC_THREAD_MAIN) {
		// execute on Redis main thread
		gq_ctx = GraphQueryCtx_New(gc, ctx, exec_ctx, command_ctx, readonly);
		_ExecuteQuery(gq_ctx);
	} else if(readonly || _optimistic_merge_query(exec_ctx)) {
		// optimistic merge
		// if readonly is true we're executing on a reader worker thread
		gq_ctx = GraphQueryCtx_New(gc, ctx, exec_ctx, command_ctx, true);
		_ExecuteQuery(gq_ctx);
	} else {
		// writer query on reader thread, delegate to a writer thread
		gq_ctx = GraphQueryCtx_New(gc, ctx, exec_ctx, command_ctx, false);
		_DelegateWriter(gq_ctx);
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

