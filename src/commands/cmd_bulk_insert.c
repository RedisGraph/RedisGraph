/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "cmd_bulk_insert.h"
#include "../graph/graph.h"
#include "../query_executor.h"
#include "../util/simple_timer.h"
#include "../bulk_insert/bulk_insert.h"

BulkInsertContext* BulkInsertContext_New(RedisModuleCtx *ctx, RedisModuleBlockedClient *bc, RedisModuleString **argv, int argc) {
    BulkInsertContext *context = malloc(sizeof(BulkInsertContext));

    context->bc = bc;
    context->argc = argc;
    context->argv = argv;

    return context;
}

void BulkInsertContext_Free(BulkInsertContext* ctx) {
    free(ctx);
}

void _MGraph_BulkInsert(void *args) {
    // Establish thread-safe environment for batch insertion
    BulkInsertContext *context = (BulkInsertContext *)args;
    RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(context->bc);
    RedisModule_ThreadSafeContextLock(ctx);

    int argc = context->argc;
    RedisModuleString **argv = context->argv;
    RedisModuleString *rs_graph_name = argv[1];
    size_t nodes = 0;   // Number of nodes created.
    size_t edges = 0;   // Number of edge created.

    // Try to retrieve GraphContext. If context and graph do not exist, create them.
    GraphContext *gc = GraphContext_Get(ctx, rs_graph_name, false);
    if (gc == NULL) gc = GraphContext_New(ctx, rs_graph_name);

    // Exit if graph creation failed
    if (gc == NULL) goto cleanup;

    int rc = BulkInsert(ctx, gc, &nodes, &edges, argv+2, argc-2);

    // Exit if insertion failed
    if (rc == BULK_FAIL) goto cleanup;

    // Replay to caller.
    double t = simple_toc(context->tic);
    char timings[1024] = {0};
    snprintf(timings, 1024, "%zu Nodes created, %zu Edges created, time: %.6f sec\n", nodes, edges, t);
    RedisModule_ReplyWithStringBuffer(ctx, timings, strlen(timings));

cleanup:
    Graph_ReleaseLock(gc->g);
    RedisModule_ThreadSafeContextUnlock(ctx);
    RedisModule_FreeThreadSafeContext(ctx);
    RedisModule_UnblockClient(context->bc, NULL);
    BulkInsertContext_Free(context);
}

int MGraph_BulkInsert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 4) return RedisModule_WrongArity(ctx);
    // Prepare context.
    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
    BulkInsertContext *context = BulkInsertContext_New(ctx, bc, argv, argc);
    simple_tic(context->tic);

    // Execute bulk insert on a dedicated thread.
    thpool_add_work(_thpool, _MGraph_BulkInsert, context);

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}
