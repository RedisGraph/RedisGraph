/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "cmd_query.h"
#include "../graph/graph.h"
#include "../index/index.h"
#include "../query_executor.h"
#include "../index/index_type.h"
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

void _index_operation(RedisModuleCtx *ctx, const char *graphName, Graph *g, AST_IndexNode *indexNode) {
  /* Set up nested array response for index creation and deletion.
   * As we need no result set, there is only one top-level element, for statistics.
   * Index_Create or Index_Delete will enqueue one string response
   * to indicate the success of the operation, and the query runtime will be appended
   * after this call returns. */
  RedisModule_ReplyWithArray(ctx, 1);
  RedisModule_ReplyWithArray(ctx, 2);

  int ret;
  switch(indexNode->operation) {
    case CREATE_INDEX:
      ret = Index_Create(ctx, graphName, g, indexNode->label, indexNode->property);
      if (ret == INDEX_OK) {
        RedisModule_ReplyWithSimpleString(ctx, "Added 1 index.");
      } else {
        RedisModule_ReplyWithSimpleString(ctx, "(no changes, no records)");
      }
      break;
    case DROP_INDEX:
      ret = Index_Delete(ctx, graphName, indexNode->label, indexNode->property);
      if (ret == INDEX_OK) {
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
    const char *graph_name = RedisModule_StringPtrLen(qctx->graphName, NULL);

    // Perform query validations before and after ModifyAST
    if (AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;

    ModifyAST(ctx, ast, graph_name);
    if (AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;

    // If this is a write query, acquire write lock.
    if(AST_ReadOnly(ast)) MGraph_AcquireReadLock();
    else MGraph_AcquireWriteLock(ctx);

    // Try to get graph.
    Graph *g = Graph_Get(ctx, qctx->graphName);
    if(!g) {
        if(ast->createNode || ast->mergeNode) {
            g = MGraph_CreateGraph(ctx, qctx->graphName);
            /* TODO: free graph if no entities were created. */
        } else {
            RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
            MGraph_ReleaseLock(ctx);
            goto cleanup;
        }
    }

    if (ast->indexNode) { // index operation
        _index_operation(ctx, graph_name, g, ast->indexNode);
    } else {
        ExecutionPlan *plan = NewExecutionPlan(ctx, g, graph_name, ast, false);
        resultSet = ExecutionPlan_Execute(plan);
        ExecutionPlanFree(plan);
        ResultSet_Replay(resultSet);    // Send result-set back to client.
    }

    // Done accessing graph data, release lock.
    MGraph_ReleaseLock(ctx);

    /* Report execution timing. */
    char* strElapsed;
    double t = simple_toc(qctx->tic) * 1000;
    asprintf(&strElapsed, "Query internal execution time: %.6f milliseconds", t);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);

    // Clean up.
cleanup:
    Free_AST_Query(ast);
    ResultSet_Free(resultSet);
    RedisModule_FreeThreadSafeContext(ctx);    
    RedisModule_UnblockClient(qctx->bc, NULL);
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
