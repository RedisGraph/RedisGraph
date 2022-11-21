/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "graphmeta_type.h"
#include "../version.h"
#include "encoding_version.h"
#include "encoder/encode_graph.h"
#include "decoders/decode_graph.h"
#include "decoders/decode_previous.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphMetaRedisModuleType;

static void *_GraphMetaType_RdbLoad(RedisModuleIO *rdb, int encver) {
	GraphContext *gc = NULL;

	if(encver > GRAPH_ENCODING_VERSION_LATEST) {
		// Not forward compatible.
		printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n",
			   REDISGRAPH_MODULE_VERSION);
		return NULL;
		// Not backward compatible.
	} else if(encver < GRAPHMETA_TYPE_DECODE_MIN_V) {
		printf("Failed loading Graph, RedisGraph version (%d) is not backward compatible with encoder version %d.\n",
			   REDISGRAPH_MODULE_VERSION, encver);
		return NULL;
		// Previous version.
	} else if(encver < GRAPH_ENCODING_VERSION_LATEST) {
		gc = Decode_Previous(rdb, encver);
	} else {
		// Current version.
		gc = RdbLoadGraph(rdb);
	}
	// Add GraphContext to global array of graphs.
	GraphContext_RegisterWithModule(gc);
	return gc;
}

static void _GraphMetaType_RdbSave(RedisModuleIO *rdb, void *value) {
	RdbSaveGraph(rdb, value);
}

static void _GraphMetaType_Free(void *value) {
	GraphContext *gc = value;
	GraphContext_DecreaseRefCount(gc);
}

int GraphMetaType_Register(RedisModuleCtx *ctx) {
	RedisModuleTypeMethods tm = { 0 };

	tm.version   =  REDISMODULE_TYPE_METHOD_VERSION;
	tm.rdb_load  =  _GraphMetaType_RdbLoad;
	tm.rdb_save  =  _GraphMetaType_RdbSave;
	tm.free      =  _GraphMetaType_Free;

	GraphMetaRedisModuleType = RedisModule_CreateDataType(ctx, "graphmeta",
			GRAPH_ENCODING_VERSION_LATEST, &tm);

	if(GraphMetaRedisModuleType == NULL) return REDISMODULE_ERR;
	return REDISMODULE_OK;
}

