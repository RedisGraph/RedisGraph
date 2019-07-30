/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_explain.h"
#include "cmd_context.h"
#include "../index/index.h"
#include "../util/rmalloc.h"
#include "../execution_plan/execution_plan.h"

extern pthread_key_t _tlsASTKey;  // Thread local storage AST key.
extern pthread_key_t _tlsGCKey;    // Thread local storage graph context key.

GraphContext *_empty_graph_context() {
	GraphContext *gc = NULL;
	gc = rm_malloc(sizeof(GraphContext));
	gc->g = Graph_New(1, 1);
	gc->index_count = 0;
	gc->attributes = NULL;
	gc->node_schemas = NULL;
	gc->string_mapping = NULL;
	gc->relation_schemas = NULL;
	gc->graph_name = rm_strdup("");

	pthread_setspecific(_tlsGCKey, gc);
	return gc;
}

/* Builds an execution plan but does not execute it
 * reports plan back to the client
 * Args:
 * argv[1] graph name [optional]
 * argv[2] query */
void _MGraph_Explain(void *args) {
	CommandCtx *qctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(qctx);
	GraphContext *gc = NULL;
	ExecutionPlan *plan = NULL;
	bool free_graph_ctx = false;
	AST *ast = NULL;
	const char *graphname = qctx->graphName;
	const char *query = qctx->query;

	// Parse the query to construct an AST
	cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
	if(parse_result == NULL) goto cleanup;

	// Perform query validations
	if(AST_Validate(ctx, parse_result) != AST_VALID) goto cleanup;

	// Prepare the constructed AST for accesses from the module
	ast = AST_Build(parse_result);

	// Handle replies for index creation/deletion
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_CREATE_NODE_PROP_INDEX) {
		RedisModule_ReplyWithSimpleString(ctx, "Create Index");
		goto cleanup;
	} else if(root_type == CYPHER_AST_DROP_NODE_PROP_INDEX) {
		RedisModule_ReplyWithSimpleString(ctx, "Drop Index");
		goto cleanup;
	}

	// Retrieve the GraphContext and acquire a read lock.
	if(graphname) {
		gc = GraphContext_Retrieve(ctx, graphname, true);
		if(!gc) {
			RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
			goto cleanup;
		}
	} else {
		free_graph_ctx = true;
		gc = _empty_graph_context();
	}

	Graph_AcquireReadLock(gc->g);
	plan = NewExecutionPlan(ctx, gc, NULL);
	ExecutionPlan_Print(plan, ctx);

cleanup:
	if(plan) {
		Graph_ReleaseLock(gc->g);
		ExecutionPlan_Free(plan);
	}

	AST_Free(ast);
	if(parse_result) cypher_parse_result_free(parse_result);
	CommandCtx_Free(qctx);
	if(free_graph_ctx) GraphContext_Free(gc);
}

int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	if(argc < 2) return RedisModule_WrongArity(ctx);

	RedisModuleString *graphName = NULL;
	RedisModuleString *query;

	if(argc == 2) {
		query = argv[1];
	} else {
		graphName = argv[1];
		query = argv[2];
	}

	/* Determin execution context
	 * queries issued within a LUA script or multi exec block must
	 * run on Redis main thread, others can run on different threads. */
	CommandCtx *context;
	int flags = RedisModule_GetContextFlags(ctx);
	if(flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
		// Run on Redis main thread.
		context = CommandCtx_New(ctx, NULL, graphName, query, argv, argc, false);
		_MGraph_Explain(context);
	} else {
		// Run on a dedicated thread.
		RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
		context = CommandCtx_New(NULL, bc, graphName, query, argv, argc, false);
		thpool_add_work(_thpool, _MGraph_Explain, context);
	}

	return REDISMODULE_OK;
}
