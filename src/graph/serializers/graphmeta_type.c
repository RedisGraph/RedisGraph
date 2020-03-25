/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../version.h"
#include "graphmeta_type.h"
#include "encoding_version.h"
#include "../graphcontext.h"
#include "encoder/encode_graphcontext.h"
#include "decoders/decode_graphcontext.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphMetaRedisModuleType;

void *GraphMetaType_RdbLoad(RedisModuleIO *rdb, int encver) {
	// GraphContext *gc = NULL;

	if(encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION) {
		// Not forward compatible.
		printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n",
			   REDISGRAPH_MODULE_VERSION);
		return NULL;
	} else if(encver >= DECODER_SUPPORT_MIN_V && encver <= DECODER_SUPPORT_MAX_V) {
		gc = RdbLoadGraphContext(rdb);
	} else {
		printf("Problem when loading Graph, RedisGraph Meta key is not backward compatible with encoder version %d and should not try to load from it.\n",
			   encver);
		return NULL;
	}

	// // Add GraphContext to global array of graphs.
	// GraphContext_RegisterWithModule(gc);

	return NULL;
}

void GraphMetaType_RdbSave(RedisModuleIO *rdb, void *value) {
	RdbSaveGraphContext(rdb, value);
}

void GraphMetaType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
	// No-Op in this type.
}

void GraphMetaType_Free(void *value) {
	// No-Op in this type.
}

int GraphMetaType_Register(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
								 .rdb_load = GraphMetaType_RdbLoad,
								 .rdb_save = GraphMetaType_RdbSave,
								 .aof_rewrite = GraphMetaType_AofRewrite,
								 .free = GraphMetaType_Free
								};

	GraphMetaRedisModuleType = RedisModule_CreateDataType(ctx, "graphmeta",
														  GRAPHCONTEXT_TYPE_ENCODING_VERSION, &tm);
	if(GraphMetaRedisModuleType == NULL) {
		return REDISMODULE_ERR;
	}
	return REDISMODULE_OK;
}
