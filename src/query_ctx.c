/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_ctx.h"
#include "util/rmalloc.h"
#include "util/simple_timer.h"
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

void QueryCtx_SetGraphCtx(GraphContext *gc) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->gc = gc;
}

void QueryCtx_SetExceptionHandler(jmp_buf *breakpoint, bool free_cause) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->breakpoint = breakpoint;
	ctx->free_cause = free_cause;
}

void QueryCtx_SetError(char *error) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->error = error;
}

AST *QueryCtx_GetAST(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->ast);
	return ctx->ast;
}

GraphContext *QueryCtx_GetGraphCtx(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->gc);
	return ctx->gc;
}

Graph *QueryCtx_GetGraph(void) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	return gc->g;
}

jmp_buf *QueryCtx_GetExceptionHandler(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->breakpoint);
	return ctx->breakpoint;
}

char *QueryCtx_GetError(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->error;
}

inline bool QueryCtx_ShouldFreeExceptionCause(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->free_cause;
}

inline bool QueryCtx_EncounteredError(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->error != NULL;
}


void QueryCtx_StartTimer(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	simple_tic(ctx->timer); // Start the execution timer.
}

double QueryCtx_GetExecutionTime(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return simple_toc(ctx->timer) * 1000;
}

void QueryCtx_Free(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();

	if(ctx->error) {
		free(ctx->error);
		ctx->error = NULL;
	}
	if(ctx->breakpoint) {
		rm_free(ctx->breakpoint);
		ctx->breakpoint = NULL;
	}

	rm_free(ctx);
	// NULL-set the context for reuse the next time this thread receives a query
	pthread_setspecific(_tlsQueryCtxKey, NULL);
}

