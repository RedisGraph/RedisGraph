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
	} else if(!VERSION_SUPPORTS_EVENTS(encver) &&
			  encver >= DECODER_SUPPORT_MIN_V_WITHOUT_SERVER_EVENTS &&
			  encver <= DECODER_SUPPORT_MAX_V_WITHOUT_SERVER_EVENTS) {
		gc = RdbLoadGraphContext_WithoutServerEvents(rdb);
		// RDB encoded using Redis 6 and up.
	} else if(VERSION_SUPPORTS_EVENTS(encver) &&
			  encver >= DECODER_SUPPORT_MIN_V_WITH_SERVER_EVENTS &&
			  encver <= DECODER_SUPPORT_MAX_V_WITH_SERVER_EVENTS) {
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

static void _GraphContextType_Free(void *value) {
	GraphContext *gc = value;
	GraphContext_Delete(gc);
}

// Register GraphContext type for Redis 6 and up.
static int _GraphContextType_RegisterWithServerEvents(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
								 .rdb_load = _GraphContextType_RdbLoad,
								 .rdb_save = _GraphContextType_RdbSaveWithServerEvents,
								 .aof_rewrite = _GraphContextType_AofRewrite,
								 .free = _GraphContextType_Free
								};

	GraphContextRedisModuleType = RedisModule_CreateDataType(ctx, "graphdata",
															 GRAPHCONTEXT_TYPE_ENCODING_VERSION_WITH_SERVER_EVENTS, &tm);
	if(GraphContextRedisModuleType == NULL) {
		return REDISMODULE_ERR;
	}
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
	if(GraphContextRedisModuleType == NULL) {
		return REDISMODULE_ERR;
	}
	return REDISMODULE_OK;
}

int GraphContextType_Register(RedisModuleCtx *ctx) {
	if(redis_major_version > 5) return _GraphContextType_RegisterWithServerEvents(ctx);
	else return _GraphContextType_RegisterWithoutServerEvents(ctx);
}
