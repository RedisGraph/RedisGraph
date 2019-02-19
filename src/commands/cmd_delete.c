/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_delete.h"

#include <assert.h>
#include "./cmd_context.h"
#include "../graph/graph.h"
#include "../query_executor.h"
#include "../util/simple_timer.h"

extern RedisModuleType *GraphContextRedisModuleType;

/* Delete graph, removing the key from Redis and
 * freeing every resource allocated by the graph. */
void _MGraph_Delete(void *args) {
    double tic[2];
    simple_tic(tic);
    CommandCtx *dCtx = (CommandCtx*)args;    
    RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(dCtx);
    RedisModuleString *graph_name = RedisModule_CreateString(ctx, dCtx->graphName, strlen(dCtx->graphName));
    CommandCtx_ThreadSafeContextLock(dCtx);

    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph_name, REDISMODULE_WRITE);
    int keytype = RedisModule_KeyType(key);
    if (keytype == REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_ReplyWithError(ctx, "Graph was not found in database.");
        goto cleanup;
    } else if (keytype != REDISMODULE_KEYTYPE_MODULE) {
        RedisModule_ReplyWithError(ctx, "Specified graph name referred to incorrect key type.");
        goto cleanup;
    }

    // Retrieve the GraphContext to disable synchronization.
    GraphContext *gc = RedisModule_ModuleTypeGetValue(key);
    
    // Acquire write lock, guarantee we're the only thread executing.
    Graph_AcquireWriteLock(gc->g);

    // Disable matrix synchronization for graph deletion.
    Graph_SetMatrixPolicy(gc->g, DISABLED);

    // Remove GraphContext from keyspace.
    if(RedisModule_DeleteKey(key) == REDISMODULE_OK) {
        char* strElapsed;
        double t = simple_toc(tic) * 1000;
        asprintf(&strElapsed, "Graph removed, internal execution time: %.6f milliseconds", t);
        RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
        free(strElapsed);
    } else {
        Graph_ReleaseLock(gc->g);
        RedisModule_ReplyWithError(ctx, "Graph deletion failed!");
    }

cleanup:
    RedisModule_Free(graph_name);
    CommandCtx_ThreadSafeContextUnlock(dCtx);
    CommandCtx_Free(dCtx);
}

int MGraph_Delete(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2) return RedisModule_WrongArity(ctx);

    CommandCtx *context;
    RedisModuleString *graph_name = argv[1];

    /* Determin query execution context
     * queries issued within a LUA script or multi exec block must
     * run on Redis main thread, others can run on different threads. */
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
        context = CommandCtx_New(ctx, NULL, NULL, NULL, graph_name, argv, argc);
        _MGraph_Delete(context);
    } else {
        RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
        context = CommandCtx_New(NULL, bc, NULL, NULL, graph_name, argv, argc);
        thpool_add_work(_thpool, _MGraph_Delete, context);
    }

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}
