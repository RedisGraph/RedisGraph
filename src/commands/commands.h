/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
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
	CMD_LIST           = 9
} GRAPH_Commands;

// GraphQueryCtx stores the allocations required to execute a query.
typedef struct {
	GraphContext *graph_ctx;  // graph context
	RedisModuleCtx *rm_ctx;   // redismodule context
	QueryCtx *query_ctx;      // query context
	ExecutionCtx *exec_ctx;   // execution context
	CommandCtx *command_ctx;  // command context
	bool readonly_query;      // read only query
} GraphQueryCtx;

static GraphQueryCtx *GraphQueryCtx_New
(
	GraphContext *graph_ctx,
	RedisModuleCtx *rm_ctx,
	ExecutionCtx *exec_ctx,
	CommandCtx *command_ctx,
	bool readonly_query
) {
	GraphQueryCtx *ctx = rm_malloc(sizeof(GraphQueryCtx));

	ctx->rm_ctx          =  rm_ctx;
	ctx->exec_ctx        =  exec_ctx;
	ctx->graph_ctx       =  graph_ctx;
	ctx->query_ctx       =  QueryCtx_GetQueryCtx();
	ctx->command_ctx     =  command_ctx;
	ctx->readonly_query  =  readonly_query;

	return ctx;
}

void static inline GraphQueryCtx_Free(GraphQueryCtx *ctx) {
	ASSERT(ctx != NULL);
	rm_free(ctx);
}

//------------------------------------------------------------------------------
// graph commands
//------------------------------------------------------------------------------

void Graph_Query(void *args);
void Graph_Slowlog(void *args);
void Graph_Profile(void *args);
void Graph_Explain(void *args);
int Graph_List(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Graph_Delete(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int Graph_Config(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int CommandDispatch(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

