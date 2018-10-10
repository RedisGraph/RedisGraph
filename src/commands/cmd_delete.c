/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "cmd_delete.h"

#include <assert.h>
#include "stores/store.h"
#include "../graph/graph.h"
#include "../query_executor.h"
#include "../util/simple_timer.h"

/* DeleteGraphContext is used to hold all information required 
 * in order to delete a graph. */
typedef struct {
    RedisModuleString *graphID;     /* Graph ID been deleted. */
    RedisModuleBlockedClient *bc;   /* Redis blocked client. */
} DeleteGraphContext;

DeleteGraphContext *DeleteGraphContext_new(RedisModuleString *graphID, RedisModuleBlockedClient *bc) {
    DeleteGraphContext *ctx = malloc(sizeof(DeleteGraphContext));
    ctx->bc = bc;
    ctx->graphID = graphID;
    return ctx;
}

void DeleteGraphContext_free(DeleteGraphContext *ctx) {
    free(ctx);
}

/* Delete graph, removes every Redis key related to given graph 
 * free every resource allocated by the graph. */
void _MGraph_Delete(void *args) {
    double tic[2];
    simple_tic(tic);
    DeleteGraphContext *dCtx = (DeleteGraphContext *)args;
    RedisModuleBlockedClient *bc = dCtx->bc;
    RedisModuleString *graphID = dCtx->graphID;

    RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(bc);
    const char *graphIDStr = RedisModule_StringPtrLen(graphID, NULL);

    MGraph_AcquireWriteLock(ctx);
    
    Graph *g = Graph_Get(ctx, graphID);

    // Graph does not exists, nothing to delete.
    if(!g) goto cleanup;

    // Remove Label stores.
    size_t keyCount = 0;
    RedisModuleString **keys = LabelStore_GetKeys(ctx, graphIDStr, &keyCount);
    assert(keyCount>0 && keys);
    
    for(int idx = 0; idx < keyCount; idx++) {
        RedisModuleString *storeKeyStr = keys[idx];
        RedisModuleKey *key = RedisModule_OpenKey(ctx, storeKeyStr, REDISMODULE_WRITE);
        if(RedisModule_DeleteKey(key) != REDISMODULE_OK) {
            // Log error!
        }
        RedisModule_Free(storeKeyStr);
    }
    free(keys);

    // Remove Graph from Redis keyspace.
    RedisModuleKey *key = RedisModule_OpenKey(ctx, graphID, REDISMODULE_WRITE);
    if(RedisModule_DeleteKey(key) != REDISMODULE_OK) {
        // Log error!
    }

    // Remove GraphContext from keyspace.
    key = GraphContext_Key(ctx, graphIDStr);
    if(RedisModule_DeleteKey(key) != REDISMODULE_OK) {
        // Log error!
    }

cleanup:
    MGraph_ReleaseLock(ctx);
    DeleteGraphContext_free(dCtx);

    char* strElapsed;
    double t = simple_toc(tic) * 1000;
    asprintf(&strElapsed, "Graph removed, internal execution time: %.6f milliseconds", t);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);
    RedisModule_FreeThreadSafeContext(ctx);
    RedisModule_UnblockClient(bc, NULL);
}

int MGraph_Delete(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2) return RedisModule_WrongArity(ctx);
    
    // Construct delete operation context.
    RedisModuleString *graphID = argv[1];
    RedisModule_RetainString(ctx, graphID);
    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
    DeleteGraphContext *dCtx = DeleteGraphContext_new(graphID, bc);

    thpool_add_work(_thpool, _MGraph_Delete, dCtx);
    RedisModule_ReplicateVerbatim(ctx);

    return REDISMODULE_OK;
}