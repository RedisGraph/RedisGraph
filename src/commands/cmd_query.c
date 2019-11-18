/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
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
	/* Set up nested array response for index creation and deletion,
	 * Following the response struture of other queries:
	 * First element is an empty result-set followed by statistics.
	 * We'll enqueue one string response to indicate the operation's success,
	 * and the query runtime will be appended after this call returns. */
	RedisModule_ReplyWithArray(ctx, 2); // Two Arrays
	RedisModule_ReplyWithArray(ctx, 0); // Empty result-set
	RedisModule_ReplyWithArray(ctx, 2); // Statistics.
	Index *idx = NULL;

	if(cypher_astnode_type(index_op) == CYPHER_AST_CREATE_NODE_PROPS_INDEX) {
		// Retrieve strings from AST node
		const char *label = cypher_ast_label_get_name(cypher_ast_create_node_props_index_get_label(
														  index_op));
		const char *prop = cypher_ast_prop_name_get_value(cypher_ast_create_node_props_index_get_prop_name(
															  index_op, 0));
		if(GraphContext_AddIndex(&idx, gc, label, prop, IDX_EXACT_MATCH) != INDEX_OK) {
			// Index creation may have failed if the label or property was invalid, or the index already exists.
			RedisModule_ReplyWithSimpleString(ctx, "(no changes, no records)");
		} else {
			Index_Construct(idx);
			RedisModule_ReplyWithSimpleString(ctx, "Indices added: 1");
		}
	} else {
		// Retrieve strings from AST node
		const char *label = cypher_ast_label_get_name(cypher_ast_drop_node_props_index_get_label(index_op));
		const char *prop = cypher_ast_prop_name_get_value(cypher_ast_drop_node_props_index_get_prop_name(
															  index_op, 0));
		if(GraphContext_DeleteIndex(gc, label, prop, IDX_EXACT_MATCH) == INDEX_OK) {
			RedisModule_ReplyWithSimpleString(ctx, "Indices removed: 1");
		} else {
			char *reply;
			asprintf(&reply, "ERR Unable to drop index on :%s(%s): no such index.", label, prop);
			RedisModule_ReplyWithError(ctx, reply);
			free(reply);
		}
	}

	/* Report execution timing. */
	ResultSet_ReportQueryRuntime(ctx);
}

static inline bool _check_compact_flag(CommandCtx *qctx) {
	// The only additional argument to check currently is whether the query results
	// should be returned in compact form
	return (qctx->argc > 3 &&
			!strcasecmp(RedisModule_StringPtrLen(qctx->argv[3], NULL), "--compact"));
}

void Graph_Query(void *args) {
	AST *ast = NULL;
	bool lockAcquired = false;
	ResultSet *result_set = NULL;
	CommandCtx *qctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(qctx);
	GraphContext *gc = CommandCtx_GetGraphContext(qctx);
	QueryCtx_SetGraphCtx(gc);

	QueryCtx_BeginTimer(); // Start query timing.
	QueryCtx_SetRedisModuleCtx(ctx);

	// Parse the query to construct an AST.
	cypher_parse_result_t *parse_result = parse(qctx->query);
	if(parse_result == NULL) goto cleanup;
	bool readonly = AST_ReadOnly(parse_result);
	// If we are a replica and the query is read-only, no work needs to be done.
	if(readonly && qctx->replicated_command) goto cleanup;

	// Perform query validations
	if(AST_Validate(ctx, parse_result) != AST_VALID) goto cleanup;
	// Prepare the constructed AST for accesses from the module
	ast = AST_Build(parse_result);

	bool compact = _check_compact_flag(qctx);
	// Acquire the appropriate lock.
	if(readonly) {
		Graph_AcquireReadLock(gc->g);
	} else {
		Graph_WriterEnter(gc->g);  // Single writer.
		/* If this is a writer query we need to re-open the graph key with write flag
		* this notifies Redis that the key is "dirty" any watcher on that key will
		* be notified. */
		CommandCtx_ThreadSafeContextLock(qctx);
		{
			GraphContext_MarkWriter(ctx, gc);
		}
		CommandCtx_ThreadSafeContextUnlock(qctx);
	}
	lockAcquired = true;

	// Set policy after lock acquisition, avoid resetting policies between readers and writers.
	Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_QUERY) {  // query operation
		result_set = NewResultSet(ctx, compact);
		ExecutionPlan *plan = NewExecutionPlan(ctx, gc, result_set);
		if(!plan) goto cleanup;
		result_set = ExecutionPlan_Execute(plan);
		ExecutionPlan_Free(plan);
		ResultSet_Replay(result_set);    // Send result-set back to client.
	} else if(root_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX ||
			  root_type == CYPHER_AST_DROP_NODE_PROPS_INDEX) {
		_index_operation(ctx, gc, ast->root);
	} else {
		assert("Unhandled query type" && false);
	}

	// Clean up.
cleanup:
	// Release the read-write lock
	if(lockAcquired) {
		// TODO In the case of a failing writing query, we may hold both locks:
		// "CREATE (a {num: 1}) MERGE ({v: a.num})"
		if(readonly)Graph_ReleaseLock(gc->g);
		else Graph_WriterLeave(gc->g);
	}

	ResultSet_Free(result_set);
	AST_Free(ast);
	parse_result_free(parse_result);
	GraphContext_Release(gc);
	CommandCtx_Free(qctx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
}

