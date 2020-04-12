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
#include "graph/serializers/graphmeta_type.h"
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

// Create the meta keys for each graph in the key space - used on RDB start event.
static void _CreateKeySpaceMetaKeys(RedisModuleCtx *ctx) {
	uint graphs_in_keyspace_count = array_len(graphs_in_keyspace);
	for(uint i = 0; i < graphs_in_keyspace_count; i ++) {
		GraphContext_CreateMetaKeys(ctx, graphs_in_keyspace[i]);
	}
}

// Delete the meta keys for each graph in the key space - used on RDB finish (save/load/fail) event.
static void _ClearKeySpaceMetaKeys(RedisModuleCtx *ctx) {
	uint graphs_in_keyspace_count = array_len(graphs_in_keyspace);
	for(uint i = 0; i < graphs_in_keyspace_count; i ++) {
		GraphContext_DeleteMetaKeys(ctx, graphs_in_keyspace[i]);
	}
}

// Delete all meta keys before the actual flush, as the flush may be out of order
static void _FlushDBHandler(RedisModuleCtx *ctx, RedisModuleEvent eid, uint64_t subevent,
							void *data) {
	if(eid.id == REDISMODULE_EVENT_FLUSHDB && subevent == REDISMODULE_SUBEVENT_FLUSHDB_START) {
		_ClearKeySpaceMetaKeys(ctx);
	}
}

// Checks if the event is persistence start event.
static bool _IsEventPersistenceStart(RedisModuleEvent eid, uint64_t subevent) {
	return eid.id == REDISMODULE_EVENT_PERSISTENCE  &&
		   (subevent == REDISMODULE_SUBEVENT_PERSISTENCE_RDB_START ||    // Normal RDB.
			subevent == REDISMODULE_SUBEVENT_PERSISTENCE_AOF_START ||    // Preamble AOF.
			subevent == REDISMODULE_SUBEVENT_PERSISTENCE_SYNC_RDB_START  // SAVE and DEBUG RELOAD.
		   );
}

// Checks if the event is persistence end event.
static bool _IsEventPersistenceEnd(RedisModuleEvent eid, uint64_t subevent) {
	return eid.id == REDISMODULE_EVENT_PERSISTENCE &&
		   (subevent == REDISMODULE_SUBEVENT_PERSISTENCE_ENDED ||  // Save ended.
			subevent == REDISMODULE_SUBEVENT_PERSISTENCE_FAILED    // Save failed.
		   );
}

// Checks if the event is loading end event.
static bool _IsEventLoadingEnd(RedisModuleEvent eid, uint64_t subevent) {
	return eid.id == REDISMODULE_EVENT_LOADING &&
		   (subevent == REDISMODULE_SUBEVENT_LOADING_ENDED ||  // Load ended.
			subevent == REDISMODULE_SUBEVENT_LOADING_FAILED    // Load failed.
		   );
}

// Server persistence event handler.
static void _PersistenceEventHandler(RedisModuleCtx *ctx, RedisModuleEvent eid, uint64_t subevent,
									 void *data) {
	if(_IsEventPersistenceStart(eid, subevent)) _CreateKeySpaceMetaKeys(ctx);
	else if(_IsEventPersistenceEnd(eid, subevent)) _ClearKeySpaceMetaKeys(ctx);
}

// Server loading event handler.
static void _LoadingEventHandler(RedisModuleCtx *ctx, RedisModuleEvent eid, uint64_t subevent,
								 void *data) {
	if(_IsEventLoadingEnd(eid, subevent)) _ClearKeySpaceMetaKeys(ctx);
}

static void _RegisterServerEvents(RedisModuleCtx *ctx) {
	RedisModule_SubscribeToKeyspaceEvents(ctx, REDISMODULE_NOTIFY_GENERIC, _RenameGraphHandler);
	// Regiseter to server events for redis 6 and up.
	if(redis_major_version > 5) {
		RedisModule_SubscribeToServerEvent(ctx, RedisModuleEvent_FlushDB, _FlushDBHandler);
		RedisModule_SubscribeToServerEvent(ctx, RedisModuleEvent_Persistence, _PersistenceEventHandler);
		RedisModule_SubscribeToServerEvent(ctx, RedisModuleEvent_Loading, _LoadingEventHandler);
	}
}

static void _RegisterForkHooks(RedisModuleCtx *ctx) {
	/* Register handlers to control the behavior of fork calls.
	 * The child process does not require a handler. */
	assert(pthread_atfork(RG_ForkPrepare, RG_AfterForkParent, RG_AfterForkChild) == 0);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	// Set the server major version as a global property.
	_SetRedisMajorVersion(ctx);
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
	_RegisterForkHooks(ctx);  // Set up hooks for forking logic to prevent bgsave deadlocks.
	_RegisterServerEvents(ctx); // Set up hooks renaming and server events on Redis 6 and up.
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
