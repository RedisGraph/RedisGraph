/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "cmd_query.h"
#include "../graph/graph.h"
#include "../parser/newast.h"
#include "../query_executor.h"
#include "../util/simple_timer.h"
#include "../execution_plan/execution_plan.h"
#include "../util/rmalloc.h"
#include "../../deps/libcypher-parser/lib/src/cypher-parser.h"


extern pthread_key_t _tlsASTKey;  // Thread local storage AST key.
extern pthread_key_t _tlsNEWASTKey;  // Thread local storage NEWAST key.

QueryContext* _queryContext_New(RedisModuleBlockedClient *bc,
                                AST* ast,
                                RedisModuleString *graphName,
                                RedisModuleString *query) {
    QueryContext* context = malloc(sizeof(QueryContext));
    context->bc = bc;
    context->ast = ast;
    context->query = query;
    context->graphName = rm_strdup(RedisModule_StringPtrLen(graphName, NULL));
    return context;
}

void _queryContext_Free(QueryContext* ctx) {
    rm_free(ctx->graphName);
    free(ctx);
}

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
    QueryContext *qctx = (QueryContext*)args;
    RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(qctx->bc);
    ResultSet* resultSet = NULL;
    AST* ast = qctx->ast;
    bool lockAcquired = false;

    /* New parser */
    const char *query = RedisModule_StringPtrLen(qctx->query, NULL);
    printf("query: %s\n", query);
    cypher_parse_result_t *new_ast = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);

    char *parse_error_reason = NULL;
    // Perform query validations
    if (AST_PerformValidations(ctx, new_ast) != AST_VALID) goto cleanup;

    // cypher_parse_result_fprint_ast(new_ast, stdout, 0, NULL, 0);

    bool readonly = NEWAST_ReadOnly(new_ast);

    /* END OF New parser */

    // Add AST to thread local storage.
    pthread_setspecific(_tlsASTKey, ast);
    pthread_setspecific(_tlsNEWASTKey, new_ast);

    // Try to access the GraphContext
    RedisModule_ThreadSafeContextLock(ctx);
    GraphContext *gc = GraphContext_Retrieve(ctx, qctx->graphName);
    RedisModule_ThreadSafeContextUnlock(ctx);
    
    if(!gc) {
        if (!NEWAST_ContainsClause(new_ast, "create") &&
            !NEWAST_ContainsClause(new_ast, "merge")) {
            RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
            goto cleanup;
        }
        assert(!readonly);

        RedisModule_ThreadSafeContextLock(ctx);
        gc = GraphContext_New(ctx, qctx->graphName, GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);
        RedisModule_ThreadSafeContextUnlock(ctx);
        
        if (!gc) {
            RedisModule_ReplyWithError(ctx, "Graph name already in use as a Redis key.");
            goto cleanup;
        }
        /* TODO: free graph if no entities were created. */
    }
    
    ModifyAST(gc, ast, new_ast);

    // Acquire the appropriate lock.
    if(readonly) Graph_AcquireReadLock(gc->g);
    else Graph_WriterEnter(gc->g);  // Single writer.
    lockAcquired = true;

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
    // Release the read-write lock
    if(lockAcquired) {
        if(readonly)Graph_ReleaseLock(gc->g);
        else Graph_WriterLeave(gc->g);
    }

    ResultSet_Free(resultSet);
    if(ast)AST_Free(ast);
    // cypher_parse_result_free(new_ast);
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

    // Return immediately if invocation context is Lua or a MULTI/EXEC block
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
        RedisModule_ReplyWithError(ctx, "RedisGraph commands may not be called from non-blocking contexts.");
        return REDISMODULE_OK;
    }

    simple_tic(tic);

    char *errMsg = NULL;
    const char *query = RedisModule_StringPtrLen(argv[2], NULL);

    // Parse AST.
    cypher_parse_result_t *new_ast = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
    if (new_ast == NULL) return REDISMODULE_OK;

    if(NEWAST_ContainsErrors(new_ast)) {
        errMsg = NEWAST_ReportErrors(new_ast);
        cypher_parse_result_free(new_ast);
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    // cypher_parse_result_fprint_ast(new_ast, stdout, 0, NULL, 0);

    AST* ast = ParseQuery(query, strlen(query), &errMsg);
    // if (!ast) {
    //     RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
    //     RedisModule_ReplyWithError(ctx, errMsg);
    //     free(errMsg);
    //     return REDISMODULE_OK;
    // }

    bool readonly = NEWAST_ReadOnly(new_ast);
    cypher_parse_result_free(new_ast);

    // Construct concurent query context.
    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);

    QueryContext *context = _queryContext_New(bc, ast, argv[1], argv[2]);

    context->tic[0] = tic[0];
    context->tic[1] = tic[1];    

    thpool_add_work(_thpool, _MGraph_Query, context);

    // Replicate only if query has potential to modify key space.
    if(!readonly) RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}
