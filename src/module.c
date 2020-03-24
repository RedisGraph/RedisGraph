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
#include "graph/serializers/graphcontext_type.h"
#include "redisearch_api.h"

//------------------------------------------------------------------------------
// Module-level global variables
//------------------------------------------------------------------------------
GraphContext **graphs_in_keyspace;  // Global array tracking all extant GraphContexts.
bool process_is_child;              // Flag indicating whether the running process is a child.
unsigned long entities_threshold;   // The limit of number of entities encoded at once.

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

	return REDISMODULE_OK;
}

static void _PrepareModuleGlobals(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	graphs_in_keyspace = array_new(GraphContext *, 1);
	process_is_child = false;
	entities_threshold = Config_GetEntitiesThreshold(ctx, argv, argc);
}

static void RG_ForkPrepare() {
	/* At this point, a fork call has been issued. (We assume that this is because BGSave was called.)
	 * Acquire the read-write lock of each graph to ensure that no graph is being modified, or else
	 * the child process will deadlock when attempting to acquire that lock.
	 * 1. If a writer thread is active, we'll wait until the writer finishes and releases the lock.
	 * 2. Otherwise, no write in progress. Acquire the lock and release it immediately after forking. */

	uint graph_count = array_len(graphs_in_keyspace);
	for(uint i = 0; i < graph_count; i++) {
		// Acquire each read-write lock as a reader to guarantee that no graph is being modified.
		Graph_AcquireReadLock(graphs_in_keyspace[i]->g);
	}
}

static void RG_AfterForkParent() {
	/* The process has forked, and the parent process is continuing.
	 * Release all locks. */

	uint graph_count = array_len(graphs_in_keyspace);
	for(uint i = 0; i < graph_count; i++) {
		// Release each read-write lock.
		Graph_ReleaseLock(graphs_in_keyspace[i]->g);
	}

}

static void RG_AfterForkChild() {
	/* Restrict GraphBLAS to use a single thread this is done for 2 reasons:
	 * 1. save resources.
	 * 2. avoid a bug in GNU OpenMP which hangs when performing parallel loop in forked process. */
	GxB_set(GxB_NTHREADS, 1);

	/* Mark that the child is a forked process so that it doesn't attempt invalid
	 * accesses of POSIX primitives it doesn't own. */
	process_is_child = true;
}

/* This callback invokes once rename for a graph is done. Since the key value is a graph context
 * which saves the name of the graph for later key accesses, this data must be consistent with the key name,
 * otherwise, the graph context will remain with the previous graph name, and a key access to this name might
 * yield an empty key or wrong value. This method changes the graph name value at the graph context to be
 * consistent with the key name. */
static int _RenameGraphHandler(RedisModuleCtx *ctx, int type, const char *event,
							   RedisModuleString *key_name) {
	if(type != REDISMODULE_NOTIFY_GENERIC) return REDISMODULE_OK;
	if(strcasecmp(event, "RENAME_TO") == 0) {
		RedisModuleKey *key = RedisModule_OpenKey(ctx, key_name, REDISMODULE_WRITE);
		if(RedisModule_ModuleTypeGetType(key) == GraphContextRedisModuleType) {
			GraphContext *gc = RedisModule_ModuleTypeGetValue(key);
			size_t len;
			const char *new_name = RedisModule_StringPtrLen(key_name, &len);
			GraphContext_Rename(gc, new_name);
		}
		RedisModule_CloseKey(key);
	}
	return REDISMODULE_OK;
}

static void RegisterForkHooks(RedisModuleCtx *ctx) {
	RedisModule_SubscribeToKeyspaceEvents(ctx, REDISMODULE_NOTIFY_GENERIC, _RenameGraphHandler);

	/* Register handlers to control the behavior of fork calls.
	 * The child process does not require a handler. */
	assert(pthread_atfork(RG_ForkPrepare, RG_AfterForkParent, RG_AfterForkChild) == 0);
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

	Proc_Register();         // Register procedures.
	AR_RegisterFuncs();      // Register arithmetic functions.
	Agg_RegisterFuncs();     // Register aggregation functions.
	// Set up global lock and variables scoped to the entire module.
	_PrepareModuleGlobals(ctx, argv, argc);
	RegisterForkHooks(ctx);  // Set up hooks for renaming and forking logic to prevent bgsave deadlocks.
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
