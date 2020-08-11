/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_bulk_insert.h"
#include "./cmd_context.h"
#include "../graph/graph.h"
#include "../bulk_insert/bulk_insert.h"
#include "../util/rmalloc.h"

void _MGraph_BulkInsert(void *args) {
	// Establish thread-safe environment for batch insertion
	CommandCtx *command_ctx = (CommandCtx *)args;
	CommandCtx_TrackCtx(command_ctx);

	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);

	RedisModuleString **argv = command_ctx->argv + 1; // skip "GRAPH.BULK"
	RedisModuleString *rs_graph_name = *argv++;
	const char *graphname = RedisModule_StringPtrLen(rs_graph_name, NULL);
	int argc = command_ctx->argc - 2; // skip "GRAPH.BULK [GRAPHNAME]"
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

	if(!strcmp(RedisModule_StringPtrLen(*argv, 0), "BEGIN")) {
		argv ++;
		argc --;
		// Verify that graph does not already exist.
		key = RedisModule_OpenKey(ctx, rs_graph_name, REDISMODULE_READ);
		RedisModule_CloseKey(key);
		if(key) {
			char *err;
			asprintf(&err, "Graph with name '%s' cannot be created, as Redis key '%s' already exists.",
					 graphname, graphname);
			RedisModule_ReplyWithError(ctx, err);
			free(err);
			goto cleanup;
		}
	}

	// Read the user-provided counts for nodes and edges in the current query.
	if(RedisModule_StringToLongLong(*argv++, &nodes_in_query) != REDISMODULE_OK) {
		RedisModule_ReplyWithError(ctx, "Error parsing node count.");
		goto cleanup;
	}

	if(RedisModule_StringToLongLong(*argv++, &relations_in_query) != REDISMODULE_OK) {
		RedisModule_ReplyWithError(ctx, "Error parsing relation count.");
		goto cleanup;
	}
	argc -= 2; // already read node count and edge count

	gc = GraphContext_Retrieve(ctx, rs_graph_name, false, true);
	initial_node_count = Graph_NodeCount(gc->g);

	// Lock the graph for writing.
	Graph_AcquireWriteLock(gc->g);

	// Disable matrix synchronization for bulk insert operation
	Graph_SetMatrixPolicy(gc->g, RESIZE_TO_CAPACITY);

	// Allocate or extend datablocks to accommodate all incoming entities
	Graph_AllocateNodes(gc->g, nodes_in_query + initial_node_count);

	int rc = BulkInsert(ctx, gc, argv, argc);

	if(rc == BULK_FAIL) {
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
	if(gc) {
		Graph_ReleaseLock(gc->g);
		GraphContext_Release(gc);
	}
	CommandCtx_ThreadSafeContextUnlock(command_ctx);
	CommandCtx_Free(command_ctx);
}

int MGraph_BulkInsert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	if(argc < 3) return RedisModule_WrongArity(ctx);
	CommandCtx *context;
	// Bulk commands should always modify slaves.
	bool is_replicated = false;
	context = CommandCtx_New(ctx, NULL, NULL, NULL, argc, argv, NULL, is_replicated);
	_MGraph_BulkInsert(context);
	RedisModule_ReplicateVerbatim(ctx);
	return REDISMODULE_OK;
}

