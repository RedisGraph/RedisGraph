/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_query.h"
#include "cmd_context.h"
#include "../graph/graph.h"
#include "../parser/ast.h"
#include "../query_executor.h"
#include "../util/simple_timer.h"
#include "../execution_plan/execution_plan.h"
#include "../util/rmalloc.h"
#include "../../deps/libcypher-parser/lib/src/cypher-parser.h"

extern pthread_key_t _tlsASTKey;  // Thread local storage AST key.

void _index_operation(RedisModuleCtx *ctx, GraphContext *gc, const cypher_astnode_t *index_op) {

    /* Set up nested array response for index creation and deletion,
     * Following the response struture of other queries:
     * First element is an empty result-set followed by statistics.
     * We'll enqueue one string response to indicate the operation's success,
     * and the query runtime will be appended after this call returns. */
    RedisModule_ReplyWithArray(ctx, 2); // Two Array
    RedisModule_ReplyWithArray(ctx, 0); // Empty result-set
    RedisModule_ReplyWithArray(ctx, 2); // Statistics.

    if (cypher_astnode_type(index_op) == CYPHER_AST_CREATE_NODE_PROP_INDEX) {
        // Retrieve strings from AST node
        const char *label = cypher_ast_label_get_name(cypher_ast_create_node_prop_index_get_label(index_op));
        const char *prop = cypher_ast_prop_name_get_value(cypher_ast_create_node_prop_index_get_prop_name(index_op));
        if (GraphContext_AddIndex(gc, label, prop) != INDEX_OK) {
            // Index creation may have failed if the label or property was invalid, or the index already exists.
            RedisModule_ReplyWithSimpleString(ctx, "(no changes, no records)");
            return;
        }
        RedisModule_ReplyWithSimpleString(ctx, "Indices added: 1");
    } else {
        // Retrieve strings from AST node
        const char *label = cypher_ast_label_get_name(cypher_ast_drop_node_prop_index_get_label(index_op));
        const char *prop = cypher_ast_prop_name_get_value(cypher_ast_drop_node_prop_index_get_prop_name(index_op));
        if (GraphContext_DeleteIndex(gc, label, prop) == INDEX_OK) {
            RedisModule_ReplyWithSimpleString(ctx, "Indices removed: 1");
        } else {
            char *reply;
            asprintf(&reply, "ERR Unable to drop index on :%s(%s): no such index.", label, prop);
            RedisModule_ReplyWithError(ctx, reply);
            free(reply);
        }
    }
}

void _MGraph_Query(void *args) {
    CommandCtx *qctx = (CommandCtx*)args;
    RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(qctx);
    ResultSet* resultSet = NULL;
    AST *ast = qctx->ast;
    bool readonly = AST_ReadOnly(ast);
    bool lockAcquired = false;

    // Set thread-local AST
    pthread_setspecific(_tlsASTKey, ast);

    // Try to access the GraphContext
    CommandCtx_ThreadSafeContextLock(qctx);
    GraphContext *gc = GraphContext_Retrieve(ctx, qctx->graphName);
    if(!gc) {
        if (!AST_ContainsClause(ast, CYPHER_AST_CREATE) &&
            !AST_ContainsClause(ast, CYPHER_AST_MERGE)) {
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
    if (AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;

    ModifyAST(gc, ast);

    // Acquire the appropriate lock.
    if(readonly) Graph_AcquireReadLock(gc->g);
    else Graph_WriterEnter(gc->g);  // Single writer.
    lockAcquired = true;

    const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
    if (root_type == CYPHER_AST_QUERY) { // query operation
        ExecutionPlan *plan = NewExecutionPlan(ctx, gc, false);
        resultSet = ExecutionPlan_Execute(plan);
        ExecutionPlanFree(plan);
        ResultSet_Replay(resultSet);    // Send result-set back to client.
    } else if (root_type == CYPHER_AST_CREATE_NODE_PROP_INDEX || root_type == CYPHER_AST_DROP_NODE_PROP_INDEX) {
        _index_operation(ctx, gc, ast->root);
    } else {
        assert("Unhandled query type" && false);
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
    // cypher_parse_result_free(ast);
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

    if(AST_ContainsErrors(parse_result)) {
        errMsg = AST_ReportErrors(parse_result);
        cypher_parse_result_free(parse_result);
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    AST *ast = AST_Build(parse_result);
    bool readonly = AST_ReadOnly(ast);

    /* Determin query execution context
     * queries issued within a LUA script or multi exec block must
     * run on Redis main thread, others can run on different threads. */
    CommandCtx *context;
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
      // Run query on Redis main thread.
      context = CommandCtx_New(ctx, NULL, ast, argv[1], argv, argc);
      context->tic[0] = tic[0];
      context->tic[1] = tic[1];
      _MGraph_Query(context);
    } else {
      // Run query on a dedicated thread.
      RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
      context = CommandCtx_New(NULL, bc, ast, argv[1], argv, argc);
      context->tic[0] = tic[0];
      context->tic[1] = tic[1];
      thpool_add_work(_thpool, _MGraph_Query, context);
    }

    // Replicate only if query has potential to modify key space.    
    if(!readonly) RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}
