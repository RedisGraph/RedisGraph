/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_bulk_insert.h"
#include "./cmd_context.h"
#include "../graph/graph.h"
#include "../query_executor.h"
#include "../bulk_insert/bulk_insert.h"
#include "../util/rmalloc.h"

void _MGraph_BulkInsert(void *args) {
    // Establish thread-safe environment for batch insertion
    CommandCtx *context = (CommandCtx*)args;
    RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(context);
    CommandCtx_ThreadSafeContextLock(context);

    RedisModuleString **argv = context->argv + 1; // skip "GRAPH.BULK"
    RedisModuleString *rs_graph_name = *argv++;
    const char *graphname = RedisModule_StringPtrLen(rs_graph_name, NULL);
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
        // Create graph and initialize its schemas.
        gc = GraphContext_New(ctx, graphname, nodes_in_query, relations_in_query);
        assert(gc);
    } else {
        // Query did not start with a "BEGIN" token
        gc = GraphContext_Retrieve(ctx, graphname);
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
    len = snprintf(reply, 1024, "%llu nodes created, %llu edges created",
                   nodes_in_query, relations_in_query);
    RedisModule_ReplyWithStringBuffer(ctx, reply, len);

cleanup:
    if (gc) Graph_ReleaseLock(gc->g);
    CommandCtx_ThreadSafeContextUnlock(context);
    CommandCtx_Free(context);
}

int MGraph_BulkInsert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3) return RedisModule_WrongArity(ctx);

     /* Determin query execution context
      * queries issued within a LUA script or multi exec block must
      * run on Redis main thread, others can run on different threads. */
    CommandCtx *context;
    int flags = RedisModule_GetContextFlags(ctx);
    if (flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
        // Construct concurent query context.
        context = CommandCtx_New(ctx, NULL, NULL, NULL, NULL, argv, argc);
        // Execute bulk on redis main thread.
        _MGraph_BulkInsert(context);
    } else {
        RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
        // Construct concurent query context.
        context = CommandCtx_New(NULL, bc, NULL, NULL, NULL, argv, argc);
        // Execute bulk insert on a dedicated thread.
        thpool_add_work(_thpool, _MGraph_BulkInsert, context);
    }

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}
