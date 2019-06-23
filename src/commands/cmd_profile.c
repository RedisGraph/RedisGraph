/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_profile.h"
#include "cmd_context.h"
#include "../graph/graph.h"
#include "../query_executor.h"
#include "../util/simple_timer.h"
#include "../execution_plan/execution_plan.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

static ResultSet* _prepare_resultset(RedisModuleCtx *ctx, AST **ast) {
    // The last AST will contain the return clause, if one is specified.
    AST *final_ast = ast[array_len(ast)-1];
    ResultSet *set = NewResultSet(final_ast, ctx, false);

    // Set result-set formatter to a NOP formatter.
    ResultSet_SetReplyFormatter(set, FORMATTER_NOP);

    // Fake header.
    set->header = rm_malloc(sizeof(ResultSetHeader));
    set->header->columns_len = 0;
    set->header->columns = NULL;

    return set;
}

void _MGraph_Profile(void *args) {
    CommandCtx *qctx = (CommandCtx*)args;
    AST **ast = qctx->ast;
    bool lockAcquired = false;
    ResultSet* resultSet = NULL;
    bool readonly = AST_ReadOnly(ast);
    RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(qctx);

    // Try to access the GraphContext
    CommandCtx_ThreadSafeContextLock(qctx);
    GraphContext *gc = GraphContext_Retrieve(ctx, qctx->graphName);
    if(!gc) {
        if(!ast[0]->createNode && !ast[0]->mergeNode) {
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
        // TODO: free graph if no entities were created.
    }

    CommandCtx_ThreadSafeContextUnlock(qctx);

    // Perform query validations before and after ModifyAST
    if (AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;
    ModifyAST(ast);
    if (AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;

    // Acquire the appropriate lock.
    if(readonly) Graph_AcquireReadLock(gc->g);
    else Graph_WriterEnter(gc->g);  // Single writer.
    lockAcquired = true;

    if (ast[0]->indexNode) { // Index operation.
        RedisModule_ReplyWithError(ctx, "Can't profile index operations.");
        goto cleanup;
    }

    resultSet = _prepare_resultset(ctx, ast);
    ExecutionPlan *plan = NewExecutionPlan(ctx, ast, resultSet, false);
    ExecutionPlan_Profile(plan);
    ExecutionPlan_Print(plan, ctx);
    ExecutionPlanFree(plan);

cleanup:
    // Release the read-write lock
    if(lockAcquired) {
        if(readonly)Graph_ReleaseLock(gc->g);
        else Graph_WriterLeave(gc->g);
    }

    ResultSet_Free(resultSet);
    CommandCtx_Free(qctx);
}

/* Profiles query
 * Args:
 * argv[1] graph name
 * argv[2] query to profile */
int MGraph_Profile(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3) return RedisModule_WrongArity(ctx);

    // Parse AST.
    char *errMsg = NULL;    
    const char *query = RedisModule_StringPtrLen(argv[2], NULL);

    AST **ast = ParseQuery(query, strlen(query), &errMsg);
    if (!ast) {
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

    bool readonly = AST_ReadOnly(ast);

    /* Determin query execution context
     * queries issued within a LUA script or multi exec block must
     * run on Redis main thread, others can run on different threads. */
    CommandCtx *context;
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
      // Run query on Redis main thread.
      context = CommandCtx_New(ctx, NULL, ast, argv[1], argv, argc);
      _MGraph_Profile(context);
    } else {
      // Run query on a dedicated thread.
      RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
      context = CommandCtx_New(NULL, bc, ast, argv[1], argv, argc);
      thpool_add_work(_thpool, _MGraph_Profile, context);
    }

    // Replicate only if query has potential to modify key space.
    if(!readonly) RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}
