/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_query.h"
#include "cmd_context.h"
#include "../graph/graph.h"
#include "../parser/newast.h"
#include "../query_executor.h"
#include "../util/simple_timer.h"
#include "../execution_plan/execution_plan.h"
#include "../util/rmalloc.h"
#include "../../deps/libcypher-parser/lib/src/cypher-parser.h"

extern pthread_key_t _tlsASTKey;  // Thread local storage AST key.
extern pthread_key_t _tlsNEWASTKey;  // Thread local storage NEWAST key.

void _index_operation(RedisModuleCtx *ctx, GraphContext *gc, AST_IndexNode *indexNode) {
    /* Set up nested array response for index creation and deletion,
     * Following the response struture of other queries:
     * First element is an empty result-set followed by statistics.
     * We'll enqueue one string response to indicate the operation's success,
     * and the query runtime will be appended after this call returns. */
    RedisModule_ReplyWithArray(ctx, 2); // Two Array
    RedisModule_ReplyWithArray(ctx, 0); // Empty result-set
    RedisModule_ReplyWithArray(ctx, 2); // Statistics.

  switch(indexNode->operation) {
    case CREATE_INDEX:
      if (GraphContext_AddIndex(gc, indexNode->label, indexNode->property) != INDEX_OK) {
        // Index creation may have failed if the label or property was invalid, or the index already exists.
        RedisModule_ReplyWithSimpleString(ctx, "(no changes, no records)");
        break;
      }
      RedisModule_ReplyWithSimpleString(ctx, "Indices added: 1");
      break;
    case DROP_INDEX:
      if (GraphContext_DeleteIndex(gc, indexNode->label, indexNode->property) == INDEX_OK) {
        RedisModule_ReplyWithSimpleString(ctx, "Indices removed: 1");
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
    CommandCtx *qctx = (CommandCtx*)args;
    RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(qctx);
    ResultSet* resultSet = NULL;
    AST **ast = qctx->ast;
    NEWAST *new_ast = qctx->new_ast;
    bool readonly = NEWAST_ReadOnly(new_ast->root);
    bool lockAcquired = false;

    // Set thread-local ASTs (TODO tmp)
    pthread_setspecific(_tlsASTKey, ast);
    pthread_setspecific(_tlsNEWASTKey, new_ast);

    // Try to access the GraphContext
    CommandCtx_ThreadSafeContextLock(qctx);
    GraphContext *gc = GraphContext_Retrieve(ctx, qctx->graphName);
    if(!gc) {
        if (!NEWAST_ContainsClause(new_ast->root, CYPHER_AST_CREATE) &&
            !NEWAST_ContainsClause(new_ast->root, CYPHER_AST_MERGE)) {
            CommandCtx_ThreadSafeContextUnlock(qctx);
            RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
            goto cleanup;
        }
        assert(!readonly);
        gc = GraphContext_New(ctx, qctx->graphName, GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);

        if(!gc) {
            CommandCtx_ThreadSafeContextUnlock(qctx);
            RedisModule_ReplyWithError(ctx, "Graph name already in use as a Redis key.");
            goto cleanup;
        }
        /* TODO: free graph if no entities were created. */
    }
    CommandCtx_ThreadSafeContextUnlock(qctx);

    // Perform query validations
    if (AST_PerformValidations(ctx, new_ast->root) != AST_VALID) goto cleanup;

    ModifyAST(gc, ast, new_ast);

    // Acquire the appropriate lock.
    if(readonly) Graph_AcquireReadLock(gc->g);
    else Graph_WriterEnter(gc->g);  // Single writer.
    lockAcquired = true;

    if (ast[0]->indexNode) { // index operation
        _index_operation(ctx, gc, ast[0]->indexNode);
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
    // Release the read-write lock
    if(lockAcquired) {
        if(readonly)Graph_ReleaseLock(gc->g);
        else Graph_WriterLeave(gc->g);
    }

    ResultSet_Free(resultSet);
    CommandCtx_Free(qctx);
    // cypher_parse_result_free(new_ast);
}

/* Queries graph
 * Args:
 * argv[1] graph name
 * argv[2] query to execute */
int MGraph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    double tic[2];
    if (argc < 3) return RedisModule_WrongArity(ctx);

    simple_tic(tic);

    char *errMsg = NULL;
    const char *query = RedisModule_StringPtrLen(argv[2], NULL);

    // Parse AST.
    cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
    assert(parse_result);

    if(NEWAST_ContainsErrors(parse_result)) {
        errMsg = NEWAST_ReportErrors(parse_result);
        cypher_parse_result_free(parse_result);
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    AST **ast = ParseQuery(query, strlen(query), &errMsg);
    // if (!ast) {
    //     RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
    //     RedisModule_ReplyWithError(ctx, errMsg);
    //     free(errMsg);
    //     return REDISMODULE_OK;
    // }

    NEWAST *new_ast = NEWAST_Build(parse_result);
    bool readonly = NEWAST_ReadOnly(new_ast->root);

    /* Determin query execution context
     * queries issued within a LUA script or multi exec block must
     * run on Redis main thread, others can run on different threads. */
    CommandCtx *context;
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
      // Run query on Redis main thread.
      context = CommandCtx_New(ctx, NULL, ast, new_ast, argv[1], argv, argc);
      context->tic[0] = tic[0];
      context->tic[1] = tic[1];
      _MGraph_Query(context);
    } else {
      // Run query on a dedicated thread.
      RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
      context = CommandCtx_New(NULL, bc, ast, new_ast, argv[1], argv, argc);
      context->tic[0] = tic[0];
      context->tic[1] = tic[1];
      thpool_add_work(_thpool, _MGraph_Query, context);
    }

    // Replicate only if query has potential to modify key space.    
    // if(!readonly) RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}
