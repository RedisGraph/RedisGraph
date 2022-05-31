/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../query_ctx.h"
#include "execution_ctx.h"
#include "cmd_bulk_insert.h"

//------------------------------------------------------------------------------
// Module Commands
//------------------------------------------------------------------------------

typedef enum {
	CMD_UNKNOWN        = 0,
	CMD_QUERY          = 1,
	CMD_RO_QUERY       = 2,
	CMD_DELETE         = 3,
	CMD_CONFIG         = 4,
	CMD_EXPLAIN        = 5,
	CMD_PROFILE        = 6,
	CMD_BULK_INSERT    = 7,
	CMD_SLOWLOG        = 8,
	CMD_LIST           = 9,
	CMD_COPY           = 10
} GRAPH_Commands;

//------------------------------------------------------------------------------
// graph commands
//------------------------------------------------------------------------------

void Graph_Query(void *args);
void Graph_Slowlog(void *args);
void Graph_Profile(void *args);
void Graph_Explain(void *args);

int Graph_Copy(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Graph_List(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Graph_Debug(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Graph_Delete(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Graph_Config(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int CommandDispatch(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

