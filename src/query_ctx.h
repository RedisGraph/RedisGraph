/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include"ast/ast.h"
#include "redismodule.h"
#include "graph/graphcontext.h"

#include <setjmp.h>

typedef struct {
	AST *ast;
	char *error;
	jmp_buf *env;
	GraphContext *gc;
	RedisModuleCtx *redis_module_ctx;
} QueryCtx;

bool QueryCtx_Init(void);
void QueryCtx_Finalize(void);

void QueryCtx_SetAST(AST *ast);
void QueryCtx_SetExceptionHandler(jmp_buf *env);
void QueryCtx_SetError(char *error);
void QueryCtx_SetGraphCtx(GraphContext *gc);
void QueryCtx_SetRedisModuleCtx(RedisModuleCtx *ctx);

AST *QueryCtx_GetAST(void);
jmp_buf *QueryCtx_GetExceptionHandler(void);
char *QueryCtx_GetError(void);
bool QueryCtx_EncounteredError(void);
GraphContext *QueryCtx_GetGraphCtx(void);
Graph *QueryCtx_GetGraph(void);
RedisModuleCtx *QueryCtx_GetRedisModuleCtx(void);

void QueryCtx_Free(void);

