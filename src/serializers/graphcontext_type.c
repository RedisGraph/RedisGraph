/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graphcontext_type.h"
#include "../version.h"
#include "encoding_version.h"
#include "encoder/encode_graph.h"
#include "decoders/decode_graph.h"
#include "decoders/decode_previous.h"
#include "../util/redis_version.h"

// Forward declerations of the module event handler functions
void ModuleEventHandler_AUXBeforeKeyspaceEvent(void);
void ModuleEventHandler_AUXAfterKeyspaceEvent(void);

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

extern uint redis_major_version;

static void *_GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
	GraphContext *gc = NULL;

	if(encver > GRAPH_ENCODING_VERSION_LATEST) {
		// Not forward compatible.
		printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n",
			   REDISGRAPH_MODULE_VERSION);
		return NULL;
		// Not backward compatible.
	} else if(encver < GRAPHCONTEXT_TYPE_DECODE_MIN_V) {
		printf("Failed loading Graph, RedisGraph version (%d) is not backward compatible with encoder version %d.\n",
			   REDISGRAPH_MODULE_VERSION, encver);
		return NULL;
		// Previous version.
	} else if(encver >= GRAPHCONTEXT_TYPE_DECODE_MIN_V && encver < GRAPH_ENCODING_VERSION_LATEST) {
		gc = Decode_Previous(rdb, encver);
	} else {
		// Current version.
		gc = RdbLoadGraph(rdb);
	}
	// Add GraphContext to global array of graphs.
	GraphContext_RegisterWithModule(gc);
	return gc;
}

// Save RDB for Redis 6 and up.
static void _GraphContextType_RdbSave(RedisModuleIO *rdb, void *value) {
	RdbSaveGraph(rdb, value);
}

static void _GraphContextType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
	// TODO: implement.
}

// Save an unsigned placeholder before and after the keyspace encoding.
static void _GraphContextType_AuxSave(RedisModuleIO *rdb, int when) {
	RedisModule_SaveUnsigned(rdb, 0);
}

// Decode the unsigned placeholders saved before and after the keyspace values and call the module event handler.
static int _GraphContextType_AuxLoad(RedisModuleIO *rdb, int encver, int when) {
	RedisModule_LoadUnsigned(rdb);
	if(when == REDISMODULE_AUX_BEFORE_RDB) ModuleEventHandler_AUXBeforeKeyspaceEvent();
	else ModuleEventHandler_AUXAfterKeyspaceEvent();
	return REDISMODULE_OK;
};

static void _GraphContextType_Free(void *value) {
	GraphContext *gc = value;
	GraphContext_Delete(gc);
}

int GraphContextType_Register(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
								 .rdb_load = _GraphContextType_RdbLoad,
								 .rdb_save = _GraphContextType_RdbSave,
								 .aof_rewrite = _GraphContextType_AofRewrite,
								 .free = _GraphContextType_Free,
								};

	if(Redis_Version_GreaterOrEqual(6, 0, 0)) {
		tm.aux_save = _GraphContextType_AuxSave;
		tm.aux_load = _GraphContextType_AuxLoad;
		tm.aux_save_triggers = REDISMODULE_AUX_BEFORE_RDB | REDISMODULE_AUX_AFTER_RDB;
	}

	GraphContextRedisModuleType = RedisModule_CreateDataType(ctx, "graphdata",
															 GRAPH_ENCODING_VERSION_LATEST, &tm);
	if(GraphContextRedisModuleType == NULL) return REDISMODULE_ERR;
	return REDISMODULE_OK;
}
