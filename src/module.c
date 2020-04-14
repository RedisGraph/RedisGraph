/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "redismodule.h"
#include "config.h"
#include "version.h"
#include "util/arr.h"
#include "query_ctx.h"
#include "arithmetic/funcs.h"
#include "commands/commands.h"
#include "util/thpool/thpool.h"
#include "graph/graphcontext.h"
#include "ast/cypher_whitelist.h"
#include "arithmetic/agg_funcs.h"
#include "procedures/procedure.h"
#include "arithmetic/arithmetic_expression.h"
#include "serializers/graphcontext_type.h"
#include "serializers/graphmeta_type.h"
#include "redisearch_api.h"

//------------------------------------------------------------------------------
// Module-level global variables
//------------------------------------------------------------------------------
GraphContext **graphs_in_keyspace;  // Global array tracking all extant GraphContexts.
bool process_is_child;              // Flag indicating whether the running process is a child.
uint64_t entities_threshold;        // The limit of number of entities encoded at once.
uint redis_major_version;           // The redis server major version.

//------------------------------------------------------------------------------
// Thread pool variables
//------------------------------------------------------------------------------
threadpool _thpool = NULL;

/* Set up thread pool,
 * number of threads within pool should be
 * the number of available hyperthreads.
 * Returns 1 if thread pool initialized, 0 otherwise. */
static int _Setup_ThreadPOOL(int threadCount) {
	// Create thread pool.
	_thpool = thpool_init(threadCount);
	if(_thpool == NULL) return 0;

	return 1;
}

// Sets the global variable of the redis major version
static void _SetRedisMajorVersion(RedisModuleCtx *ctx) {
	const char *server_version;
	int major;
	int minor;
	int patch;
	// Check if there is an implementation for redis module api for redis 6 an up, by checking the existence pf a Redis 6 API function pointer.
	// If the function pointer is null, the server version is Redis 5.
	if(RedisModule_GetServerInfo) {
		// Retrive the server info.
		RedisModuleServerInfoData *info =  RedisModule_GetServerInfo(ctx, "Server");
		server_version = RedisModule_ServerInfoGetFieldC(info, "redis_version");
		sscanf(server_version, "%d.%d.%d", &major, &minor, &patch);
		RedisModule_FreeServerInfo(ctx, info);
		if(major > 5) redis_major_version = major;
		// Check for Redis 6 rc versions which starts with 5.0.x. Those versions support RedisModule_GetServerInfo.
		else redis_major_version = major == 5 && minor == 9 ? 6 : major;
	} else {
		// RedisModule_GetServerInfo exists only on Redis 6 and up, so the current server major version is 5.
		redis_major_version = 5;
	}
}

static int _RegisterDataTypes(RedisModuleCtx *ctx) {
	if(GraphContextType_Register(ctx) == REDISMODULE_ERR) {
		printf("Failed to register GraphContext type\n");
		return REDISMODULE_ERR;
	}

	// Register graph meta types on redis versions > 5 as the graph serialization there is diefferent.
	if(redis_major_version > 5) {
		if(GraphMetaType_Register(ctx) == REDISMODULE_ERR) {
			printf("Failed to register GraphMeta type\n");
			return REDISMODULE_ERR;
		}
	}
	return REDISMODULE_OK;
}

static void _PrepareModuleGlobals(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	graphs_in_keyspace = array_new(GraphContext *, 1);
	process_is_child = false;
	entities_threshold = Config_GetEntitiesThreshold(ctx, argv, argc);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	/* TODO: when module unloads call GrB_finalize. */
	assert(GxB_init(GrB_NONBLOCKING, rm_malloc, rm_calloc, rm_realloc, rm_free, true) == GrB_SUCCESS);
	GxB_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format
	GxB_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse

	if(RedisModule_Init(ctx, "graph", REDISGRAPH_MODULE_VERSION,
						REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RediSearch_Init(ctx, REDISEARCH_INIT_LIBRARY) != REDISMODULE_OK) {
		return REDISMODULE_ERR;
	}

	// Set the server major version as a global property.
	_SetRedisMajorVersion(ctx);

	Proc_Register();         // Register procedures.
	AR_RegisterFuncs();      // Register arithmetic functions.
	Agg_RegisterFuncs();     // Register aggregation functions.
	// Set up global lock and variables scoped to the entire module.
	_PrepareModuleGlobals(ctx, argv, argc);
	CypherWhitelist_Build(); // Build whitelist of supported Cypher elements.

	// Create thread local storage key.
	if(!QueryCtx_Init()) return REDISMODULE_ERR;

	long long threadCount = Config_GetThreadCount(ctx, argv, argc);
	if(!_Setup_ThreadPOOL(threadCount)) return REDISMODULE_ERR;
	RedisModule_Log(ctx, "notice", "Thread pool created, using %d threads.", threadCount);

	if(_RegisterDataTypes(ctx) != REDISMODULE_OK) return REDISMODULE_ERR;

	if(RedisModule_CreateCommand(ctx, "graph.QUERY", CommandDispatch, "write deny-oom", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.DELETE", MGraph_Delete, "write", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.EXPLAIN", CommandDispatch, "write", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.PROFILE", CommandDispatch, "write", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.BULK", MGraph_BulkInsert, "write deny-oom", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.SLOWLOG", CommandDispatch, "readonly", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	return REDISMODULE_OK;
}
