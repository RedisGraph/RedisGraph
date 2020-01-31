/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./cmd_slowlog.h"
#include "../slow_log/slow_log.h"

int MGraph_Slowlog(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	SlowLog_Replay(ctx);
	return REDISMODULE_OK;
}