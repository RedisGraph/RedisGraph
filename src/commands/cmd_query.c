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
#include "../execution_plan/execution_plan.h"

static void _index_operation(RedisModuleCtx *ctx, GraphContext *gc,
							 const cypher_astnode_t *index_op) {
	Index *idx = NULL;

	if(cypher_astnode_type(index_op) == CYPHER_AST_CREATE_NODE_PROPS_INDEX) {
		if(cypher_ast_create_node_props_index_nprops(index_op) > 1) {
			// Reply with error if the query specifies multiple properties to index.
			// TODO Remove this limitation when composite indexes are added.
			char *error;
			asprintf(&error, "RedisGraph does not currently support composite indexes.");
			QueryCtx_SetError(error);
			return;
		}

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
	ResultSet *result_set = NULL;
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc = CommandCtx_GetGraphContext(command_ctx);
	QueryCtx_SetGlobalExecutionCtx(command_ctx);

	QueryCtx_BeginTimer(); // Start query timing.

	// Parse the query to construct an AST.
	cypher_parse_result_t *parse_result = parse(command_ctx->query);
	if(parse_result == NULL) goto cleanup;

	// Perform query validations
	if(AST_Validate(ctx, parse_result) != AST_VALID) goto cleanup;

	bool readonly = AST_ReadOnly(parse_result);

	// Prepare the constructed AST for accesses from the module
	ast = AST_Build(parse_result);

	bool compact = _check_compact_flag(command_ctx);
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
	result_set = NewResultSet(ctx, compact);
	QueryCtx_SetResultSet(result_set);
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_QUERY) {  // query operation
		ExecutionPlan *plan = NewExecutionPlan();
		/* Make sure there are no compile-time errors.
		 * We prefer to emit the error only once the entire execution-plan
		 * is constructed in-favour of the time it was encountered
		 * for memory management considerations.
		 * this should be revisited in order to save some time (fail fast). */
		if(QueryCtx_EncounteredError()) {
			/* TODO: move ExecutionPlan_Free to `cleanup`
			 * once no all pendding operation commitment (create,delete,update)
			 * are no performed in free callback. */
			if(plan) ExecutionPlan_Free(plan);
			QueryCtx_EmitException();
			goto cleanup;
		}

		if(!plan) goto cleanup;
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

	ResultSet_Free(result_set);
	AST_Free(ast);
	parse_result_free(parse_result);
	GraphContext_Release(gc);
	CommandCtx_Free(command_ctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
}

