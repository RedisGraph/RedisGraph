/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
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
#include "commands/commands.h"
#include "util/thpool/thpool.h"
#include "graph/graphcontext.h"
#include "ast/cypher_whitelist.h"
#include "arithmetic/agg_funcs.h"
#include "procedures/procedure.h"
#include "arithmetic/arithmetic_expression.h"
#include "graph/serializers/graphcontext_type.h"
#include "redisearch_api.h"
#include "../../../../deps/uuid4/src/uuid4.h"

//------------------------------------------------------------------------------
// Module-level global variables
//------------------------------------------------------------------------------
pthread_mutex_t _module_mutex;     // Module-level lock.
GraphContext **graphs_in_keyspace; // Global array tracking all extant GraphContexts.
bool process_is_child;             // Flag indicating whether the running process is a child.

//------------------------------------------------------------------------------
// Thread pool variables
//------------------------------------------------------------------------------
threadpool _thpool = NULL;
pthread_key_t _tlsGCKey;    // Thread local storage graph context key.
pthread_key_t _tlsASTKey;   // Thread local storage AST key.

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

/* Create thread local storage keys. */
static int _Setup_ThreadLocalStorage() {
	int error = pthread_key_create(&_tlsGCKey, NULL);
	if(error) {
		printf("Failed to create thread local storage key.\n");
		return 0;
	}
	error = pthread_key_create(&_tlsASTKey, NULL);
	if(error) {
		printf("Failed to create thread local storage key.\n");
		return 0;
	}
	return 1;
}

static int _RegisterDataTypes(RedisModuleCtx *ctx) {
	if(GraphContextType_Register(ctx) == REDISMODULE_ERR) {
		printf("Failed to register GraphContext type\n");
		return REDISMODULE_ERR;
	}

	return REDISMODULE_OK;
}

static void _PrepareModuleGlobals() {
	assert(pthread_mutex_init(&_module_mutex, NULL) == 0);
	graphs_in_keyspace = array_new(GraphContext *, 1);
	process_is_child = false;
}

static void RG_ForkPrepare() {
	/* At this point, a fork call has been issued. (We assume that this is because BGSave was called.)
	 * Acquire the read-write lock of each graph to ensure that no graph is being modified, or else
	 * the child process will deadlock when attempting to acquire that lock.
	 * 1. If a writer thread is active, we'll wait until the writer finishes and releases the lock.
	 * 2. Otherwise, no write in progress. Acquire the lock and release it immediately after forking. */

	/* Acquire the module-scoped lock to ensure that no graphs are created or deleted during the
	 * lock acquisition process. It will be released after forking. */
	assert(pthread_mutex_lock(&_module_mutex) == 0);

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

	assert(pthread_mutex_unlock(&_module_mutex) == 0); // Release the module-scoped lock.
}

static void RG_AfterForkChild() {
	/* Mark that the child is a forked process so that it doesn't attempt invalid
	 * accesses of POSIX primitives it doesn't own. */
	process_is_child = true;
}

static void RegisterForkHooks() {
	/* Register handlers to control the behavior of fork calls.
	 * The child process does not require a handler. */
	assert(pthread_atfork(RG_ForkPrepare, RG_AfterForkParent, RG_AfterForkChild) == 0);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	/* TODO: when module unloads call GrB_finalize. */
	assert(GrB_init(GrB_NONBLOCKING) == GrB_SUCCESS);
	GxB_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format
	GxB_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse

	if(RedisModule_Init(ctx, "graph", REDISGRAPH_MODULE_VERSION,
						REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RediSearch_Init(ctx, REDISEARCH_INIT_LIBRARY) != REDISMODULE_OK) {
		return REDISMODULE_ERR;
	}

	// currently we do not support AOF in graph
	// TODO: remove when we do.
	int contextFlags = RedisModule_GetContextFlags(ctx);
	if(contextFlags & REDISMODULE_CTX_FLAGS_AOF) {
		RedisModule_Log(ctx, "warning", "RedisGraph does not support AOF");
		return REDISMODULE_ERR;
	}

	uuid4_init();
	Proc_Register();         // Register procedures.
	AR_RegisterFuncs();      // Register arithmetic functions.
	Agg_RegisterFuncs();     // Register aggregation functions.
	_PrepareModuleGlobals(); // Set up global lock and variables scoped to the entire module.
	RegisterForkHooks();     // Set up forking logic to prevent bgsave deadlocks.
	CypherWhitelist_Build(); // Build whitelist of supported Cypher elements.

	if(!_Setup_ThreadLocalStorage()) return REDISMODULE_ERR;

	long long threadCount = Config_GetThreadCount(ctx, argv, argc);
	if(!_Setup_ThreadPOOL(threadCount)) return REDISMODULE_ERR;
	RedisModule_Log(ctx, "notice", "Thread pool created, using %d threads.", threadCount);

	if(_RegisterDataTypes(ctx) != REDISMODULE_OK) return REDISMODULE_ERR;

	if(RedisModule_CreateCommand(ctx, "graph.QUERY", MGraph_Query, "write deny-oom", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.DELETE", MGraph_Delete, "write", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.EXPLAIN", MGraph_Explain, "write", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.PROFILE", MGraph_Profile, "write", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	if(RedisModule_CreateCommand(ctx, "graph.BULK", MGraph_BulkInsert, "write deny-oom", 1, 1,
								 1) == REDISMODULE_ERR) {
		return REDISMODULE_ERR;
	}

	return REDISMODULE_OK;
}
