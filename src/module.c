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
#include "commands/commands.h"
#include "util/thpool/thpool.h"
#include "ast/cypher_whitelist.h"
#include "arithmetic/agg_funcs.h"
#include "procedures/procedure.h"
#include "arithmetic/arithmetic_expression.h"
#include "graph/serializers/graphcontext_type.h"
#include "../deps/RediSearch/src/redisearch_api.h"

/* Thread pool. */
threadpool _thpool = NULL;
pthread_key_t _tlsGCKey;    // Thread local storage graph context key.
pthread_key_t _tlsASTKey;   // Thread local storage AST key.

/* Set up thread pool,
 * number of threads within pool should be
 * the number of available hyperthreads.
 * Returns 1 if thread pool initialized, 0 otherwise. */
int _Setup_ThreadPOOL(int threadCount) {
	// Create thread pool.
	_thpool = thpool_init(threadCount);
	if(_thpool == NULL) return 0;

	return 1;
}

/* Create thread local storage keys. */
int _Setup_ThreadLocalStorage() {
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

int _RegisterDataTypes(RedisModuleCtx *ctx) {
	if(GraphContextType_Register(ctx) == REDISMODULE_ERR) {
		printf("Failed to register GraphContext type\n");
		return REDISMODULE_ERR;
	}

	return REDISMODULE_OK;
}

static void RG_ForkPrepare() {
	/* Assuming BGSave called, Acquire write lock
	 * make sure we're not in the middel of writing,
	 * this is accomplished by acquiring the write-lock:
	 * 1. write-lock is already taken, we'll wait until writer finishes and releases the lock
	 * 2. no write in progress, we'll simply acquire an unlocked lock and release it soon enough. */

	// Acquire write lock on each graph object.
	// TODO: Need to get a hold of each graph object.
	/* for(int i = 0; i < graphs; i++) {
	    Graph_AcquireWriteLock(gc->g);
	} */
}

static void RG_AfterForkParent() {
	/* Release write lock on Redis parent process. */
	// Release write lock on each graph object.
	// TODO: Need to get a hold of each graph object.
	/* for(int i = 0; i < graphs; i++) {
	    Graph_ReleaseLock(gc->g);
	} */
}

static void RG_AfterForkChild() {
	/* In child process
	 * Release lock inherited from parent, we're not required to hold any locks
	 * in the child process. */

	// Release write lock on each graph object.
	// TODO: Need to get a hold of each graph object.
	/* for(int i = 0; i < graphs; i++) {
	    Graph_ReleaseLock(gc->g);
	} */
}

static void RegisterForkHooks() {
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

	Proc_Register();
	AR_RegisterFuncs();      // Register arithmetic functions.
	Agg_RegisterFuncs();     // Register aggregation functions.
	RegisterForkHooks();
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
