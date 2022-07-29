/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "../serializers/graphalias_type.h"

// GraphAlias type as it is registered with Redis
extern RedisModuleType *GraphAliasRedisModuleType;
// GraphContext type as it is registered with Redis
extern RedisModuleType *GraphContextRedisModuleType;

// create an alias for an existing graph
// the idea of graph alias is similar to a file-system symbolic link
// the alias seems like a graph object but it simply proxy all of the
// graph operations run against it to the "original" graph it is aliasing
//
// usage: GRAPH.ALIAS <alias> <graph ID>
// 
// the operation may fail if:
// 1. alias already exists
// 2. graph ID is not of a graph type
//
// the created alias will show up in Redis keyspace and is replicated
int Graph_Alias
(
	RedisModuleCtx *ctx,
	RedisModuleString **argv,
	int argc
) {
	//--------------------------------------------------------------------------
	// validations
	//--------------------------------------------------------------------------
	
	if(argc != 3) {
		return RedisModule_WrongArity(ctx);
	}

	const char        *err   = NULL;
	RedisModuleString *alias = argv[1];
	RedisModuleString *name  = argv[2];

	// make sure alias isn't in the keyspace
	RedisModuleKey *key = RedisModule_OpenKey(ctx, alias,
			REDISMODULE_READ | REDISMODULE_OPEN_KEY_NOTOUCH);
	if(key != NULL) {
		// key exists
		err = "failed to create alias, key exists";
		goto error;
	}
	RedisModule_CloseKey(key);

	// make sure graph exists
	key = RedisModule_OpenKey(ctx, name,
			REDISMODULE_READ | REDISMODULE_OPEN_KEY_NOTOUCH);
	if(key == NULL) {
		// key missing
		err = "failed to create alias, missing graph key";
		goto error;
	} else if(RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) {
		// key of the wrong type
		err = "failed to create alias, wrong key type";
		goto error;
	}
	RedisModule_CloseKey(key);

	//--------------------------------------------------------------------------
	// create alias
	//--------------------------------------------------------------------------

	key = RedisModule_OpenKey(ctx, alias, REDISMODULE_WRITE);
	GraphAlias *ga = GraphAlias_New(alias, name);
	RedisModule_ModuleTypeSetValue(key, GraphAliasRedisModuleType, ga);
	RedisModule_CloseKey(key);

	// replicate command
	RedisModule_ReplicateVerbatim(ctx);

	RedisModule_ReplyWithSimpleString(ctx, "OK");
	return REDISMODULE_OK;

error:
	if(key != NULL) {
		RedisModule_CloseKey(key);
	}

	RedisModule_ReplyWithError(ctx, err);
	return REDISMODULE_OK;
}

