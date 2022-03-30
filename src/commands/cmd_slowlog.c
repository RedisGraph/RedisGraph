/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_context.h"
#include "../slow_log/slow_log.h"

void Graph_Slowlog(void *args) {
	CommandCtx *command_ctx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(command_ctx);
	GraphContext *gc = CommandCtx_GetGraphContext(command_ctx);

	CommandCtx_TrackCtx(command_ctx);

	SlowLog *slowlog = GraphContext_GetSlowLog(gc);
	SlowLog_Replay(slowlog, ctx);

	GraphContext_DecreaseRefCount(gc);
	CommandCtx_Free(command_ctx);
}

