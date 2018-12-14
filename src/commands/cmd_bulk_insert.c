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

    RedisModuleString **argv = context->argv + 1; // skip "GRAPH.BULK"
    RedisModuleString *rs_graph_name = *argv++;
    int argc = context->argc - 2; // skip "GRAPH.BULK [GRAPHNAME]"
    RedisModuleKey *key;

    char reply[1024] = {0}; // Prepare the Redis string response
    int len;

    GraphContext *gc = NULL;

    // Number of entities already created
    size_t initial_node_count = 0;

    // Number of entities being created in this query
    // (declared as long longs to match Redis conversion function)
    long long nodes_in_query;
    long long relations_in_query;

    // Check the next to see if this is a starting query
    bool initial_query = false;
    if (!strcmp(RedisModule_StringPtrLen(*argv, 0), "BEGIN")) {
        argv ++;
        argc --;
        initial_query = true;
        // Verify that graph does not already exist.
        key = RedisModule_OpenKey(ctx, rs_graph_name, REDISMODULE_READ);
        if (key) {
            const char *graphname = RedisModule_StringPtrLen(rs_graph_name, NULL);
            char *err;
            asprintf(&err, "Graph with name '%s' cannot be created, as Redis key '%s' already exists.", graphname, graphname); 
            RedisModule_ReplyWithError(ctx, err);
            free(err);
            goto cleanup;
        }
    }

    // Read the user-provided counts for nodes and edges in the current query.
    if (RedisModule_StringToLongLong(*argv++, &nodes_in_query) != REDISMODULE_OK) {
        RedisModule_ReplyWithError(ctx, "Error parsing node count.");
        goto cleanup;
    }

    if (RedisModule_StringToLongLong(*argv++, &relations_in_query) != REDISMODULE_OK) {
        RedisModule_ReplyWithError(ctx, "Error parsing relation count.");
        goto cleanup;
    }
    argc -= 2; // already read node count and edge count

    if (initial_query) {
        // Create graph and initialize its data stores.
        gc = GraphContext_New(ctx, rs_graph_name, nodes_in_query, relations_in_query);
        assert(gc);
    } else {
        // Query did not start with a "BEGIN" token
        gc = GraphContext_Retrieve(ctx, rs_graph_name);
        if (gc == NULL) {
            RedisModule_ReplyWithError(ctx, "Bulk insert query did not include a BEGIN token and graph was not found.");
            goto cleanup;
        }
        initial_node_count = Graph_NodeCount(gc->g);
    }

    // Lock the graph for writing.
    Graph_AcquireWriteLock(gc->g);

    // Disable matrix synchronization for bulk insert operation
    Graph_SetMatrixPolicy(gc->g, RESIZE_TO_CAPACITY);

    // Allocate or extend datablocks to accommodate all incoming entities
    Graph_AllocateNodes(gc->g, nodes_in_query + initial_node_count);

    int rc = BulkInsert(ctx, gc, argv, argc);

    if (rc == BULK_FAIL) {
        // If insertion failed, clean up keyspace and free added entities.
        key = RedisModule_OpenKey(ctx, rs_graph_name, REDISMODULE_WRITE);
        RedisModule_DeleteKey(key);
        gc = NULL;
        goto cleanup;
    }

    // Replay to caller.
    double t = simple_toc(context->tic);
    len = snprintf(reply, 1024, "%llu nodes created, %llu edges created, time: %.6f sec",
                   nodes_in_query, relations_in_query, t);
    RedisModule_ReplyWithStringBuffer(ctx, reply, len);

cleanup:
    if (gc) Graph_ReleaseLock(gc->g);
    RedisModule_ThreadSafeContextUnlock(ctx);
    RedisModule_FreeThreadSafeContext(ctx);
    RedisModule_UnblockClient(context->bc, NULL);
    BulkInsertContext_Free(context);
}

int MGraph_BulkInsert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3) return RedisModule_WrongArity(ctx);

    // Return immediately if invocation context is Lua or a MULTI/EXEC block
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
        RedisModule_ReplyWithError(ctx, "RedisGraph commands may not be called from non-blocking contexts.");
        return REDISMODULE_OK;
    }

    // Prepare context.
    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
    BulkInsertContext *context = BulkInsertContext_New(ctx, bc, argv, argc);
    simple_tic(context->tic);

    // Execute bulk insert on a dedicated thread.
    thpool_add_work(_thpool, _MGraph_BulkInsert, context);

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

