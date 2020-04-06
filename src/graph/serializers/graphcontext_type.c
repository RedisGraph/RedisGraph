/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../version.h"
#include "../graphcontext.h"
#include "encoding_version.h"
#include "graphcontext_type.h"
#include "encoder/encode_graphcontext.h"
#include "decoders/decode_graphcontext.h"
#include "decoders/prev/decode_previous.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

void *GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
	GraphContext *gc = NULL;

	if(encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION_LATEST) {
		// Not forward compatible.
		printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n",
			   REDISGRAPH_MODULE_VERSION);
		return NULL;
	} else if(encver < PREV_DECODER_SUPPORT_MIN_V) {
		printf("Failed loading Graph, RedisGraph version (%d) is not backward compatible with encoder version %d.\n",
			   REDISGRAPH_MODULE_VERSION, encver);
		return NULL;
	} else if(encver >= PREV_DECODER_SUPPORT_MIN_V && encver <= PREV_DECODER_SUPPORT_MAX_V) {
		gc = Decode_Previous(rdb, encver);
	} else if(encver % 2 == 0 &&
			  encver >= DECODER_SUPPORT_MIN_V_WITHOUT_SERVER_EVENTS &&
			  encver <= DECODER_SUPPORT_MAX_V_WITHOUT_SERVER_EVENTS) {
		gc = RdbLoadGraphContext(rdb);
	} else if(encver % 2 == 1 &&
			  encver >= DECODER_SUPPORT_MIN_V_WITH_SERVER_EVENTS &&
			  encver <= DECODER_SUPPORT_MAX_V_WITH_SERVER_EVENTS) {
		gc = RdbLoadGraphContext(rdb);
	}

// Add GraphContext to global array of graphs.
	GraphContext_RegisterWithModule(gc);

	return gc;
}

void GraphContextType_RdbSave(RedisModuleIO *rdb, void *value) {
	RdbSaveGraphContext(rdb, value);
}

void GraphContextType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
	// TODO: implement.
}

void GraphContextType_Free(void *value) {
	GraphContext *gc = value;
	GraphContext_Delete(gc);
}

int GraphContextType_RegisterWithServerEvents(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
								 .rdb_load = GraphContextType_RdbLoad,
								 .rdb_save = GraphContextType_RdbSave,
								 .aof_rewrite = GraphContextType_AofRewrite,
								 .free = GraphContextType_Free
								};

	GraphContextRedisModuleType = RedisModule_CreateDataType(ctx, "graphdata",
															 GRAPHCONTEXT_TYPE_ENCODING_VERSION_WITH_SERVER_EVENTS, &tm);
	if(GraphContextRedisModuleType == NULL) {
		return REDISMODULE_ERR;
	}
	return REDISMODULE_OK;
}

int GraphContextType_RegisterWithoutServerEvents(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
								 .rdb_load = GraphContextType_RdbLoad,
								 .rdb_save = GraphContextType_RdbSave,
								 .aof_rewrite = GraphContextType_AofRewrite,
								 .free = GraphContextType_Free
								};

	GraphContextRedisModuleType = RedisModule_CreateDataType(ctx, "graphdata",
															 GRAPHCONTEXT_TYPE_ENCODING_VERSION_WITHOUT_SERVER_EVENTS, &tm);
	if(GraphContextRedisModuleType == NULL) {
		return REDISMODULE_ERR;
	}
	return REDISMODULE_OK;
}
