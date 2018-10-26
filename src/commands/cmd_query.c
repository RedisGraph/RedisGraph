/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "cmd_query.h"
#include "../graph/graph.h"
#include "../query_executor.h"
#include "../util/simple_timer.h"
#include "../execution_plan/execution_plan.h"

QueryContext* _queryContext_New(RedisModuleBlockedClient *bc, AST_Query* ast, RedisModuleString *graphName) {
    QueryContext* context = malloc(sizeof(QueryContext));
    context->bc = bc;
    context->ast = ast;
    context->graphName = graphName;
    return context;
}

void _queryContext_Free(QueryContext* ctx) {
    free(ctx);
}

void _index_operation(RedisModuleCtx *ctx, GraphContext *gc, AST_IndexNode *indexNode) {
  Graph *g = gc->g;
  /* Set up nested array response for index creation and deletion.
   * As we need no result set, there is only one top-level element, for statistics.
   * We'll enqueue one string response to indicate the operation's success,
   * and the query runtime will be appended after this call returns. */
  RedisModule_ReplyWithArray(ctx, 1);
  RedisModule_ReplyWithArray(ctx, 2);

  switch(indexNode->operation) {
    case CREATE_INDEX:
      if (GraphContext_GetIndex(gc, indexNode->label, indexNode->property)) {
        RedisModule_ReplyWithSimpleString(ctx, "(no changes, no records)");
      } else {
        // retrieve label matrix
        GraphContext_AddIndex(gc, indexNode->label, indexNode->property);
        RedisModule_ReplyWithSimpleString(ctx, "Added 1 index.");
      }
      break;
    case DROP_INDEX:
      if (GraphContext_DeleteIndex(gc, indexNode->label, indexNode->property) == INDEX_OK) {
        RedisModule_ReplyWithSimpleString(ctx, "Removed 1 index.");
      } else {
        char *reply;
        asprintf(&reply, "ERR Unable to drop index on :%s(%s): no such index.", indexNode->label, indexNode->property);
        RedisModule_ReplyWithError(ctx, reply);
        free(reply);
      }
      break;
    default:
      assert(0);
  }
}

void _MGraph_Query(void *args) {
    QueryContext *qctx = (QueryContext*)args;
    RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(qctx->bc);
    AST_Query* ast = qctx->ast;
    ResultSet* resultSet = NULL;

    bool readonly = AST_ReadOnly(ast);
    // If the query modifies the keyspace, acquire the Redis global lock
    // TODO Think about this, because I think it may be broader here than necessary
    if (!readonly) RedisModule_ThreadSafeContextLock(ctx);

    // Try to get graph context.
    GraphContext *gc = GraphContext_Get(ctx, qctx->graphName, readonly);
    if (!gc && !ast->createNode && !ast->mergeNode) {
        RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
        goto cleanup;
    }

    // Perform query validations before and after ModifyAST
    if (AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;

    ModifyAST(gc, ast);
    if (AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;

    if(!gc) {
        assert(!readonly);
        gc = GraphContext_New(ctx, qctx->graphName);
        /* TODO: free graph if no entities were created. */
    }

    if (ast->indexNode) { // index operation
        _index_operation(ctx, gc, ast->indexNode);
    } else {
        ExecutionPlan *plan = NewExecutionPlan(ctx, gc, ast, false);
        resultSet = ExecutionPlan_Execute(plan);
        ExecutionPlanFree(plan);
        ResultSet_Replay(resultSet);    // Send result-set back to client.
    }

    /* Report execution timing. */
    char* strElapsed;
    double t = simple_toc(qctx->tic) * 1000;
    asprintf(&strElapsed, "Query internal execution time: %.6f milliseconds", t);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);

    // Clean up.
cleanup:
    GraphContext_ReleaseLock(gc);
    // Release Redis global lock if it was acquired
    if (!readonly) {
      RedisModule_ThreadSafeContextUnlock(ctx);
    }
    Free_AST_Query(ast);
    ResultSet_Free(resultSet);
    RedisModule_UnblockClient(qctx->bc, NULL);
    RedisModule_FreeThreadSafeContext(ctx);
    _queryContext_Free(qctx);
}

/* Queries graph
 * Args:
 * argv[1] graph name
 * argv[2] query to execute */
int MGraph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    double tic[2];
    if (argc < 3) return RedisModule_WrongArity(ctx);

    simple_tic(tic);

    // Parse AST.
    // TODO: support concurent parsing.
    char *errMsg = NULL;
    const char *query = RedisModule_StringPtrLen(argv[2], NULL);
    AST_Query* ast = ParseQuery(query, strlen(query), &errMsg);
    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    bool readonly = AST_ReadOnly(ast);

    // Construct concurent query context.
    RedisModuleBlockedClient *bc =
        RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);

    QueryContext *context = _queryContext_New(bc, ast, argv[1]);

    context->tic[0] = tic[0];
    context->tic[1] = tic[1];    

    thpool_add_work(_thpool, _MGraph_Query, context);

    // Replicate only if query has potential to modify key space.
    if(!readonly) RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}
