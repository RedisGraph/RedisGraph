/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../version.h"
#include "../graphcontext.h"
#include "graphcontext_type.h"
#include "encoder/encode_graphcontext.h"
#include "decoders/decode_graphcontext.h"
#include "decoders/prev/decode_previous.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

#define GRAPHCONTEXT_TYPE_ENCODING_VERSION 6 // Current RDB encoding version

#define DECODER_SUPPORT_MAX_V 6      // Highest RDB version that can be decoded.
#define DECODER_SUPPORT_MIN_V 6      // Lowest version that can be decoded using the latest routine.
#define PREV_DECODER_SUPPORT_MIN_V 4 // Lowest version that has backwards-compatibility decoding routines.

void *GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
	GraphContext *gc = NULL;

	if(encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION) {
		// Not forward compatible.
		printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n",
			   REDISGRAPH_MODULE_VERSION);
		return NULL;
	} else if(encver >= DECODER_SUPPORT_MIN_V && encver <= DECODER_SUPPORT_MAX_V) {
		gc = RdbLoadGraphContext(rdb);
	} else if(encver >= PREV_DECODER_SUPPORT_MIN_V) {
		gc = Decode_Previous(rdb, encver);
	} else {
		printf("Failed loading Graph, RedisGraph version (%d) is not backward compatible with encoder version %d.\n",
			   REDISGRAPH_MODULE_VERSION, encver);
		return NULL;
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

int GraphContextType_Register(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
								 .rdb_load = GraphContextType_RdbLoad,
								 .rdb_save = GraphContextType_RdbSave,
								 .aof_rewrite = GraphContextType_AofRewrite,
								 .free = GraphContextType_Free
								};

	GraphContextRedisModuleType = RedisModule_CreateDataType(ctx, "graphdata",
															 GRAPHCONTEXT_TYPE_ENCODING_VERSION, &tm);
	if(GraphContextRedisModuleType == NULL) {
		return REDISMODULE_ERR;
	}
	return REDISMODULE_OK;
}
