/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_query.h"
#include "../ast/ast.h"
#include "../util/arr.h"
#include "cmd_context.h"
#include "../query_ctx.h"
#include "../graph/graph.h"
#include "../util/rmalloc.h"
#include "../util/cache/cache.h"
#include "../execution_plan/execution_plan.h"

static void _index_operation(RedisModuleCtx *ctx, GraphContext *gc,
							 const cypher_astnode_t *index_op) {
	Index *idx = NULL;

	if(cypher_astnode_type(index_op) == CYPHER_AST_CREATE_NODE_PROPS_INDEX) {
		// Retrieve strings from AST node
		const char *label = cypher_ast_label_get_name(cypher_ast_create_node_props_index_get_label(
														  index_op));
		const char *prop = cypher_ast_prop_name_get_value(cypher_ast_create_node_props_index_get_prop_name(
															  index_op, 0));
		QueryCtx_LockForCommit();
		if(GraphContext_AddIndex(&idx, gc, label, prop, IDX_EXACT_MATCH) == INDEX_OK) Index_Construct(idx);
		QueryCtx_UnlockCommit(NULL);
	} else {
		// Retrieve strings from AST node
		const char *label = cypher_ast_label_get_name(cypher_ast_drop_node_props_index_get_label(index_op));
		const char *prop = cypher_ast_prop_name_get_value(cypher_ast_drop_node_props_index_get_prop_name(
															  index_op, 0));
		QueryCtx_LockForCommit();
		int res = GraphContext_DeleteIndex(gc, label, prop, IDX_EXACT_MATCH);
		QueryCtx_UnlockCommit(NULL);

		if(res != INDEX_OK) {
			char *error;
			asprintf(&error, "ERR Unable to drop index on :%s(%s): no such index.", label, prop);
			QueryCtx_SetError(error);
		}
	}
}

static inline bool _check_compact_flag(CommandCtx *command_ctx) {
	// The only additional argument to check currently is whether the query results
	// should be returned in compact form
	return (command_ctx->argc > 3 &&
			!strcasecmp(RedisModule_StringPtrLen(command_ctx->argv[3], NULL), "--compact"));
}

void Graph_Query(void *args) {
	AST *ast = NULL;
	bool lockAcquired = false;
	ExecutionPlan *plan = NULL;
	ResultSet *result_set = NULL;
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc = CommandCtx_GetGraphContext(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);

	QueryCtx_BeginTimer(); // Start query timing.
	const char *query_string;
	cypher_parse_result_t *query_parse_result = NULL;
	// Parse and validate parameters only. Extract query string.
	cypher_parse_result_t *params_parse_result = parse_params(command_ctx->query, &query_string);
	if(params_parse_result == NULL) goto cleanup;

	// Check the LRU cache to see if we already have a plan for this query.
	plan = Cache_GetValue(query_cache, query_string);
	if(plan) goto execute;

	// Parse the query to construct an AST and validate it.
	query_parse_result = parse_query(query_string);
	if(query_parse_result == NULL) goto cleanup;


	bool readonly = AST_ReadOnly(query_parse_result);

	// Prepare the constructed AST for accesses from the module
	ast = AST_Build(query_parse_result);

	bool compact = _check_compact_flag(command_ctx);
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
	QueryCtx_SetResultSet(result_set);
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_QUERY) {  // query operation
		plan = NewExecutionPlan();
		/* Make sure there are no compile-time errors.
		 * We prefer to emit the error only once the entire execution-plan
		 * is constructed in-favour of the time it was encountered
		 * for memory management considerations.
		 * this should be revisited in order to save some time (fail fast). */
		if(QueryCtx_EncounteredError()) {
			if(plan) ExecutionPlan_Free(plan);
			QueryCtx_EmitException();
			goto cleanup;
		}

		if(!plan) goto cleanup;
execute: // TODO ugly jump, consider alternatives
		ExecutionPlan_PreparePlan(plan);
		result_set = ExecutionPlan_Execute(plan);
		ExecutionPlan_Free(plan);
	} else if(root_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX ||
			  root_type == CYPHER_AST_DROP_NODE_PROPS_INDEX) {
		_index_operation(ctx, gc, ast->root);
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
	SlowLog_Add(slowlog, command_ctx->command_name, command_ctx->query, QueryCtx_GetExecutionTime());

	ResultSet_Free(result_set);
	AST_Free(ast);
	parse_result_free(params_parse_result);
	parse_result_free(query_parse_result);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
}

