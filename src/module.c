/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <unistd.h>
#include <pthread.h>
#include "redismodule.h"
#include "debug.h"
#include "errors.h"
#include "config.h"
#include "version.h"
#include "util/arr.h"
#include "util/cron.h"
#include "query_ctx.h"
#include "arithmetic/funcs.h"
#include "commands/commands.h"
#include "util/thpool/pools.h"
#include "graph/graphcontext.h"
#include "ast/cypher_whitelist.h"
#include "procedures/procedure.h"
#include "arithmetic/arithmetic_expression.h"
#include "module_event_handlers.h"
#include "serializers/graphcontext_type.h"
#include "serializers/graphmeta_type.h"
#include "redisearch_api.h"
#include "util/redis_version.h"
#include "reconf_handler.h"

//------------------------------------------------------------------------------
// Minimal supported Redis version
//------------------------------------------------------------------------------
#define MIN_REDIS_VERION_MAJOR 6
#define MIN_REDIS_VERION_MINOR 0
#define MIN_REDIS_VERION_PATCH 0

//------------------------------------------------------------------------------
// Module-level global variables
//------------------------------------------------------------------------------
GraphContext **graphs_in_keyspace;  // Global array tracking all extant GraphContexts.
bool process_is_child;              // Flag indicating whether the running process is a child.

extern CommandCtx **command_ctxs;

static int _RegisterDataTypes(RedisModuleCtx *ctx) {
	if(GraphContextType_Register(ctx) == REDISMODULE_ERR) {
		printf("Failed to register GraphContext type\n");
		return REDISMODULE_ERR;
	}

	if(GraphMetaType_Register(ctx) == REDISMODULE_ERR) {
		printf("Failed to register GraphMeta type\n");
		return REDISMODULE_ERR;
	}
	return REDISMODULE_OK;
}

static void _PrepareModuleGlobals(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	graphs_in_keyspace = array_new(GraphContext *, 1);
	process_is_child = false;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	if(RedisModule_Init(ctx, "graph", REDISGRAPH_MODULE_VERSION,
						REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	/* TODO: when module unloads call GrB_finalize. */
	GrB_Info res = GxB_init(GrB_NONBLOCKING, RedisModule_Alloc, RedisModule_Calloc, RedisModule_Realloc, RedisModule_Free, true);
	if(res != GrB_SUCCESS) {
		RedisModule_Log(ctx, "warning", "Encountered error initializing GraphBLAS");
		return REDISMODULE_ERR;
	}

	GxB_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format

	// validate minimum redis-server version
	if(!Redis_Version_GreaterOrEqual(MIN_REDIS_VERION_MAJOR,
									 MIN_REDIS_VERION_MINOR, MIN_REDIS_VERION_PATCH)) {
		RedisModule_Log(ctx, "warning", "RedisGraph requires redis-server version %d.%d.%d and up",
						MIN_REDIS_VERION_MAJOR, MIN_REDIS_VERION_MINOR, MIN_REDIS_VERION_PATCH);
		return REDISMODULE_ERR;
	}

	if(RediSearch_Init(ctx, REDISEARCH_INIT_LIBRARY) != REDISMODULE_OK) {
		return REDISMODULE_ERR;
	}

	RedisModule_Log(ctx, "notice", "Starting up RedisGraph version %d.%d.%d.",
					REDISGRAPH_VERSION_MAJOR, REDISGRAPH_VERSION_MINOR, REDISGRAPH_VERSION_PATCH);

	Proc_Register();         // Register procedures.
	AR_RegisterFuncs();      // Register arithmetic functions.
	Cron_Start();            // Start CRON
	// Set up global lock and variables scoped to the entire module.
	_PrepareModuleGlobals(ctx, argv, argc);

	// Set up the module's configurable variables, using user-defined values where provided.
	if(Config_Init(ctx, argv, argc) != REDISMODULE_OK) return REDISMODULE_ERR;

	Config_Subscribe_Changes(reconf_handler);

	RegisterEventHandlers(ctx);
	CypherWhitelist_Build(); // Build whitelist of supported Cypher elements.

	// Create thread local storage keys for query and error contexts.
	if(!QueryCtx_Init()) return REDISMODULE_ERR;
	if(!ErrorCtx_Init()) return REDISMODULE_ERR;

	int reader_thread_count;
	int bulk_thread_count = 1;
	int writer_thread_count = 1;
	Config_Option_get(Config_THREAD_POOL_SIZE, &reader_thread_count);

	if(!ThreadPools_CreatePools(reader_thread_count, writer_thread_count, bulk_thread_count)) {
		return REDISMODULE_ERR;
	}

	RedisModule_Log(ctx, "notice", "Thread pool created, using %d threads.", reader_thread_count);

	int ompThreadCount;
	Config_Option_get(Config_OPENMP_NTHREAD, &ompThreadCount);

	if(GxB_set(GxB_NTHREADS, ompThreadCount) != GrB_SUCCESS) {
		RedisModule_Log(ctx, "warning", "Failed to set OpenMP thread count to %d", ompThreadCount);
		return REDISMODULE_ERR;
	}
	RedisModule_Log(ctx, "notice", "Maximum number of OpenMP threads set to %d", ompThreadCount);

	// initialize array of command contexts
	command_ctxs = calloc(ThreadPools_ThreadCount() + 1, sizeof(CommandCtx *));

	if(_RegisterDataTypes(ctx) != REDISMODULE_OK) return REDISMODULE_ERR;

	if(RedisModule_CreateCommand(ctx, "graph.QUERY", CommandDispatch, "write deny-oom", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.RO_QUERY", CommandDispatch, "readonly", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.DELETE", Graph_Delete, "write", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.EXPLAIN", CommandDispatch, "write deny-oom", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.PROFILE", CommandDispatch, "write deny-oom", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.BULK", Graph_BulkInsert, "write deny-oom", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.SLOWLOG", CommandDispatch, "readonly", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.CONFIG", Graph_Config, "write", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.LIST", Graph_List, "readonly", 0, 0,
								 0) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	setupCrashHandlers(ctx);

	return REDISMODULE_OK;
}

