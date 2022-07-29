/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../redismodule.h"

typedef struct {
	RedisModuleString *alias;  // alias of graph
	RedisModuleString *name;   // graph name being aliased
} GraphAlias;

// register GraphAlias with Redis module types
int GraphAliasType_Register
(
	RedisModuleCtx *ctx
);

// create a new GraphAlias object
GraphAlias *GraphAlias_New
(
	RedisModuleString *alias,  // alias of graph
	RedisModuleString *name    // graph name being aliased
);

// retrievs GraphAlias from Redis keyspace
GraphAlias *GraphAlias_Retrieve
(
	RedisModuleCtx *ctx,
	RedisModuleString *ID
);

// free graph alias
void GraphAlias_Free
(
	GraphAlias *ga  // graph alias to free
);

