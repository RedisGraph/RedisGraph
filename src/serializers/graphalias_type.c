/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "graphalias_type.h"
#include "../util/rmalloc.h"
#include "encoding_version.h"

// declaration of the type for redis registration
RedisModuleType *GraphAliasRedisModuleType;

static void _GraphAliasType_RdbSave
(
	RedisModuleIO *rdb,
	void *value
) {
	// Graph Alias encoding format:
	// string alias
	// string graph name

	GraphAlias *ga = (GraphAlias*)value;
	RedisModule_SaveString(rdb, ga->alias);
	RedisModule_SaveString(rdb, ga->name);
}

static void *_GraphAliasType_RdbLoad
(
	RedisModuleIO *rdb,
	int encver
) {
	// Graph Alias format:
	// string alias
	// string graph name

	RedisModuleString *alias = RedisModule_LoadString(rdb);
	RedisModuleString *name  = RedisModule_LoadString(rdb);

	return GraphAlias_New(alias, name);
}

static void _GraphAliasType_Free
(
	void *value
) {
	GraphAlias *ga = value;
	GraphAlias_Free(ga);
}

int GraphAliasType_Register
(
	RedisModuleCtx *ctx
) {
	RedisModuleTypeMethods tm = { 0 };
	tm.free     =  _GraphAliasType_Free;
	tm.rdb_save =  _GraphAliasType_RdbSave;
	tm.rdb_load =  _GraphAliasType_RdbLoad;
	tm.version  =  REDISMODULE_TYPE_METHOD_VERSION;

	GraphAliasRedisModuleType = RedisModule_CreateDataType(ctx, "graphalas",
			GRAPH_ENCODING_VERSION_LATEST, &tm);

	if(GraphAliasRedisModuleType == NULL) {
		return REDISMODULE_ERR;
	}

	return REDISMODULE_OK;
}

// create a new GraphAlias object
GraphAlias *GraphAlias_New
(
	RedisModuleString *alias,  // alias of graph
	RedisModuleString *name    // graph name being aliased
) {
	ASSERT(name  != NULL);
	ASSERT(alias != NULL);

	RedisModule_RetainString(NULL, name);
	RedisModule_RetainString(NULL, alias);

	GraphAlias *ga = rm_malloc(sizeof(GraphAlias));

	ga->name  = name;
	ga->alias = alias;

	return ga;
}

// retrievs GraphAlias from Redis keyspace
GraphAlias *GraphAlias_Retrieve
(
	RedisModuleCtx *ctx,
	RedisModuleString *ID
) {
	ASSERT(ID  != NULL);
	ASSERT(ctx != NULL);

	GraphAlias     *ga  = NULL;
	RedisModuleKey *key = RedisModule_OpenKey(ctx, ID, REDISMODULE_READ);

	if(RedisModule_ModuleTypeGetType(key) == GraphAliasRedisModuleType) {
		ga = RedisModule_ModuleTypeGetValue(key);
	}

	if(key != NULL) {
		RedisModule_CloseKey(key);
	}

	return ga;
}

// free graph alias
void GraphAlias_Free
(
	GraphAlias *ga  // graph alias to free
) {
	ASSERT(ga != NULL);

	RedisModule_FreeString(NULL, ga->name);
	RedisModule_FreeString(NULL, ga->alias);

	rm_free(ga);
}

