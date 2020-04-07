/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../version.h"
#include "graphmeta_type.h"
#include "encoding_version.h"
#include "../graphcontext.h"
#include "encoder/with_server_events/encode_with_server_events.h"
#include "decoders/with_server_events/decode_with_server_events.h"
#include "../../util/rmalloc.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphMetaRedisModuleType;

static void *_GraphMetaType_RdbLoad(RedisModuleIO *rdb, int encver) {
	GraphContext *gc = NULL;

	if(encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION_LATEST) {
		// Not forward compatible.
		printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n",
			   REDISGRAPH_MODULE_VERSION);
		return NULL;
		// Meta key only available on Redis with server events
	} else if(encver % 2 == 1 &&
			  encver >= DECODER_SUPPORT_MIN_V_WITH_SERVER_EVENTS &&
			  encver <= DECODER_SUPPORT_MAX_V_WITH_SERVER_EVENTS) {
		gc = RdbLoadGraphContext_WithServerEvents(rdb);
	} else {
		printf("Problem when loading Graph, RedisGraph Meta key is not backward compatible with encoder version %d and should not try to load from it.\n",
			   encver);
		return NULL;
	}

	// Add GraphContext to global array of graphs.
	GraphContext_RegisterWithModule(gc);
	// Add meta key to the graph context meta keys collection.
	const RedisModuleString *meta_redis_string = RedisModule_GetKeyNameFromIO(rdb);
	const char *meta_key_name = RedisModule_StringPtrLen(meta_redis_string, NULL);
	GraphEncodeContext_AddKey(gc->encoding_context, meta_key_name);
	// Create meta context.
	GraphMetaContext *meta = GraphMetaContext_New(gc, meta_key_name);
	return meta;
}

static void _GraphMetaType_RdbSave(RedisModuleIO *rdb, void *value) {
	GraphMetaContext *meta = value;
	RdbSaveGraphContext_WithServerEvents(rdb, meta->gc);
}

static void _GraphMetaType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
	// No-Op in this type.
}

static void _GraphMetaType_Free(void *value) {
	GraphMetaContext *meta = value;
	GraphEncodeContext_DeleteKey(meta->gc->encoding_context, meta->meta_key_name);
	GraphMetaContext_Free(meta);
}

int GraphMetaType_Register(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
								 .rdb_load = _GraphMetaType_RdbLoad,
								 .rdb_save = _GraphMetaType_RdbSave,
								 .aof_rewrite = _GraphMetaType_AofRewrite,
								 .free = _GraphMetaType_Free
								};

	GraphMetaRedisModuleType = RedisModule_CreateDataType(ctx, "graphmeta",
														  GRAPHCONTEXT_TYPE_ENCODING_VERSION_WITH_SERVER_EVENTS, &tm);
	if(GraphMetaRedisModuleType == NULL) {
		return REDISMODULE_ERR;
	}
	return REDISMODULE_OK;
}
