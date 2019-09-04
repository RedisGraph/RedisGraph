/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_ctx.h"

#include "util/rmalloc.h"
#include <assert.h>
#include <pthread.h>

pthread_key_t _tlsQueryCtxKey;  // Thread local storage query context key.

static inline QueryCtx *_QueryCtx_GetCtx() {
	QueryCtx *ctx = pthread_getspecific(_tlsQueryCtxKey);

	if(!ctx) {
		ctx = rm_calloc(1, sizeof(QueryCtx));
		pthread_setspecific(_tlsQueryCtxKey, ctx);
	}

	return ctx;
}

bool QueryCtx_Init(void) {
	return (pthread_key_create(&_tlsQueryCtxKey, NULL) == 0);
}

void QueryCtx_Finalize(void) {
	assert(pthread_key_delete(_tlsQueryCtxKey));
}

void QueryCtx_SetAST(AST *ast) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->ast = ast;
}

void QueryCtx_SetEnv(jmp_buf *env) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->env = env;
}

void QueryCtx_SetError(char *error) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->error = error;
}

void QueryCtx_SetGraphCtx(GraphContext *gc) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->gc = gc;
}

void QueryCtx_SetRedisModuleCtx(RedisModuleCtx *redis_module_ctx) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->redis_module_ctx = redis_module_ctx;
}

AST *QueryCtx_GetAST(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->ast);
	return ctx->ast;
}

jmp_buf *QueryCtx_GetEnv(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->env);
	return ctx->env;
}

char *QueryCtx_GetError() {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->error);
	return ctx->error;
}

Graph *QueryCtx_GetGraph(void) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	return gc->g;
}

GraphContext *QueryCtx_GetGraphCtx(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->gc);
	return ctx->gc;
}

RedisModuleCtx *QueryCtx_GetRedisModuleCtx(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->redis_module_ctx);
	return ctx->redis_module_ctx;
}

void QueryCtx_Free(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();

	if(ctx->error) {
		free(ctx->error);
		ctx->error = NULL;
	}
	if(ctx->env) {
		rm_free(ctx->env);
		ctx->env = NULL;
	}

	rm_free(ctx);
	// NULL-set the context for reuse the next time this thread receives a query
	pthread_setspecific(_tlsQueryCtxKey, NULL);
}
