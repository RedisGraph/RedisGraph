/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_bulk_insert.h"
#include "cmd_context.h"
#include "../util/rmalloc.h"
#include "../util/thpool/pools.h"
#include "../bulk_insert/bulk_insert.h"

typedef struct {
	CommandCtx *command_ctx;
	RedisModuleString **argv;
	int argc;
} BulkCtx;

void _MGraph_BulkInsert(void *args) {
	// Unpack arguments
	BulkCtx *bulk_ctx = args;
	CommandCtx *command_ctx = bulk_ctx->command_ctx;
	RedisModuleString **argv = bulk_ctx->argv;
	int argc = bulk_ctx->argc;

	CommandCtx_TrackCtx(command_ctx);
	RedisModuleBlockedClient *bc = CommandCtx_GetBlockingClient(command_ctx);
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);

	argv += 1; // skip "GRAPH.BULK"
	RedisModuleString *rs_graph_name = *argv++;
	const char *graphname = RedisModule_StringPtrLen(rs_graph_name, NULL);
	argc -= 2; // skip "GRAPH.BULK [GRAPHNAME]"
	RedisModuleKey *key;

	char reply[1024] = {0}; // Prepare the Redis string response
	int len;

	GraphContext *gc = NULL;

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

	int rc = BulkInsert(ctx, gc, argv, argc, nodes_in_query, relations_in_query);

	if(rc == BULK_FAIL) {
		// If insertion failed, clean up keyspace and free added entities.
		key = RedisModule_OpenKey(ctx, rs_graph_name, REDISMODULE_WRITE);
		RedisModule_DeleteKey(key);
		gc = NULL;
		goto cleanup;
	}

	// Successful bulk commands should always modify slaves.
	RedisModule_ReplicateVerbatim(ctx);

	// Replay to caller.
	len = snprintf(reply, 1024, "%llu nodes created, %llu edges created",
				   nodes_in_query, relations_in_query);
	RedisModule_ReplyWithStringBuffer(ctx, reply, len);

cleanup:
	if(gc) GraphContext_Release(gc);
	CommandCtx_ThreadSafeContextUnlock(command_ctx);
	CommandCtx_Free(command_ctx);
	rm_free(bulk_ctx);
}

int MGraph_BulkInsert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	if(argc < 3) return RedisModule_WrongArity(ctx);

	BulkCtx *bulk_ctx = rm_malloc(sizeof(BulkCtx));
	RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
	bulk_ctx->command_ctx = CommandCtx_New(NULL, bc, NULL, NULL, NULL,
										   EXEC_THREAD_MAIN, false, false, 0);
	bulk_ctx->argv = argv;
	bulk_ctx->argc = argc;
	ThreadPools_AddWorkWriter(_MGraph_BulkInsert, bulk_ctx);

	return REDISMODULE_OK;
}

