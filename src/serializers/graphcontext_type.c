/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graphcontext_type.h"
#include "../version.h"
#include "encoding_version.h"
#include "encoder/with_server_events/encode_with_server_events.h"
#include "encoder/without_server_events/encode_without_server_events.h"
#include "decoders/with_server_events/decode_with_server_events.h"
#include "decoders/without_server_events/decode_without_server_events.h"
#include "decoders/prev/decode_previous.h"

#define VERSION_SUPPORTS_EVENTS(encver) ((encver) % 2 == 1)

// Forward declerations of the module event handler functions
void ModuleEventHandler_AUXBeforeKeyspaceEvent();
void ModuleEventHandler_AUXAfterKeyspaceEvent();

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

extern uint redis_major_version;

static void *_GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
	GraphContext *gc = NULL;

	if(encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION_LATEST) {
		// Not forward compatible.
		printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n",
			   REDISGRAPH_MODULE_VERSION);
		return NULL;
		// Not backward compatible.
	} else if(encver < PREV_DECODER_SUPPORT_MIN_V) {
		printf("Failed loading Graph, RedisGraph version (%d) is not backward compatible with encoder version %d.\n",
			   REDISGRAPH_MODULE_VERSION, encver);
		return NULL;
		// Previous version.
	} else if(encver >= PREV_DECODER_SUPPORT_MIN_V && encver <= PREV_DECODER_SUPPORT_MAX_V) {
		gc = Decode_Previous(rdb, encver);
		// RDB encoded using Redis 5.
	} else if(!VERSION_SUPPORTS_EVENTS(encver)) {
		gc = RdbLoadGraphContext_WithoutServerEvents(rdb);
		// RDB encoded using Redis 6 and up.
	} else if(VERSION_SUPPORTS_EVENTS(encver)) {
		gc = RdbLoadGraphContext_WithServerEvents(rdb);
	}
	// Add GraphContext to global array of graphs.
	GraphContext_RegisterWithModule(gc);
	return gc;
}

// Save RDB for Redis 5.
static void _GraphContextType_RdbSaveWithoutServerEvents(RedisModuleIO *rdb, void *value) {
	RdbSaveGraphContext_WithoutServerEvents(rdb, value);
}

// Save RDB for Redis 6 and up.
static void _GraphContextType_RdbSaveWithServerEvents(RedisModuleIO *rdb, void *value) {
	RdbSaveGraphContext_WithServerEvents(rdb, value);
}

static void _GraphContextType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
	// TODO: implement.
}

// Save a string before and after the keyspace encoding.
static void _GraphContextType_AuxSave(RedisModuleIO *rdb, int when) {
	const char *str = "graph";
	RedisModule_SaveStringBuffer(rdb, str, strlen(str) + 1);
}

// Decode the strings saved before and after the keyspace values and call the module event handler.
static int _GraphContextType_AuxLoad(RedisModuleIO *rdb, int encver, int when) {
	rm_free(RedisModule_LoadStringBuffer(rdb, NULL));
	if(when == REDISMODULE_AUX_BEFORE_RDB) ModuleEventHandler_AUXBeforeKeyspaceEvent();
	else ModuleEventHandler_AUXAfterKeyspaceEvent();
	return REDISMODULE_OK;
};

static void _GraphContextType_Free(void *value) {
	GraphContext *gc = value;
	// Check if graph is in decode - The graph key has been decoded but not all the virtual keys finished.
	if(GraphContext_IsInDecode(gc)) {
		RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(NULL);
		RedisModule_ReplyWithError(ctx, "Graph is currently replicating");
		RedisModule_FreeThreadSafeContext(ctx);
		return;
	}
	GraphContext_Delete(gc);
}

// Register GraphContext type for Redis 6 and up.
static int _GraphContextType_RegisterWithServerEvents(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
								 .rdb_load = _GraphContextType_RdbLoad,
								 .rdb_save = _GraphContextType_RdbSaveWithServerEvents,
								 .aof_rewrite = _GraphContextType_AofRewrite,
								 .free = _GraphContextType_Free,
								 .aux_save = _GraphContextType_AuxSave,
								 .aux_load = _GraphContextType_AuxLoad,
								 .aux_save_triggers = REDISMODULE_AUX_BEFORE_RDB | REDISMODULE_AUX_AFTER_RDB
								};

	GraphContextRedisModuleType = RedisModule_CreateDataType(ctx, "graphdata",
															 GRAPHCONTEXT_TYPE_ENCODING_VERSION_WITH_SERVER_EVENTS, &tm);
	if(GraphContextRedisModuleType == NULL) return REDISMODULE_ERR;
	return REDISMODULE_OK;
}

// Register GraphContext type for Redis 5.
static int _GraphContextType_RegisterWithoutServerEvents(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
								 .rdb_load = _GraphContextType_RdbLoad,
								 .rdb_save = _GraphContextType_RdbSaveWithoutServerEvents,
								 .aof_rewrite = _GraphContextType_AofRewrite,
								 .free = _GraphContextType_Free
								};

	GraphContextRedisModuleType = RedisModule_CreateDataType(ctx, "graphdata",
															 GRAPHCONTEXT_TYPE_ENCODING_VERSION_WITHOUT_SERVER_EVENTS, &tm);
	if(GraphContextRedisModuleType == NULL) return REDISMODULE_ERR;
	return REDISMODULE_OK;
}

int GraphContextType_Register(RedisModuleCtx *ctx) {
	if(redis_major_version > 5) return _GraphContextType_RegisterWithServerEvents(ctx);
	else return _GraphContextType_RegisterWithoutServerEvents(ctx);
}
