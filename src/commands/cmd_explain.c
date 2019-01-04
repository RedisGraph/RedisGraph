/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "cmd_explain.h"
#include "../index/index.h"
#include "../query_executor.h"
#include "../execution_plan/execution_plan.h"

extern pthread_key_t _tlsASTKey;  // Thread local storage AST key.

/* Builds an execution plan but does not execute it
 * reports plan back to the client
 * Args:
 * argv[1] graph name
 * argv[2] query */
int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3) return RedisModule_WrongArity(ctx);

    // Return immediately if invocation context is Lua or a MULTI/EXEC block
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
        RedisModule_ReplyWithError(ctx, "RedisGraph commands may not be called from non-blocking contexts.");
        return REDISMODULE_OK;
    }

    const char *query = RedisModule_StringPtrLen(argv[2], NULL);

    /* Parse query, get AST. */
    char *errMsg = NULL;
    AST* ast = NULL;
    GraphContext *gc = NULL;
    ExecutionPlan *plan = NULL;

    ast = ParseQuery(query, strlen(query), &errMsg);
    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }
    pthread_setspecific(_tlsASTKey, ast);

    // Retrieve the GraphContext and acquire a read lock.
    // RedisModule_ThreadSafeContextLock(ctx);
    gc = GraphContext_Retrieve(ctx, argv[1]);
    // RedisModule_ThreadSafeContextUnlock(ctx);
    if(!gc) {
        RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
        goto cleanup;
    }

    // Perform query validations before and after ModifyAST
    if (AST_PerformValidations(ctx, ast) != AST_VALID) return REDISMODULE_OK;

    ModifyAST(gc, ast);
    if (AST_PerformValidations(ctx, ast) != AST_VALID) return REDISMODULE_OK;

    if (ast->indexNode != NULL) { // index operation
        char *reply = (ast->indexNode->operation == CREATE_INDEX) ? "Create Index" : "Drop Index";
        RedisModule_ReplyWithSimpleString(ctx, reply);
        goto cleanup;
    }

    Graph_AcquireReadLock(gc->g);
    plan = NewExecutionPlan(ctx, gc, ast, true);
    char* strPlan = ExecutionPlanPrint(plan);
    RedisModule_ReplyWithStringBuffer(ctx, strPlan, strlen(strPlan));

cleanup:
    if(plan) {
        Graph_ReleaseLock(gc->g);
        ExecutionPlanFree(plan);
    }
    if(ast) AST_Free(ast);
    return REDISMODULE_OK;
}
