/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../version.h"
#include "../graphcontext.h"
#include "graphcontext_type.h"
#include "encoder/encode_graphcontext.h"
#include "decoders/decode_graphcontext.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

void *GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
	GraphContext *gc = NULL;

	if(encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION) {
		// Not forward compatible.
		printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n",
			   REDISGRAPH_MODULE_VERSION);
	} else {
		gc = RdbLoadGraphContext(rdb);
	}

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
	Graph_SetMatrixPolicy(gc->g, DISABLED);
	GraphContext_Free(gc);
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
