/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <unistd.h>
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
#include "module_event_handlers.h"
#include "serializers/graphcontext_type.h"
#include "serializers/graphmeta_type.h"
#include "redisearch_api.h"
#include "util/redis_version.h"

//------------------------------------------------------------------------------
// Minimal supported Redis version
//------------------------------------------------------------------------------
#define MIN_REDIS_VERION_MAJOR 5
#define MIN_REDIS_VERION_MINOR 0
#define MIN_REDIS_VERION_PATCH 7

//------------------------------------------------------------------------------
// Module-level global variables
//------------------------------------------------------------------------------
RG_Config config;                   // Module global configuration.
GraphContext **graphs_in_keyspace;  // Global array tracking all extant GraphContexts.
bool process_is_child;              // Flag indicating whether the running process is a child.

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
	/* TODO: when module unloads call GrB_finalize. */
	GrB_Info res = GxB_init(GrB_NONBLOCKING, rm_malloc, rm_calloc, rm_realloc, rm_free, true);
	if(res != GrB_SUCCESS) {
		RedisModule_Log(ctx, "warning", "Encountered error initializing GraphBLAS: '%s'", GrB_error());
		return REDISMODULE_ERR;
	}
	GxB_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format
	GxB_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse

	if(RedisModule_Init(ctx, "graph", REDISGRAPH_MODULE_VERSION,
						REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	// Validate minimum redis-server version.
	if(!Redis_Version_GreaterOrEqual(MIN_REDIS_VERION_MAJOR, MIN_REDIS_VERION_MINOR,
									 MIN_REDIS_VERION_PATCH)) {
		RedisModule_Log(ctx, "warning", "RedisGraph requires redis-server version %d.%d.%d and up",
						MIN_REDIS_VERION_MAJOR, MIN_REDIS_VERION_MINOR, MIN_REDIS_VERION_PATCH);
		return REDISMODULE_ERR;
	}

	if(RediSearch_Init(ctx, REDISEARCH_INIT_LIBRARY) != REDISMODULE_OK) {
		return REDISMODULE_ERR;
	}

	Proc_Register();         // Register procedures.
	AR_RegisterFuncs();      // Register arithmetic functions.
	Agg_RegisterFuncs();     // Register aggregation functions.
	// Set up global lock and variables scoped to the entire module.
	_PrepareModuleGlobals(ctx, argv, argc);

	// Set up the module's configurable variables, using user-defined values where provided.
	if(Config_Init(ctx, argv, argc) != REDISMODULE_OK) return REDISMODULE_ERR;

	RegisterEventHandlers(ctx);
	CypherWhitelist_Build(); // Build whitelist of supported Cypher elements.

	// Create thread local storage key.
	if(!QueryCtx_Init()) return REDISMODULE_ERR;

	int threadCount = Config_GetThreadCount();
	if(!_Setup_ThreadPOOL(threadCount)) return REDISMODULE_ERR;
	RedisModule_Log(ctx, "notice", "Thread pool created, using %d threads.", threadCount);

	int ompThreadCount = Config_GetOMPThreadCount();
	if(GxB_set(GxB_NTHREADS, ompThreadCount) != GrB_SUCCESS) {
		RedisModule_Log(ctx, "warning", "Failed to set OpenMP thread count to %d", ompThreadCount);
		return REDISMODULE_ERR;
	}
	RedisModule_Log(ctx, "notice", "Maximum number of OpenMP threads set to %d", ompThreadCount);

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

