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
int _BeginBulkInsert(RedisModuleCtx *ctx, RedisModuleString *graph_name,
                     long long *final_node_count, long long *final_edge_count,
                     RedisModuleString **argv) {

    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph_name, REDISMODULE_READ);
    int keytype = RedisModule_KeyType(key);
    if (keytype != REDISMODULE_KEYTYPE_EMPTY) {
        char *err;
        if (keytype == REDISMODULE_KEYTYPE_MODULE) {
            err = "Bulk insertion not allowed on pre-existing graph.";
        } else {
            err = "Graph name already in use as different key type.";
        }
        RedisModule_CloseKey(key);
        RedisModule_ReplyWithError(ctx, err);
        return BULK_FAIL;
    }

    // Read the user-provided counts for nodes and edges in the final graph.
    if (RedisModule_StringToLongLong(*argv++, final_node_count) != REDISMODULE_OK) {
        RedisModule_ReplyWithError(ctx, "Error parsing total node count.");
        return BULK_FAIL;
    }
    if (RedisModule_StringToLongLong(*argv++, final_edge_count) != REDISMODULE_OK) {
        RedisModule_ReplyWithError(ctx, "Error parsing total relation count.");
        return BULK_FAIL;
    }

    return BULK_OK;
}

void _MGraph_BulkInsert(void *args) {
    // Establish thread-safe environment for batch insertion
    BulkInsertContext *context = (BulkInsertContext *)args;
    RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(context->bc);
    // TODO Only create key and lock context in the END block
    RedisModule_ThreadSafeContextLock(ctx);

    RedisModuleString **argv = context->argv + 1; // skip "GRAPH.BULK"
    int argc = context->argc - 2; // skip "GRAPH.BULK [GRAPHNAME]"
    RedisModuleString *rs_graph_name = *argv++;
    RedisModuleKey *key;
    size_t nodes = 0;   // Number of nodes created.
    size_t edges = 0;   // Number of edge created.

    GraphContext *gc = NULL;

    // Type chosen to match Redis conversion function
    long long final_node_count;
    long long final_edge_count;
    // Check the next to see if this is a starting query
    if (!strcmp(RedisModule_StringPtrLen(*argv, 0), "BEGIN")) {
        argv ++;
        if (_BeginBulkInsert(ctx, rs_graph_name, &final_node_count, &final_edge_count, argv) != BULK_OK) {
            goto cleanup;
        }
        argv += 3; // skip "BEGIN [NODE_COUNT] [EDGE_COUNT]"
        argc -= 3; // skip "BEGIN [NODE_COUNT] [EDGE_COUNT]"

        // Create graph and initialize its data stores.
        gc = GraphContext_NewWithCapacity(ctx, rs_graph_name,
                                          final_node_count, final_edge_count);
        // Exit if graph creation failed
        if (gc == NULL) {
            RedisModule_ReplyWithError(ctx, "Failed to allocate space for graph.");
            goto cleanup;
        } else if (argc == 0) {
            // Only the allocation string was received, so our work is done.
            char success[1024] = {0};
            int len = snprintf(success, 1024, "Initialized a graph to accommodate %lld nodes and %lld edges.", final_node_count, final_edge_count);
            RedisModule_ReplyWithStringBuffer(ctx, success, len);
            goto cleanup;
        }
    } else {
        gc = GraphContext_Retrieve(ctx, rs_graph_name, false);
        if (gc == NULL) {
            RedisModule_ReplyWithError(ctx, "Bulk insert query did not include a BEGIN token and graph was not found.");
            goto cleanup;
        }
    }

    // Lock the graph for writing.
    Graph_AcquireWriteLock(gc->g);

    // Disable matrix synchronization for bulk insert operation
    Graph_SetMatrixPolicy(gc->g, RESIZE_TO_CAPACITY);

    int rc = BulkInsert(ctx, gc, &nodes, &edges, argv, argc);

    if (rc == BULK_FAIL) {
        // If insertion failed, clean up keyspace and free added entities.
        key = RedisModule_OpenKey(ctx, rs_graph_name, REDISMODULE_WRITE);
        RedisModule_DeleteKey(key);
        goto cleanup;
    }

    // Replay to caller.
    double t = simple_toc(context->tic);
    char timings[1024] = {0};
    snprintf(timings, 1024, "%zu Nodes created, %zu Edges created, time: %.6f sec", nodes, edges, t);
    RedisModule_ReplyWithStringBuffer(ctx, timings, strlen(timings));

cleanup:
    if (gc) Graph_ReleaseLock(gc->g);
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

