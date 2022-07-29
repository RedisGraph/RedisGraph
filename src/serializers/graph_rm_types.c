/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graphmeta_type.h"
#include "graphalias_type.h"
#include "graphcontext_type.h"

// register all graph redis module data types
int GraphRMTypes_Register
(
	RedisModuleCtx *ctx
) {

	int res;

	// register graph context type
	res = GraphContextType_Register(ctx);
	if (res != REDISMODULE_OK) {
		return res;
	}

	// register graph meta type
	res = GraphMetaType_Register(ctx);
	if (res != REDISMODULE_OK) {
		return res;
	}

	// register graph alias type
	res = GraphAliasType_Register(ctx);
	if (res != REDISMODULE_OK) {
		return res;
	}

	return REDISMODULE_OK;
}

