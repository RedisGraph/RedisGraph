/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "cmd_delete.h"

#include <assert.h>
#include "../graph/graph.h"
#include "../query_executor.h"
#include "../util/simple_timer.h"

extern RedisModuleType *GraphContextRedisModuleType;

/* DeleteContext contains the Redis string used as the graph's key
 * and a blocked Redis client to interact with. */
typedef struct {
    RedisModuleString *graph_name;  /* Name of graph to delete. */
    RedisModuleBlockedClient *bc;   /* Redis blocked client. */
} DeleteContext;

DeleteContext* _DeleteContext_New(RedisModuleString *graph_name, RedisModuleBlockedClient *bc) {
    DeleteContext *ctx = malloc(sizeof(DeleteContext));
    ctx->bc = bc;
    ctx->graph_name = graph_name;
    return ctx;
}

void _DeleteContext_Free(DeleteContext *ctx) {
    free(ctx);
}

/* Delete graph, removing the key from Redis and
 * freeing every resource allocated by the graph. */
void _MGraph_Delete(void *args) {
    // Disable matrix synchronization
    Graph_SetSynchronization(false);
    double tic[2];
    simple_tic(tic);
    DeleteContext *dCtx = args;
    RedisModuleBlockedClient *bc = dCtx->bc;
    RedisModuleString *graph_name = dCtx->graph_name;

    RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(bc);
    RedisModule_ThreadSafeContextLock(ctx);

    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph_name, REDISMODULE_WRITE);
    int keytype = RedisModule_KeyType(key);
    if (keytype == REDISMODULE_KEYTYPE_EMPTY) {
      RedisModule_ReplyWithError(ctx, "Graph was not found in database.");
      goto cleanup;
    } else if (keytype != REDISMODULE_KEYTYPE_MODULE) {
      RedisModule_ReplyWithError(ctx, "Specified graph name referred to incorrect key type.");
      goto cleanup;
    }

    // The DeleteKey call will free the GraphContext, but we must first fetch it
    // to acquire the lock.
    GraphContext *gc = RedisModule_ModuleTypeGetValue(key);
    Graph_AcquireWriteLock(gc->g);

    // Remove GraphContext from keyspace.
    if(RedisModule_DeleteKey(key) == REDISMODULE_OK) {
      char* strElapsed;
      double t = simple_toc(tic) * 1000;
      asprintf(&strElapsed, "Graph removed, internal execution time: %.6f milliseconds", t);
      RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
      free(strElapsed);
    } else {
      RedisModule_ReplyWithError(ctx, "Graph deletion failed!");
    }

cleanup:
    RedisModule_ThreadSafeContextUnlock(ctx);
    _DeleteContext_Free(dCtx);
    RedisModule_UnblockClient(bc, NULL);
    RedisModule_FreeThreadSafeContext(ctx);
}

int MGraph_Delete(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2) return RedisModule_WrongArity(ctx);

    // Construct delete operation context.
    RedisModuleString *graph_name = argv[1];
    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
    DeleteContext *dCtx = _DeleteContext_New(graph_name, bc);

    thpool_add_work(_thpool, _MGraph_Delete, dCtx);
    RedisModule_ReplicateVerbatim(ctx);

    return REDISMODULE_OK;
}

