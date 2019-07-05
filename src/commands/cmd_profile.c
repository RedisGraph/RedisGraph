/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_profile.h"
#include "cmd_context.h"
#include "../graph/graph.h"
#include "../util/simple_timer.h"
#include "../execution_plan/execution_plan.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

void _MGraph_Profile(void *args) {
    CommandCtx *qctx = (CommandCtx*)args;
    RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(qctx);

    bool lockAcquired = false;

    AST *ast = AST_Build(qctx->parse_result);
    bool readonly = AST_ReadOnly(ast->root);

    // Try to access the GraphContext
    CommandCtx_ThreadSafeContextLock(qctx);
    GraphContext *gc = GraphContext_Retrieve(ctx, qctx->graphName, readonly);
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
        // TODO: free graph if no entities were created.
    }

    CommandCtx_ThreadSafeContextUnlock(qctx);

    // Perform query validations
    if (AST_PerformValidations(ctx, ast) != AST_VALID) goto cleanup;

    // Acquire the appropriate lock.
    if(readonly) Graph_AcquireReadLock(gc->g);
    else Graph_WriterEnter(gc->g);  // Single writer.
    lockAcquired = true;

    const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
    if (root_type == CYPHER_AST_CREATE_NODE_PROP_INDEX || root_type == CYPHER_AST_DROP_NODE_PROP_INDEX) {
        RedisModule_ReplyWithError(ctx, "Can't profile index operations.");
        goto cleanup;
    } else if (root_type != CYPHER_AST_QUERY) {
        assert("Unhandled query type" && false);
    }

    ExecutionPlan *plan = NewExecutionPlan(ctx, gc, false, false);
    ExecutionPlan_Profile(plan);
    ExecutionPlan_Print(plan, ctx);
    ExecutionPlan_Free(plan);

cleanup:
    // Release the read-write lock
    if(lockAcquired) {
        if(readonly)Graph_ReleaseLock(gc->g);
        else Graph_WriterLeave(gc->g);
    }

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

    // Parse AST.
    cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);

    if(AST_ContainsErrors(parse_result)) {
        char *errMsg = AST_ReportErrors(parse_result);
        cypher_parse_result_free(parse_result);
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    const cypher_astnode_t *query_root = AST_GetBody(parse_result);
    bool readonly = AST_ReadOnly(query_root);

    /* Determin query execution context
     * queries issued within a LUA script or multi exec block must
     * run on Redis main thread, others can run on different threads. */
    CommandCtx *context;
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
        // Run query on Redis main thread.
        context = CommandCtx_New(ctx, NULL, parse_result, argv[1], argv, argc);
        _MGraph_Profile(context);
    } else {
        // Run query on a dedicated thread.
        RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
        context = CommandCtx_New(NULL, bc, parse_result, argv[1], argv, argc);
        thpool_add_work(_thpool, _MGraph_Profile, context);
    }

    // Replicate only if query has potential to modify key space.
    if(!readonly) RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}
