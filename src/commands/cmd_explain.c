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

/* Builds an execution plan but does not execute it
 * reports plan back to the client
 * Args:
 * argv[1] graph name
 * argv[2] query */
int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3) return RedisModule_WrongArity(ctx);

    const char *query = RedisModule_StringPtrLen(argv[2], NULL);

    /* Parse query, get AST. */
    char *errMsg = NULL;
    AST_Query* ast = ParseQuery(query, strlen(query), &errMsg);

    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    ExecutionPlan *plan = NULL;
    // Retrieve the GraphContext and acquire a read lock.
    GraphContext *gc = GraphContext_Retrieve(ctx, argv[1], true);
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

    plan = NewExecutionPlan(ctx, gc, ast, true);
    char* strPlan = ExecutionPlanPrint(plan);
    RedisModule_ReplyWithStringBuffer(ctx, strPlan, strlen(strPlan));

cleanup:
    GraphContext_Release(gc);
    ExecutionPlanFree(plan);
    Free_AST_Query(ast);
    return REDISMODULE_OK;
}

