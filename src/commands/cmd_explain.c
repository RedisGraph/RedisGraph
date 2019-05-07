/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
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

    const char *graphname = RedisModule_StringPtrLen(argv[1], NULL);
    const char *query = RedisModule_StringPtrLen(argv[2], NULL);

    /* Parse query, get AST. */
    GraphContext *gc = NULL;
    ExecutionPlan *plan = NULL;

    cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
    AST *ast = AST_Build(parse_result);

    pthread_setspecific(_tlsASTKey, ast);

    // Retrieve the GraphContext and acquire a read lock.
    gc = GraphContext_Retrieve(ctx, graphname);
    if(!gc) {
        RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
        goto cleanup;
    }

    // Perform query validations before and after ModifyAST
    if(AST_PerformValidations(ctx, ast) != AST_VALID) return REDISMODULE_OK;

    // TODO index ops
    // if (ast[0]->indexNode != NULL) { // index operation
        // char *reply = (ast[0]->indexNode->operation == CREATE_INDEX) ? "Create Index" : "Drop Index";
        // RedisModule_ReplyWithSimpleString(ctx, reply);
        // goto cleanup;
    // }

    Graph_AcquireReadLock(gc->g);
    plan = NewExecutionPlan(ctx, gc, true);
    char* strPlan = ExecutionPlan_Print(plan);
    RedisModule_ReplyWithStringBuffer(ctx, strPlan, strlen(strPlan));

cleanup:
    if(plan) {
        Graph_ReleaseLock(gc->g);
        ExecutionPlanFree(plan);
    }
    if(parse_result) cypher_parse_result_free(parse_result);
    return REDISMODULE_OK;
}
