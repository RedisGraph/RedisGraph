/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_explain.h"
#include "cmd_context.h"
#include "../index/index.h"
#include "../util/rmalloc.h"
#include "../query_executor.h"
#include "../execution_plan/execution_plan.h"

extern pthread_key_t _tlsGCKey;    // Thread local storage graph context key.

GraphContext* _empty_graph_context() {
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
    CommandCtx *qctx = (CommandCtx*)args;
    RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(qctx);
    AST **ast = qctx->ast;
    GraphContext *gc = NULL;
    ExecutionPlan *plan = NULL;
    bool free_graph_ctx = false;
    const char *graphname = qctx->graphName;

    // Perform query validations before and after ModifyAST.
    if(AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;
    ModifyAST(ast);
    if(AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;

    // index operation.
    if(ast[0]->indexNode != NULL) {
        RedisModule_ReplyWithArray(ctx, 1);
        char *reply = (ast[0]->indexNode->operation == CREATE_INDEX) ? "Create Index" : "Drop Index";
        RedisModule_ReplyWithSimpleString(ctx, reply);
        goto cleanup;
    }

    // Retrieve the GraphContext and acquire a read lock.
    if(graphname) {
        gc = GraphContext_Retrieve(ctx, graphname);
        if(!gc) {
            RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
            goto cleanup;
        }
    } else {
        free_graph_ctx = true;
        gc = _empty_graph_context();
    }

    Graph_AcquireReadLock(gc->g);
    plan = NewExecutionPlan(ctx, ast, NULL, true);
    ExecutionPlan_Print(plan, ctx);

cleanup:
    if(plan) {
        Graph_ReleaseLock(gc->g);
        ExecutionPlanFree(plan);
    }

    CommandCtx_Free(qctx);
    if(free_graph_ctx) GraphContext_Free(gc);
}

int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc < 2) return RedisModule_WrongArity(ctx);

    const char *query;
    RedisModuleString *graphName = NULL;

    if(argc == 2) {
        query = RedisModule_StringPtrLen(argv[1], NULL);
    } else {
        graphName = argv[1];
        query = RedisModule_StringPtrLen(argv[2], NULL);
    }

    /* Parse query, get AST. */
    char *errMsg = NULL;
    AST** ast = ParseQuery(query, strlen(query), &errMsg);
    if(!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    if(AST_Empty(ast[0])) {
        AST_Free(ast);
        RedisModule_ReplyWithError(ctx, "Error empty query.");
        return REDISMODULE_OK;
    }

    /* Determin execution context
     * queries issued within a LUA script or multi exec block must
     * run on Redis main thread, others can run on different threads. */
    CommandCtx *context;
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
      // Run on Redis main thread.
      context = CommandCtx_New(ctx, NULL, ast, graphName, argv, argc);
      _MGraph_Explain(context);
    } else {
      // Run on a dedicated thread.
      RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
      context = CommandCtx_New(NULL, bc, ast, graphName, argv, argc);
      thpool_add_work(_thpool, _MGraph_Explain, context);
    }

    return REDISMODULE_OK;
}
