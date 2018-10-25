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
    
    // Remove GraphContext from keyspace.
    RedisModuleKey *key = GraphContext_Key(ctx, graphIDStr);
    if(RedisModule_DeleteKey(key) != REDISMODULE_OK) {
        // Log error!
    }

    MGraph_ReleaseLock(ctx);
    DeleteGraphContext_free(dCtx);

    char* strElapsed;
    double t = simple_toc(tic) * 1000;
    asprintf(&strElapsed, "Graph removed, internal execution time: %.6f milliseconds", t);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);
    RedisModule_UnblockClient(bc, NULL);
    RedisModule_FreeThreadSafeContext(ctx);
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