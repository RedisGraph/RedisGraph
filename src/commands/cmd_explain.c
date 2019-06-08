/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_explain.h"
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
int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc < 2) return RedisModule_WrongArity(ctx);
    
    const char *query;
    const char *graphname = NULL;
    bool free_graph_ctx = false;

    if(argc == 2) {
        query = RedisModule_StringPtrLen(argv[1], NULL);
    } else {
        graphname = RedisModule_StringPtrLen(argv[1], NULL);
        query = RedisModule_StringPtrLen(argv[2], NULL);
    }

    /* Parse query, get AST. */
    AST** ast = NULL;
    char *errMsg = NULL;
    GraphContext *gc = NULL;
    ExecutionPlan *plan = NULL;

    ast = ParseQuery(query, strlen(query), &errMsg);
    if(!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }
    if(AST_Empty(ast[0])) {
        RedisModule_ReplyWithError(ctx, "Error empty query.");
        goto cleanup;
    }

    // Perform query validations before and after ModifyAST.
    if(AST_PerformValidations(ctx, ast) != AST_VALID) return REDISMODULE_OK;
    ModifyAST(ast);
    if(AST_PerformValidations(ctx, ast) != AST_VALID) return REDISMODULE_OK;

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
    if(ast) AST_Free(ast);
    if(free_graph_ctx) GraphContext_Free(gc);
    return REDISMODULE_OK;
}
